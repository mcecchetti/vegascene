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
