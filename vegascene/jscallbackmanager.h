/*=========================================================================

Program: Vegascene
Module: jscallbackmanager.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME JSCallbackManager - class used for managing the asyncronous invocation
// of JavaScript functions.

// .SECTION Description
// This class is used as a C++ back-end by the JavaScript Vega module,
// for managing the asyncronous execution of JavaScript functions after a given
// delay.


#ifndef JSCALLBACKMANAGER_H
#define JSCALLBACKMANAGER_H


#include <QList>
#include <QJSValue>
#include <QtConcurrent>
#include <QFutureWatcher>


class JSCallbackManager : public QObject
{
    Q_OBJECT

    typedef QList<QFutureWatcher<void>*> WatcherContainer;

public:
    JSCallbackManager();

    ~JSCallbackManager();

    void WaitForFinished();

public slots:
    void Start(const QJSValue func, int delay);

signals:
    void WakeAll();

private:
    bool Finished;
    WatcherContainer Watchers;
};


#endif // JSCALLBACKMANAGER_H
