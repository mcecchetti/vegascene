#ifndef VEGASCENE_H
#define VEGASCENE_H

#include "jscallbackmanager.h"
#include "jscontext2d.h"
#include "data.h"
#include "jsconsole.h"

#include <string>
#include <vector>


typedef std::string String;
typedef std::vector<String> ArgListType;



class JSModule
{
public:
    typedef std::vector<JSModule> DependenciesListType;

public:
    JSModule( const String& ns, const String& path )
        : NameSpace( ns ), FilePath( path )
    {
    }

    void AddRequiredModule( const JSModule& module )
    {
        Dependencies.push_back( module );
    }

public:
    String NameSpace;
    String FilePath;
    DependenciesListType Dependencies;
    String preLoadConfig;
    String postLoadConfig;
};




template< typename JSEngine >
class VegaScene
{
public:
    VegaScene();

    bool LoadSpec(const String& spec);

    bool LoadSpecFromFile(const String& filePath);

    const String& GetBaseURL() const;

    void SetBaseURL(const String& baseURL);

    bool Render();

    const String& GetResult();

    bool Write(const String& filePath = String());


private:
    bool SafeEvaluate(const String& code, String* result = NULL);

    template< typename T >
    bool InjectNonJSObject(T& object, String varName);

    bool LoadJSLib(const String& pathToLib);

    bool LoadJSModule(const JSModule& info, bool withDependencies = true);

    bool LoadJSTemplate(const String& filePath, const ArgListType & argList);
    bool LoadJSTemplate(const String& filePath,
                        const String& arg1 = String(),
                        const String& arg2 = String(),
                        const String& arg3 = String(),
                        const String& arg4 = String(),
                        const String& arg5 = String());

    bool ReadFile(const String& filePath, String& fileContent) const;

private:
    static String SetPreLoadConfig()
    {
        String sourcePath("../jslib/vega.preload.config.js");
        return sourcePath;
    }

    static String SetPostLoadConfig()
    {
        String sourcePath("../jslib/vega.postload.config.js");
        return sourcePath;
    }

    void SetJSFunctionSetTimeoutDef();
    void SetJSFunctionRenderDef();
    void setJSObjContext2d();
    void SetJSFuncDataLoad();
    void SetJSObjConsole();

    void DefineNamespace();
    void DefineProperty(const String& name);
    String GetQualifiedName( const char* name ) const;

private:
    static const char* JSVarCallbackManagerName;
    static const char* JSVarContext2dName;
    static const char* JSVarDataLoaderName;
    static const char* JSVarSpecContentName;
    static const char* JSVarResultName;
    static const char* JSFuncSetTimeoutName;
    static const char* JSFuncRenderName;

private:
    JSEngine Engine;
    String Result;
    bool EngineReady;
    bool Initialized;
    bool SpecLoaded;
    bool Rendered;
    JSModule JSModuleVega;
    JSCallbackManager CallbackManager;
    JSContext2d Context2d;
    Data DataLoader;
    String BaseURL;
    String JSVarNamespace;
    JSConsole Console;
};


template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarCallbackManagerName = "timeoutBackend";

template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarContext2dName = "context2d";

template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarDataLoaderName = "data";

template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarSpecContentName = "specContent";

template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarResultName = "outSceneGraph";

template< typename JSEngine >
const char* VegaScene<JSEngine>::JSFuncRenderName = "render";




int vegascene(const String& specFilePath,
              const String& outFilePath = String(""),
              const String& baseURL = String(""));

#endif // VEGASCENE_H
