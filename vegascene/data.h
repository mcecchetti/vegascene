/*=========================================================================

Program: Vegascene
Module: data.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME Data - class used for handling data resource loading
// .SECTION Description
// This class is used as a C++ back-end by the JavaScript Vega module,
// for loading data resources referenced in spec files.


#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QString>
#include <QJSValue>




class Data : public QObject
{
    Q_OBJECT

public:
    const QString& GetBaseURL() const;

    void SetBaseURL(const QString& baseURL);

public slots:
    // Description:
    // Read a data resource file referenced through the passed uri and invoke
    // the passed callback with argument the data file content.
    void Load(const QString& uri, const QJSValue& callback) const;

private:
    static bool HasProtocol(const QString& url);

    static bool IsFile(const QString& url);

    static void LoadFile(const QString& url, const QJSValue& callback);

    static void ReadFile(const QString& filePath, const QJSValue& callback);

public:
    static const char* LoadProtocolRE;
    static const char* LoadFileProtocol;

private:
    // A base url used for retrieving data resources.
    QString BaseURL;
};

#endif // DATA_H
