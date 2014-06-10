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
// .NAME VegaScene - main class for reading Vega spec files and generating
// the scene graph
// .SECTION Description
// This class is used for reading Vega spec files and generating the related
// scene graph in JSON format. The generated scene graph can be get as a string
// or written directly to a file.


#ifndef VEGASCENE_H
#define VEGASCENE_H

#include "jscallbackmanager.h"
#include "jscontext2d.h"
#include "data.h"
#include "jsconsole.h"
#include "jsmodule.h"

#include <QJSEngine>

#include <string>
#include <vector>

// Provide, at interface level, a bit of abstraction on the string type used.
typedef std::string String;

class VegaScene
{
private:
    // The type of JavaScript engine used internally.
    typedef QJSEngine JSEngine;

public:
    // Description:
    // Create an instance of VegaScene. JavaScript modules and fragments
    // loading, C++ back-ends creation and injection are performed at this
    // stage.
    VegaScene();

    // Description:
    // Load the Vega spec from a string into the JavaScript engine.
    // Return true if the operation is successfully, else return false.
    bool LoadSpec(const String& spec);

    // Description:
    // Load the Vega spec from a file into the JavaScript engine.
    // Return true if the operation is successfully, else return false.
    bool LoadSpecFromFile(const String& filePath);

    // Description:
    // Return the url utilized as base path for finding data resources,
    // referenced by the Vega spec file.
    const String& GetBaseURL() const;

    // Description:
    // Specify the url utilized as base path for finding data resources,
    // referenced by the Vega spec file.
    void SetBaseURL(const String& baseURL);

    // Description:
    // Generate the scene graph related to the currently loaded Vega spec file.
    // Return true if the operation is successfully, else return false.
    bool Render();

    // Description:
    // Return the scene graph, related to the last rendering, in JSON format
    // as a string.
    const String& GetResult();

    // Description:
    // Write the scene graph, related to the last rendering, in JSON format
    // to a file. If the passed file path is empty the scene graph is sent to
    // the standard output.
    bool Write(const String& filePath = String());

private:
    typedef std::vector<String> ArgListType;

    bool SafeEvaluate(const String& code, String* result = NULL);

    template< typename T >
    bool InjectNonJSObject(T& object, String varName);

    bool LoadJSLib(const String& pathToLib);

    bool LoadJSModule(const JSModule& info, bool withDependencies = true);

    bool LoadJSTemplate(const String& filePath, const ArgListType& argList);
    bool LoadJSTemplate(const String& filePath,
                        const String& arg1 = String(),
                        const String& arg2 = String(),
                        const String& arg3 = String(),
                        const String& arg4 = String(),
                        const String& arg5 = String());

    bool ReadFile(const String& filePath, String& fileContent) const;

    static String SetPreLoadConfig();
    static String SetPostLoadConfig();

    void SetJSFuncSetTimeout();
    void SetJSFuncRender();
    void SetJSObjContext2d();
    void SetJSFuncDataLoad();
    void SetJSObjConsole();
    void SetJSFuncRun();

    void DefineNamespace();
    void DefineProperty(const String& name);
    String GetQualifiedName( const char* name ) const;

private:
    // Description:
    // Names of JavaScript variables, representing objects or functions,
    // which will be defined in the JavaScript engine context.
    static const char* JSVarCallbackManagerName;
    static const char* JSVarContext2dName;
    static const char* JSVarDataLoaderName;
    static const char* JSVarSpecContentName;
    static const char* JSVarResultName;
    static const char* JSFuncSetTimeoutName;
    static const char* JSFuncRenderName;

private:
    // The JavaScript engine instance.
    JSEngine Engine;

    // The generated scene graph in JSON format
    String Result;

    // A status attribute flag used for signaling if an error occurred.
    bool EngineReady;

    // A status attribute flag used for signaling if initialization has been
    // performed.
    bool Initialized;

    // A status attribute flag used for signaling if spec file has been loaded.
    bool SpecLoaded;

    // A status attribute flag used for signaling if the scene graph has been
    // generated successfully.
    bool Rendered;

    // A structure containing information for loading the Vega module.
    JSModule JSModuleVega;

    // A C++ object used as a back-end for the `(window.)setTimeout` function.
    JSCallbackManager CallbackManager;

    // A C++ object used as a back-end for the `context2d.measureText` function.
    JSContext2d Context2d;

    // A C++ object used as a back-end for loading data resources.
    Data DataLoader;

    // A base url used for retrieving data resources.
    String BaseURL;

    // The vegascene namespace.
    String JSVarNamespace;

    // A C++ object used as a back-end for the `console.log` funtion.
    JSConsole Console;
};


// Description:
// Write the scene graph generated from the passed Vega spec file to a file or
// to the standard output if passed the output file path is empty.
// The output content will be in JSON format. An URL used as base path for
// finding data resources referenced in the spec file, can be passed as third
// argument.
int vegascene(const String& specFilePath,
              const String& outFilePath = String(""),
              const String& baseURL = String(""));

#endif // VEGASCENE_H
