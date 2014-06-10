
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
