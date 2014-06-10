/*=========================================================================

Program: Vegascene
Module: jscallback.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME JSCallback - class used for invoking a JavaScript function after a
// given delay.
// .SECTION Description
// This class is used as a C++ back-end by the JavaScript Vega module,
// for executing a JavaScript function after a given delay.


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
