/*=========================================================================

Program: Vegascene
Module: jsmodule.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME JSModule - keep the needed information for loading a JavaScript module.
// .SECTION Description
// This class is used for defining the needed information for loading a
// JavaScript module. It is possible to define all modules the given module
// depends on. Finally it is possible to specify path to JavaScript files used
// for the module's pre loading configuration and for the module's post loading
// configuration.


#ifndef JSMODULE_H
#define JSMODULE_H


#include <string>
#include <vector>




typedef std::string String;


class JSModule
{
public:
    typedef std::vector<JSModule> DependenciesListType;

public:
    JSModule(const String& ns, const String& path);

    void AddRequiredModule(const JSModule& module);

public:
    String NameSpace;
    String FilePath;
    DependenciesListType Dependencies;
    String preLoadConfig;
    String postLoadConfig;
};


#endif // JSMODULE_H
