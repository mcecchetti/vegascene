/*=========================================================================

Program: Vegascene
Module: jsconsole.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME JSConsole - class used as a back-end by the JavaScript Vega module
// for the `console` object.
// .SECTION Description
// This class is used as a C++ back-end by the JavaScript Vega module,
// for the `console` object. It provides log capabilities and can be used as
// an heler class for debugging goals.


#ifndef JSCONSOLE_H
#define JSCONSOLE_H


#include <QObject>
#include <QJSValue>



class JSConsole : public QObject
{
    Q_OBJECT

public slots:
    void Log(const QJSValue & message) const;

    void View(const QString& name,
              const QJSValue & object,
              unsigned int maxDeep = 3) const;

};



#endif // JSCONSOLE_H
