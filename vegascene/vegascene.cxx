/*=========================================================================

Program: Vegascene
Module: vegascene.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/


#include "vegascene.h"

#include "jscallbackmanager.h"
#include "data.h"
#include "jsconsole.h"


#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstringlist.h>
#include <QRegExp>
#include <QDir>
#include <QtQml/qjsengine.h>
#include <QJSValueList>


#include <iostream>
#include <vector>


//------------------------------------------------------------------------------
#define JSMODULE_BASE_PATH "../jsmodules/"
#define JSMODULE_PATH(NAME) \
     JSMODULE_BASE_PATH #NAME ".js"


//------------------------------------------------------------------------------
#define JSLIB_BASE_PATH "../jslib/"
#define JSLIB_PATH(NAME) \
     JSLIB_BASE_PATH #NAME


//------------------------------------------------------------------------------
const char* VegaScene::JSVarCallbackManagerName = "timeoutBackend";
const char* VegaScene::JSVarContext2dName = "context2d";
const char* VegaScene::JSVarDataLoaderName = "data";
const char* VegaScene::JSVarSpecContentName = "specContent";
const char* VegaScene::JSVarResultName = "outSceneGraph";
const char* VegaScene::JSFuncRenderName = "render";


//------------------------------------------------------------------------------
VegaScene::VegaScene()
    : Engine(),
      Result(),
      EngineReady(true),
      Initialized(false),
      SpecLoaded(false),
      Rendered(false),
      JSModuleVega("vg", JSMODULE_PATH(vega)),
      CallbackManager(),
      Context2d(),
      DataLoader(),
      BaseURL(""),
      JSVarNamespace("vegascene"),
      Console()
{
    JSModule d3("d3", JSMODULE_PATH(d3));
    this->JSModuleVega.AddRequiredModule(d3);
#ifdef WITH_TOPOJSON
    JSModule tj("topojson", JSMODULE_PATH(topojson));
    this->JSModuleVega.AddRequiredModule(tj);
#endif
    this->JSModuleVega.preLoadConfig = VegaScene::SetPreLoadConfig();
    this->JSModuleVega.postLoadConfig = VegaScene::SetPostLoadConfig();
    this->LoadJSModule(this->JSModuleVega);

#ifdef WITH_EXAMPLE_LIBS
    this->LoadJSLib(JSLIB_PATH(d3.geo.projection.min.js));
#endif

    // Define object `vegascene = {}` inside the engine.
    this->DefineNamespace();

    // Initialize `console.log` js function using the Console object as a
    // back-end.
    this->InjectNonJSObject(this->Console, "console");
    SetJSObjConsole();

    // Initialize `(window.)setTimeout` js function using the CallbackManager
    // object as a back-end.
    this->InjectNonJSObject(this->CallbackManager,
                            VegaScene::JSVarCallbackManagerName);
    SetJSFuncSetTimeout();

    // Initialize `context2d` js object using the Context2d object as a
    // back-end.
    this->InjectNonJSObject(this->Context2d, VegaScene::JSVarContext2dName);
    SetJSObjContext2d();

    // Initialize data loading routines using the DataLoader object as a
    // back-end.
    this->InjectNonJSObject(this->DataLoader, VegaScene::JSVarDataLoaderName);
    SetJSFuncDataLoad();

    // Define `render` function which starts the generation of the scene graph.
    SetJSFuncRender();

    // Define `run` function which parses spec and passes it to `render`.
    SetJSFuncRun();

    this->Initialized = this-> EngineReady;
}


//------------------------------------------------------------------------------
bool VegaScene::LoadSpec(const String& spec)
{
    this->SpecLoaded = false;
    if (!(this->Initialized)) return false;

    QString ns = QString::fromStdString(this->JSVarNamespace);
    QJSValue nsObject = this->Engine.globalObject().property(ns);
    QString varName = QString::fromLatin1(VegaScene::JSVarSpecContentName);
    QJSValue specObj(QString::fromStdString(spec));
    nsObject.setProperty(varName, specObj);
    String qVarName = this->GetQualifiedName(VegaScene::JSVarSpecContentName);
    this->SpecLoaded = this->SafeEvaluate(qVarName);
#ifdef DEBUG
    if (this->SpecLoaded)
    {
        std::cout << "Spec contents loaded!" << std::endl;
    }
#endif
    return this->SpecLoaded;
}


//------------------------------------------------------------------------------
bool VegaScene::LoadSpecFromFile(const String& filePath)
{
    this->SpecLoaded = false;
    if (!(this->Initialized)) return false;

    String spec;
    if (this->ReadFile(filePath, spec))
    {
        if (spec.empty())
        {
            spec = String("{}");
        }
        return this->LoadSpec(spec);
    }
    else
    {
        return false;
    }
}


//------------------------------------------------------------------------------
const String& VegaScene::GetBaseURL() const
{
    return this->BaseURL;
}


//------------------------------------------------------------------------------
void VegaScene::SetBaseURL(const String& baseURL)
{
    QString newBaseURL = QString::fromStdString(baseURL);
    QRegExp re(Data::LoadProtocolRE);
    if (!re.exactMatch(newBaseURL))
    {
        if (QDir::isAbsolutePath(newBaseURL))
        {
            newBaseURL.prepend(Data::LoadFileProtocol);
        }
        else
        {
            newBaseURL = QString(Data::LoadFileProtocol) + QDir::currentPath()
                    + QDir::separator() + newBaseURL;
        }
    }
    newBaseURL.append(QDir::separator());
    this->BaseURL = newBaseURL.toStdString();
    String source = String("vg.config.baseURL = '") + this->BaseURL + String("';");
    this->SafeEvaluate(source);
    this->DataLoader.SetBaseURL(QString::fromStdString(this->BaseURL));
}


//------------------------------------------------------------------------------
bool VegaScene::Render()
{
    this->Rendered = false;
    if( !(this->Initialized) ) return false;
    if( !(this->SpecLoaded) ) return false;

    String program;
    program += this->GetQualifiedName("run") + String("();");
    this->Rendered = this->SafeEvaluate(program);

    // Callback execution can start.
    CallbackManager.WakeAll();
#ifdef DEBUG
    if (this->Rendered)
    {
        std::cout << "log: Scene graph has been rendered!" << std::endl;
    }
#endif
    return this->Rendered;
}


//------------------------------------------------------------------------------
const String& VegaScene::GetResult()
{
    if (this->Rendered)
    {
        // Wait to evaluate result for callback end.
        this->CallbackManager.WaitForFinished();

        String qJSVarResult = this->GetQualifiedName(VegaScene::JSVarResultName);
        String program;
        program += String("JSON.stringify(") + qJSVarResult + String(", null, 2)");
        this->SafeEvaluate(program, &(this->Result));
    }
    return this->Result;
}


//------------------------------------------------------------------------------
bool VegaScene::Write(const String& filePath)
{
    if (this->Rendered)
    {
        if (filePath.empty())
        {
            String output = this->GetResult();
#ifdef DEBUG
            std::cout << "Scene Graph in JSON formmat: " << std::endl;
#endif
            std::cout << output << std::endl;
            return true;
        }
        else
        {
            QFile file(filePath.c_str());
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                std::cerr << "error: can not write to file: '" << filePath
                          << "'" << std::endl;
                return false;
            }
            else
            {
                QString output = QString::fromStdString(this->GetResult());
                QTextStream out(&file);
                out << output;
                file.close();
                return true;
            }
        }
    }
    return false;
}


//------------------------------------------------------------------------------
// Description:
// Internal function that evaluate the source code in `code` and check if an
// error occurred. In such a case the `EngineReady` status attribute is set to
// false. If no error occurs the result is returned in `result`.
// Return the value of the `EngineReady` status attribute.
bool VegaScene::SafeEvaluate(const String& code, String* result)
{
    if (!(this->EngineReady)) return false;
    QJSValue res = this->Engine.evaluate(QString::fromStdString(code));
    if (res.isError())
    {
        this->EngineReady = false;
#ifdef DEBUG
        std::cerr << "In evaluating:\n"
                  << "```\n"
                  << code << "\n"
                  << "```\n"
                  << "error: uncaught exception: "
                        << res.toString().toStdString() << std::endl;
#endif
        return false;
    }
    else
    {
        this->EngineReady = true;
        if (result != NULL)
            *result = res.toString().toStdString();
    }
    return this->EngineReady;
}


//------------------------------------------------------------------------------
// Description:
// Internal function which let C++ object properties, signals and slots be
// accessible inside the JavaScript engine context through the name defined in
// `varName` which is attached to the vegascene namespace. If an error occurs
// the `EngineReady` status attribute is set to false. Return the value of
// the `EngineReady` status attribute.
template< typename T >
bool VegaScene::InjectNonJSObject(T& object, String varName)
{
    if( !(this->EngineReady) ) return false;
    QJSValue jsObject = this->Engine.newQObject(&object);
    QString ns = QString::fromStdString(this->JSVarNamespace);
    QJSValue nsObject = this->Engine.globalObject().property(ns);
    nsObject.setProperty(QString::fromStdString(varName), jsObject);

    // check
    String qVarName = this->GetQualifiedName(varName.c_str());
    return this->SafeEvaluate(qVarName);
}


//------------------------------------------------------------------------------
// Description:
// Internal function used for loading into the engine context a JavaScript
// source file. If an error occurs the `EngineReady` status attribute is set
// to false. Return the value of the `EngineReady` status attribute.
bool VegaScene::LoadJSLib(const String& pathToLib)
{
    if ( !(this->EngineReady) ) return false;

    String libSource;
    this->EngineReady = this->ReadFile(pathToLib, libSource);
    if (this->EngineReady)
    {
        QJSValue result = this->Engine.evaluate(QString::fromStdString(libSource));
        this->EngineReady = !result.isError();
#ifdef DEBUG
        if(!this->EngineReady)
        {
            std::cerr << "In loading JavaScript source file: "
                      << pathToLib << "\n"
                      << "error: uncaught exception: "
                      << result.toString().toStdString() << std::endl;
        }
#endif
    }
    return this->EngineReady;
}


//------------------------------------------------------------------------------
// Description:
// Internal function used for loading into the engine context a JavaScript
// module, which is retrieved by the `info.Filepath` attribute. Eventually
// required modules are loaded before the current module except the value of
// `withDependencies` parameter is false. Through external JavaScript source
// files is also possible to load into the engine context pre loading
// configuration setting and post loading configuration setting.
// Pay attention: the routine does not handle either circular or shared
// dependencies.
// If an error occurs the `EngineReady` status attribute is set to false.
// Return the value of the `EngineReady` status attribute.
bool VegaScene::LoadJSModule(const JSModule& info, bool withDependencies)
{
    if ( !(this->EngineReady) ) return false;

    // First load eventually required modules.
    if (withDependencies)
    {
        typedef JSModule::DependenciesListType::const_iterator ConstIterator;
        ConstIterator end = info.Dependencies.end();
        for (ConstIterator it = info.Dependencies.begin(); it != end; ++it)
        {
            if (!this->LoadJSModule(*it, true))
            {
                return false;
            }
        }
    }

    if (!info.preLoadConfig.empty())
    {
        this->EngineReady = this->LoadJSLib(info.preLoadConfig);
    }

    if (this->EngineReady)
    {
        String moduleSource;
        this->EngineReady = ReadFile(info.FilePath, moduleSource);
        if (this->EngineReady)
        {
            QString source = QString::fromStdString(moduleSource);
            QJSValue result = this->Engine.evaluate(source);
            this->EngineReady = !result.isError();
            if (this->EngineReady)
            {
                // Check if the module namespace is defined.
                QString ns = QString::fromStdString(info.NameSpace);
                this->EngineReady = this->Engine.globalObject().hasProperty(ns);
#ifdef DEBUG
                if (this->EngineReady)
                {
                    std::cout << "log: JavaScript module with namespace '"
                                 + info.NameSpace
                                 + "' has been loaded successfully."
                              << std::endl;
                }
                else
                {
                    std::cerr << "error: JavaScript module with namespace '"
                              << info.NameSpace
                              << "' has been loaded but namespace is not present."
                              << std::endl;
                }
#endif
                if (this->EngineReady && !info.postLoadConfig.empty())
                {
                    this->EngineReady = this->LoadJSLib(info.postLoadConfig);
                }

            }
#ifdef DEBUG
            else
            {
                std::cerr << "In loading JavaScript module with namespace '"
                          << info.NameSpace << "'\n"
                          << "error: uncaught exception: "
                          << result.toString().toStdString() << std::endl;
            }
#endif
        }
    }
    return this->EngineReady;
}


//------------------------------------------------------------------------------
// Description:
// Internal function used for loading into the engine context a JavaScript
// source file whose path is `filePath`. The loaded source file must contain a
// function named `vegascene_template`. The elements of the `argList` parameter
// are passed, as function arguments to `vegascene_template`.
// The arguments pushed into `argList` can be of two types: the name of an
// object that must be already defined inside the engine or the unqualified
// name of a property of any object. In the latter case the name must be
// prepended with a dot “.”. In this way `LoadJSTemplate` function can discern
// between object names and property names.
// In fact for each string representing an object name the `LoadJSTemplate`
// function creates a QJSValue instance which refers to such an object, and
// passes it as an argument to `vegascene_template` in place of the object name
// string. Each string representing a property name is passed “as is” (except
// that for removing the heading dot) to `vegascene_template`.
bool VegaScene::LoadJSTemplate(const String& filePath,
                                 const String & arg1,
                                 const String & arg2,
                                 const String & arg3,
                                 const String & arg4,
                                 const String & arg5)
{
    ArgListType argList;
    const String* args[5] = {&arg1, &arg2, &arg3, &arg4, &arg5};
    for (int i = 0; i < 5; ++i) {
        if (args[i]->empty()) break;
        argList.push_back(*(args[i]));
    }

    return this->LoadJSTemplate(filePath, argList);
}


//------------------------------------------------------------------------------
bool VegaScene::LoadJSTemplate(const String& filePath,
                               const ArgListType & argList)
{
    if ( !(this->EngineReady) ) return false;

    QJSValueList jsArgList;


    typedef ArgListType::const_iterator CIterator;
    for (CIterator it = argList.begin(), end = argList.end(); it != end; ++it)
    {
        QJSValue arg;
        QString name = QString::fromStdString(*it);
        if (name[0] == '.')
        {
            arg = QJSValue(name.remove(0,1));
        }
        else
        {
            arg = this->Engine.evaluate(name);
            this->EngineReady = !arg.isError() && arg.isObject();
            if (!this->EngineReady)
            {
#ifdef DEBUG
                std::cerr << "In loading JavaScript template: "
                          << filePath << "\n"
                          << "error: in reading argument '" << *it << "': "
                          << arg.toString().toStdString() << std::endl;
#endif
                return false;
            }
        }
        jsArgList.append(arg);
    }

    this->EngineReady = this->LoadJSLib(filePath);
    QJSValue templateFunc = this->Engine.evaluate("vegascene_template");
    this->EngineReady = !templateFunc.isError();
    if (this->EngineReady)
    {
        this->EngineReady = templateFunc.isCallable();
        if (this->EngineReady)
        {
            QJSValue result = templateFunc.call(jsArgList);
            this->EngineReady = !result.isError();
#ifdef DEBUG
            if (!this->EngineReady)
            {
                std::cerr << "In loading JavaScript template: "
                          << filePath << "\n"
                          << "error: in instantiating template: "
                          << result.toString().toStdString() << std::endl;
            }
#endif
        }
#ifdef DEBUG
        else
        {
            std::cerr << "In loading JavaScript template: " << filePath << "\n"
                      << "error: template function is not a callable object.";
        }
#endif
    }
#ifdef DEBUG
    else
    {
        std::cerr << "In loading JavaScript template: " << filePath << "\n"
                  << "error: in getting template function: "
                  << templateFunc.toString().toStdString() << std::endl;
    }
#endif

    this->Engine.evaluate("vegascene_template = undefined");
    return this->EngineReady;
}


//------------------------------------------------------------------------------
bool VegaScene::ReadFile(const String& filePath, String& fileContent) const
{
    QFile scriptFile(QString::fromStdString(filePath));
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
#ifdef DEBUG
        std::cerr << "I cannot open file: " << filePath << std::endl;
#endif
        return false;
    }
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();
    fileContent = contents.toStdString();
    return true;
}


//------------------------------------------------------------------------------
String VegaScene::SetPreLoadConfig()
{
    String sourcePath(JSLIB_PATH(vega.preload.config.js));
    return sourcePath;
}


//------------------------------------------------------------------------------
String VegaScene::SetPostLoadConfig()
{
    String sourcePath(JSLIB_PATH(vega.postload.config.js));
    return sourcePath;
}



//------------------------------------------------------------------------------
String PrependPropFlag(const char* name)
{
    String flaggedName = String(".") + String(name);
    return flaggedName;
}


//------------------------------------------------------------------------------
void VegaScene::SetJSFuncSetTimeout()
{
    String timeoutBackend = this->GetQualifiedName(VegaScene::JSVarCallbackManagerName);
    this->LoadJSTemplate(JSLIB_PATH(setTimeout.js), timeoutBackend);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSFuncRender()
{
    String render = PrependPropFlag(VegaScene::JSFuncRenderName);
    String outSceneGraph = PrependPropFlag(VegaScene::JSVarResultName);
    this->LoadJSTemplate(JSLIB_PATH(vegascene.render.js),
                         this->JSVarNamespace, render, outSceneGraph);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSFuncRun()
{
    String outSceneGraph = PrependPropFlag(VegaScene::JSVarResultName);
    String specContent = PrependPropFlag(VegaScene::JSVarSpecContentName);
    String render = PrependPropFlag(VegaScene::JSFuncRenderName);
    this->LoadJSTemplate(JSLIB_PATH(vegascene.run.js),
                         this->JSVarNamespace, outSceneGraph,
                         specContent, render);

}


//------------------------------------------------------------------------------
void VegaScene::SetJSObjConsole()
{
    String consoleBackend = this->GetQualifiedName("console");
    this->LoadJSTemplate(JSLIB_PATH(console.js), consoleBackend);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSObjContext2d()
{
    String context2dBackend = PrependPropFlag(VegaScene::JSVarContext2dName);
    this->LoadJSTemplate(JSLIB_PATH(vegascene.canvas.js),
                         this->JSVarNamespace, context2dBackend);

}


//------------------------------------------------------------------------------
void VegaScene::SetJSFuncDataLoad()
{
    String dataLoadBackend = this->GetQualifiedName(VegaScene::JSVarDataLoaderName);
    this->LoadJSTemplate(JSLIB_PATH(vega.data.load.js), dataLoadBackend);
}


//------------------------------------------------------------------------------
void VegaScene::DefineNamespace()
{
    QJSValue globalObject = this->Engine.globalObject();
    while (globalObject.hasProperty(QString::fromStdString(this->JSVarNamespace)))
    {
        this->JSVarNamespace += String("_");
    }
    QString varName = QString::fromStdString(this->JSVarNamespace);
    globalObject.setProperty(varName, this->Engine.newObject());
    this->SafeEvaluate(this->JSVarNamespace);
}


//------------------------------------------------------------------------------
void VegaScene::DefineProperty(const String& name)
{
    QString propName = QString::fromStdString(name);
    QJSValue globalObject = this->Engine.globalObject();
    QJSValue vs = globalObject.property(QString::fromStdString(this->JSVarNamespace));
    if (!vs.hasProperty(propName))
    {
        vs.setProperty(propName, this->Engine.newObject());
    }
}


//------------------------------------------------------------------------------
String VegaScene::GetQualifiedName( const char* name ) const
{
    return this->JSVarNamespace + String(".") + String(name);
}




//------------------------------------------------------------------------------
int vegascene(const String& specFilePath,
              const String& outFilePath,
              const String& baseURL)
{
    VegaScene vs;
    vs.SetBaseURL(baseURL);
#ifdef DEBUG
    std::cout << "Base URL: " << vs.GetBaseURL() << std::endl;
#endif
    vs.LoadSpecFromFile(specFilePath);
    vs.Render();
    return !vs.Write(outFilePath);
}
