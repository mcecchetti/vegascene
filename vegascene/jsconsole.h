#ifndef JSCONSOLE_H
#define JSCONSOLE_H

#include <iostream>

#include <QtQml/qjsengine.h>
#include <QJSValueIterator>


class JSConsole : public QObject
{
    Q_OBJECT

public slots:
    void Log(const QJSValue & message) const
    {
        std::cout << message.toString().toStdString() << std::endl;
    }

    void View(const QString& name, const QJSValue & object, unsigned int maxDeep = 3) const
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

private:
    void Indent(int n) const
    {
        while(n--)
        {
            std::cout << "  ";
        }
    }
};



#endif // JSCONSOLE_H
