#ifndef VEGASCENE_H
#define VEGASCENE_H

#include "jscallbackmanager.h"
#include "jsconsole.h"

#include <string>


typedef std::string String;


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

    const String& GetBaseURL();

    void SetBaseURL(const String& baseURL);

    bool Render();

    const String& GetResult();

    bool Write(const String& filePath = String());


private:
    bool SafeEvaluate(const String& code, String* result = NULL);

    template< typename T >
    bool InjectNonJSObject(T& object, String varName);

    bool LoadJSModule(const JSModule& info, bool withDependencies = true);

    bool ReadFile(const String& filePath, String& fileContent) const;

    /**
     * if (d3 !== undefined) {
     *     if (d3.svg === undefined) {
     *        function d3_svg() {};
     *        var prot = d3_svg.prototype;
     *        prot.self = function() { return this };
     *        prot.arc = prot.area = prot.line = prot.symbol
     *            = prot.x = prot.y = prot.y0 = prot.y1
     *            = prot.type = prot.size = prot.self;
     *        d3.svg = new d3_svg();
     *     }
     * }
     */
    static String SetPreLoadConfig()
    {
        String source;
        source += String("if (d3 !== undefined) {");
        source += String("if (d3.svg === undefined) {");
        source += String("function d3_svg() {};");
        source += String("var prot = d3_svg.prototype;");
        source += String("prot.self = function() { return this };");
        source += String("prot.arc = prot.area = prot.line = prot.symbol ");
        source += String("= prot.x = prot.y = prot.y0 = prot.y1 ");
        source += String("= prot.type = prot.size = prot.self;");
        source += String("d3.svg = new d3_svg(); } }");

        return source;
    }

    /**
     * if (vg.scene.bounds.mark !== undefined)
     *     vg.scene.bounds.mark = function() {};
     * if (vg.headless.View.prototype.autopad !== undefined)
     *     vg.headless.View.prototype.autopad = function() { return this; };
     */
    static String SetPostLoadConfig()
    {
        String source;
        source += String("if (vg.scene.bounds.mark !== undefined) ");
        source += String("vg.scene.bounds.mark = function() {};");
        source += String("if (vg.headless.View.prototype.autopad !== undefined) ");
        source += String("vg.headless.View.prototype.autopad = function() { return this; };");
        return source;
    }

    /**
     * function setTimeout(callback, delay){
     *     timeoutBackend.start(callback, delay);
     * }
     *
     */
    void SetJSFunctionSetTimeoutDef();

    /**
     * function render(spec) {
     *     vg.headless.render(
     *         {spec: spec, renderer: 'scene'},
     *         function(err, data) {
     *             if (err) throw err;
     *                 outSceneGraph = data.scene;
     *         }
     *     );
     *  }
     *
     */
    void SetJSFunctionRenderDef();

    void DefineNamespace();

    String GetQualifiedName( const char* name ) const;

private:
    static const char* JSVarCallbackManagerName;
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
    String BaseURL;
    String JSVarNamespace;
    JSConsole Console;
};


template< typename JSEngine >
const char* VegaScene<JSEngine>::JSVarCallbackManagerName = "timeoutBackend";

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
