
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
      JSModuleVega("vg", "../jsmodules/vega.js"),
      CallbackManager(),
      Context2d(Engine),
      DataLoader(),
      BaseURL(""),
      JSVarNamespace("vegascene"),
      Console()
{
    JSModule d3("d3", "../jsmodules/d3.js");
    this->JSModuleVega.AddRequiredModule(d3);
#ifdef WITH_TOPOJSON
    JSModule tj("topojson", "../jsmodules/topojson.js");
    this->JSModuleVega.AddRequiredModule(tj);
#endif
    this->JSModuleVega.preLoadConfig = VegaScene::SetPreLoadConfig();
    this->JSModuleVega.postLoadConfig = VegaScene::SetPostLoadConfig();
    this->LoadJSModule(this->JSModuleVega);

#ifdef WITH_EXAMPLE_LIBS
    this->LoadJSLib("../jslib/d3.geo.projection.min.js");
#endif

    this->DefineNamespace();

    this->InjectNonJSObject(this->Console, "console");
    SetJSObjConsole();

    this->InjectNonJSObject(this->CallbackManager, VegaScene::JSVarCallbackManagerName);
    SetJSFunctionSetTimeoutDef();

    this->InjectNonJSObject(this->Context2d, VegaScene::JSVarContext2dName);
    SetJSObjContext2d();

    this->InjectNonJSObject(this->DataLoader, VegaScene::JSVarDataLoaderName);
    SetJSFuncDataLoad();

    SetJSFunctionRenderDef();

    this->Initialized = this-> EngineReady;
}


//------------------------------------------------------------------------------
bool VegaScene::LoadSpec(const String& spec)
{
    this->SpecLoaded = false;
    if (!(this->Initialized)) return false;

    QJSValue nsObject = this->Engine.globalObject().property(QString::fromStdString(this->JSVarNamespace));
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
    if ( !(this->Initialized) ) return false;

    String spec;
    if (ReadFile(filePath, spec))
    {
        if (spec.empty())
        {
            spec = String("{}");
        }
        return LoadSpec(spec);
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

    String qJSVarResult = this->GetQualifiedName(VegaScene::JSVarResultName);
    String qJSVarSpecContent = this->GetQualifiedName(VegaScene::JSVarSpecContentName);
    String qJSFuncRender = this->GetQualifiedName(VegaScene::JSFuncRenderName);
    String qJSFuncRun = this->GetQualifiedName("run");

    // program:
    // vegascene.run = function(){
    //      vegascene.outSceneGraph = 0;
    //      var spec = JSON.parse(vegascene.specContent);
    //      render(spec);
    // };
    // vegascene.run();
    String program;
    program += qJSFuncRun + String(" = function(){");
    program += qJSVarResult + String(" = 0; ");
    program += String("var spec = JSON.parse(") + qJSVarSpecContent + String("); ");
    program += qJSFuncRender + String("(spec); };");
    program += qJSFuncRun + String("();");
    this->Rendered = this->SafeEvaluate(program);
    CallbackManager.WakeAll();
#ifdef DEBUG
    if (this->Rendered)
    {
        std::cout << "Rendered!" << std::endl;
    }
#endif
    return this->Rendered;
}


//------------------------------------------------------------------------------
const String& VegaScene::GetResult()
{
    if (this->Rendered)
    {
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
template< typename T >
bool VegaScene::InjectNonJSObject(T& object, String varName)
{
    if( !(this->EngineReady) ) return false;
    QJSValue jsObject = this->Engine.newQObject(&object);
    QJSValue nsObject = this->Engine.globalObject().property(QString::fromStdString(this->JSVarNamespace));
    nsObject.setProperty(QString::fromStdString(varName), jsObject);
    String qVarName = this->GetQualifiedName(varName.c_str());
    return this->SafeEvaluate(qVarName);
}


//------------------------------------------------------------------------------
bool VegaScene::LoadJSLib(const String& pathToLib)
{
    if ( !(this->EngineReady) ) return false;

    String libSource;
    this->EngineReady = ReadFile(pathToLib, libSource);
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
bool VegaScene::LoadJSModule(const JSModule& info, bool withDependencies)
{
    if ( !(this->EngineReady) ) return false;
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
            QJSValue result = this->Engine.evaluate(QString::fromStdString(moduleSource));
            this->EngineReady = !result.isError();
            if (this->EngineReady)
            {
                this->EngineReady = this->Engine.globalObject().hasProperty(QString::fromStdString(info.NameSpace));
#ifdef DEBUG
                if (this->EngineReady)
                {
                    std::cout << "JavaScript module with namespace '"
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
                if (!info.postLoadConfig.empty())
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
                std::cerr << "In loading JavaScript template: " << filePath << "\n"
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
    this->EngineReady = !templateFunc.isError() && templateFunc.isCallable();
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
                std::cerr << "In loading JavaScript template: " << filePath << "\n"
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
    String sourcePath("../jslib/vega.preload.config.js");
    return sourcePath;
}


//------------------------------------------------------------------------------
String VegaScene::SetPostLoadConfig()
{
    String sourcePath("../jslib/vega.postload.config.js");
    return sourcePath;
}



//------------------------------------------------------------------------------
String PrependPropFlag(const char* name)
{
    String flaggedName = String(".") + String(name);
    return flaggedName;
}


//------------------------------------------------------------------------------
void VegaScene::SetJSFunctionSetTimeoutDef()
{
    String timeoutBackend = this->GetQualifiedName(VegaScene::JSVarCallbackManagerName);
    this->LoadJSTemplate("../jslib/setTimeout.js", timeoutBackend);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSFunctionRenderDef()
{
    String render = PrependPropFlag(VegaScene::JSFuncRenderName);
    String outSceneGraph = PrependPropFlag(VegaScene::JSVarResultName);
    this->LoadJSTemplate("../jslib/vegascene.render.js",
                         this->JSVarNamespace, render, outSceneGraph);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSObjConsole()
{
    String consoleBackend = this->GetQualifiedName("console");
    this->LoadJSTemplate("../jslib/console.js", consoleBackend);
}


//------------------------------------------------------------------------------
void VegaScene::SetJSObjContext2d()
{
    String context2dBackend = PrependPropFlag(VegaScene::JSVarContext2dName);
    this->LoadJSTemplate("../jslib/vegascene.canvas.js",
                         this->JSVarNamespace, context2dBackend);

}


//------------------------------------------------------------------------------
void VegaScene::SetJSFuncDataLoad()
{
    String dataLoadBackend = this->GetQualifiedName(VegaScene::JSVarDataLoaderName);
    this->LoadJSTemplate("../jslib/vega.data.load.js", dataLoadBackend);
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
