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

private:
    static String SetPreLoadConfig();
    static String SetPostLoadConfig();

    void SetJSFunctionSetTimeoutDef();
    void SetJSFunctionRenderDef();
    void SetJSObjContext2d();
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
