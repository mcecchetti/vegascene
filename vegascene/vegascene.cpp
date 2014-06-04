
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

#include <iostream>




//------------------------------------------------------------------------------
template< typename JSEngine >
VegaScene<JSEngine>::VegaScene()
    : Engine(),
      Result(),
      EngineReady(true),
      Initialized(false),
      SpecLoaded(false),
      Rendered(false),
      JSModuleVega("vg", "../jsmodules/vega.js"),
      CallbackManager(),
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
#ifdef WITH_JSLIBS
    JSModule d3GeoProjection("d3", "../jslib/d3.geo.projection.min.js");
    this->JSModuleVega.AddRequiredModule(d3GeoProjection);
#endif
    this->JSModuleVega.preLoadConfig = VegaScene::SetPreLoadConfig();
    this->JSModuleVega.postLoadConfig = VegaScene::SetPostLoadConfig();

    this->LoadJSModule(this->JSModuleVega);

    this->DefineNamespace();

    this->InjectNonJSObject(this->CallbackManager, VegaScene::JSVarCallbackManagerName);

    this->InjectNonJSObject(this->DataLoader, VegaScene::JSVarDataLoaderName);
    String source = String("if (vg.data.load !== undefined)");
    source += String("vg.data.load = ") + this->GetQualifiedName("data.Load;");
    this->SafeEvaluate(source);

    SetJSFunctionSetTimeoutDef();
    SetJSFunctionRenderDef();

    this->InjectNonJSObject(this->Console, "console");
    String program = String("printView = ") + this->GetQualifiedName("console.View");
    this->SafeEvaluate(program);
    if (this-> EngineReady)
    {
        program = String("console = {}; console.log = ") + this->GetQualifiedName("console.Log");
        this->SafeEvaluate(program);
    }

    this->Initialized = this-> EngineReady;
}


//------------------------------------------------------------------------------
template< typename JSEngine >
bool VegaScene<JSEngine>::LoadSpec(const String& spec)
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
template< typename JSEngine >
bool VegaScene<JSEngine>::LoadSpecFromFile(const String& filePath)
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
template< typename JSEngine >
const String& VegaScene<JSEngine>::GetBaseURL() const
{
    return this->BaseURL;
}


//------------------------------------------------------------------------------
template< typename JSEngine >
void VegaScene<JSEngine>::SetBaseURL(const String& baseURL)
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
template< typename JSEngine >
bool VegaScene<JSEngine>::Render()
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
template< typename JSEngine >
const String& VegaScene<JSEngine>::GetResult()
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
template< typename JSEngine >
bool VegaScene<JSEngine>::Write(const String& filePath)
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
                std::cerr << "error: can not write to file: '" << filePath << "'" << std::endl;
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
template< typename JSEngine >
bool VegaScene<JSEngine>::SafeEvaluate(const String& code, String* result)
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
template< typename JSEngine >
template< typename T >
bool VegaScene<JSEngine>::InjectNonJSObject(T& object, String varName)
{
    if( !(this->EngineReady) ) return false;
    QJSValue jsObject = this->Engine.newQObject(&object);
    QJSValue nsObject = this->Engine.globalObject().property(QString::fromStdString(this->JSVarNamespace));
    nsObject.setProperty(QString::fromStdString(varName), jsObject);
    String qVarName = this->GetQualifiedName(varName.c_str());
    return this->SafeEvaluate(qVarName);
}


//------------------------------------------------------------------------------
template< typename JSEngine >
bool VegaScene<JSEngine>::LoadJSModule(const JSModule& info, bool withDependencies)
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
        QJSValue result =  this->Engine.evaluate(QString::fromStdString(info.preLoadConfig));
        this->EngineReady = !result.isError();
        if (!this->EngineReady)
        {
#ifdef DEBUG
            std::cerr << "In loading JavaScript module with namespace '" << info.NameSpace << "'\n"
                      << "preload config error: uncaught exception: " << result.toString().toStdString() << std::endl;
#endif
        return false;
        }
    }

    String moduleSource;
    if (ReadFile(info.FilePath, moduleSource))
    {
        QJSValue result = this->Engine.evaluate(QString::fromStdString(moduleSource), QString::fromStdString(info.NameSpace));
        this->EngineReady = !result.isError();
        if (this->EngineReady)
        {
            this->EngineReady = this->Engine.globalObject().hasProperty(QString::fromStdString(info.NameSpace));
#ifdef DEBUG
            if (this->EngineReady)
            {
                std::cout << "JavaScript module with namespace '" + info.NameSpace + "' has been loaded successfully." << std::endl;
            }
            else
            {
                std::cerr << "error: JavaScript module with namespace '" << info.NameSpace << "' has been loaded but namespace is not present." << std::endl;

            }
#endif
            if (!info.postLoadConfig.empty())
            {
                QJSValue result =  this->Engine.evaluate(QString::fromStdString(info.postLoadConfig));
                this->EngineReady = !result.isError();
                if (!this->EngineReady)
                {
        #ifdef DEBUG
                    std::cerr << "In loading JavaScript module with namespace '" << info.NameSpace << "'\n"
                              << "postload config error: uncaught exception: " << result.toString().toStdString() << std::endl;
        #endif
                }
            }

        }
#ifdef DEBUG
        else
        {

            std::cerr << "In loading JavaScript module with namespace '" << info.NameSpace << "'\n"
                      << "error: uncaught exception: " << result.toString().toStdString() << std::endl;

        }
#endif
    }
    else
    {
        this->EngineReady = false;
    }
    return this->EngineReady;
}


//------------------------------------------------------------------------------
template< typename JSEngine >
bool VegaScene<JSEngine>::ReadFile(const String& filePath, String& fileContent) const
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
template< typename JSEngine >
void VegaScene<JSEngine>::SetJSFunctionSetTimeoutDef()
{
    String timeoutBackend = this->GetQualifiedName(VegaScene::JSVarCallbackManagerName);
    String funcDef;
    funcDef += String("function setTimeout(callback, delay){");
    funcDef += timeoutBackend + String(".Start(callback, delay); }");

    this->SafeEvaluate(funcDef);
}


//------------------------------------------------------------------------------
template< typename JSEngine >
void VegaScene<JSEngine>::SetJSFunctionRenderDef()
{
    String outSceneGraph = this->GetQualifiedName(VegaScene::JSVarResultName);
    String render = this->GetQualifiedName(VegaScene::JSFuncRenderName);
    String funcDef;
    funcDef += render + String(" = function(spec) {");
    funcDef += String("vg.headless.render(");
    funcDef += String("{spec: spec, renderer: 'scene'},");
    funcDef += String("function(err, data) {");
    funcDef += String("if (err) throw err;");
    funcDef += outSceneGraph + String(" = data.scene; } ); }");
    this->SafeEvaluate(funcDef);
}


//------------------------------------------------------------------------------
template< typename JSEngine >
void VegaScene<JSEngine>::DefineNamespace()
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
template< typename JSEngine >
String VegaScene<JSEngine>::GetQualifiedName( const char* name ) const
{
    return this->JSVarNamespace + String(".") + String(name);
}




//------------------------------------------------------------------------------
int vegascene(const String& specFilePath,
              const String& outFilePath,
              const String& baseURL)
{
    VegaScene<QJSEngine> vs;
    vs.SetBaseURL(baseURL);
#ifdef DEBUG
    std::cout << "Base URL: " << vs.GetBaseURL() << std::endl;
#endif
    vs.LoadSpecFromFile(specFilePath);
    vs.Render();
    return !vs.Write(outFilePath);
}
