#ifndef JSCONSOLE_H
#define JSCONSOLE_H


#include <QObject>
#include <QJSValue>
#include <QJSValueIterator>


class JSConsole : public QObject
{
    Q_OBJECT

public slots:
    void Log(const QJSValue & message) const;

    void View(const QString& name,
              const QJSValue & object,
              unsigned int maxDeep = 3) const;

private:
    void Indent(int n) const;
};



#endif // JSCONSOLE_H
