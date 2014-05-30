#ifndef JSCALLBACK_H
#define JSCALLBACK_H


#include <QObject>
#include <QJSValue>


class JSCallback : public QObject
{
    Q_OBJECT
public:
    JSCallback(const QJSValue& function, int delay);

public slots:
    void Call();

    void WakeUp();

private:
    QJSValue JSFunction;
    int Delay;
    bool Wait;
};


#endif // JSCALLBACK_H
