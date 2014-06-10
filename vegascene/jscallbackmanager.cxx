/*=========================================================================

Program: Vegascene
Module: jscallbackmanager.cxx

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/


#include "jscallbackmanager.h"

#include "jscallback.h"

#include <QJSValue>
#include <QtConcurrent>
#include <QFutureWatcher>



//------------------------------------------------------------------------------
JSCallbackManager::JSCallbackManager()
    : Finished(true),
      Watchers()
{}


//------------------------------------------------------------------------------
JSCallbackManager::~JSCallbackManager()
{
    this->WaitForFinished();
}


//------------------------------------------------------------------------------
void JSCallbackManager::WaitForFinished()
{
    if ( !(this->Finished) )
    {
        typedef JSCallbackManager::WatcherContainer::const_iterator CIterator;
        CIterator it = this->Watchers.constBegin();
        while (it != this->Watchers.constEnd())
        {
            if (*it != NULL)
            {
                (*it)->waitForFinished();
            }
            ++it;
        }
        this->Finished = true;
    }
}


//------------------------------------------------------------------------------
void JSCallbackManager::Start(const QJSValue func, int delay)
{
    if (func.isCallable())
    {
        this->Finished = false;
        JSCallback* callback = new JSCallback(func, delay);
        connect( this, SIGNAL(WakeAll()), callback, SLOT(WakeUp()) );
        QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
        connect( watcher, SIGNAL(finished()), callback, SLOT(deleteLater()) );
        connect( watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()) );
        this->Watchers.append( watcher );
        watcher->setFuture( QtConcurrent::run( callback, &JSCallback::Call ) );
    }
}
