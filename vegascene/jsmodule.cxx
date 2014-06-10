/*=========================================================================

Program: Vegascene
Module: jsmodule.cxx

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/


#include "jsmodule.h"


//------------------------------------------------------------------------------
JSModule::JSModule(const String& ns, const String& path)
    : NameSpace(ns), FilePath(path)
{
}


//------------------------------------------------------------------------------
void JSModule::AddRequiredModule(const JSModule& module)
{
    this->Dependencies.push_back(module);
}
