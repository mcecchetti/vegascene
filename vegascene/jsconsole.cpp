
#include "jsconsole.h"

#include <iostream>




//------------------------------------------------------------------------------
void JSConsole::Log(const QJSValue & message) const
{
    std::cout << message.toString().toStdString() << std::endl;
}


//------------------------------------------------------------------------------
void JSConsole::View(const QString& name,
                     const QJSValue& object,
                     unsigned int maxDeep) const
{
    static unsigned int deep = 1;
    if (deep > maxDeep) return;
    ++deep;

    this->Indent(deep-2);
    if (object.isObject())
    {
        std::cout << name.toStdString() << ": {" << std::endl;
        QJSValueIterator it(object);
        while (it.hasNext())
        {
            it.next();
            this->View(it.name(), it.value(), maxDeep);
        }
        this->Indent(deep-2);
        std::cout << "}" << std::endl;
    }
    else
    {
        std::cout << name.toStdString()
                  << ": " << object.toString().toStdString() << std::endl;
    }
    --deep;
}


//------------------------------------------------------------------------------
void JSConsole::Indent(int n) const
{
    while(n--)
    {
        std::cout << "  ";
    }
}
