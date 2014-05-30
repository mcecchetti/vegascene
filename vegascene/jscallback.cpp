
#include "jscallback.h"

#include <QJSValue>
#include <QThread>

#ifdef DEBUG
#include <iostream>
#endif


//------------------------------------------------------------------------------
JSCallback::JSCallback(const QJSValue& function, int delay)
    : JSFunction(function), Delay(delay), Wait(true)
{}


//------------------------------------------------------------------------------
void JSCallback::Call()
{
    if (this->JSFunction.isCallable())
    {
        QThread* cthread = QThread::currentThread();
        cthread->msleep(this->Delay);
        while(this->Wait)
        {
            cthread->msleep( 100 );
        }
        this->JSFunction.call();
#ifdef DEBUG
        std::cout << "callback executed" << std::endl;
#endif
    }
}


//------------------------------------------------------------------------------
void JSCallback::WakeUp()
{
    this->Wait = false;
}
