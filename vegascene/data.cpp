
#include "data.h"

#include <QFile>
#include <QIODevice>
#include <QtCore/qtextstream.h>
#include <QRegExp>
#include <QJSValueList>

#include <iostream>




//------------------------------------------------------------------------------
// Regular expression used to check if a given url is defined with a protocol
// or it is a simple file path.
const char* Data::LoadProtocolRE = "^[A-Za-z]+\\:\\/\\/.*";

// The protocol used for absolute file paths.
const char* Data::LoadFileProtocol = "file://";


//------------------------------------------------------------------------------
const QString& Data::GetBaseURL() const
{
    return this->BaseURL;
}


//------------------------------------------------------------------------------
void Data::SetBaseURL(const QString& baseURL)
{
    this->BaseURL = baseURL;
}


//------------------------------------------------------------------------------
void Data::Load(const QString& uri, const QJSValue& callback) const
{
    QString url = Data::HasProtocol(uri) ? uri : this->BaseURL + uri;
    if (Data::IsFile(url))
    {
        Data::LoadFile(url, callback);
    }
    else // At present only local data resources are handled.
    {
        std::cerr << "error: while executing Data::Load: uri ("
                  << uri.toStdString() << ") not supported." << std::endl;
    }
}


//------------------------------------------------------------------------------
// Internal function used for checking if the given url is defined with a
// protocol.
bool Data::HasProtocol(const QString& url)
{
    QRegExp re(Data::LoadProtocolRE);
    return re.exactMatch(url);
}


//------------------------------------------------------------------------------
// Internal function used for checking if a given url is defined with the file
// protocol (i.e., `file://`).
bool Data::IsFile(const QString& url)
{
    QString fileProtocol(Data::LoadFileProtocol);
    return url.startsWith(fileProtocol);
}


//------------------------------------------------------------------------------
void Data::LoadFile(const QString& url, const QJSValue& callback)
{
    std::cout << "log: LOAD FILE: " << url.toStdString() << std::endl;
    QString file = url;
    QString fileProtocol(Data::LoadFileProtocol);
    if (file.startsWith(fileProtocol))
    {
        file.remove(0, fileProtocol.length());
    }
    Data::ReadFile(file, callback);
}


//------------------------------------------------------------------------------
void Data::ReadFile(const QString& filePath, const QJSValue& callback)
{
#ifdef DEBUG
    std::cout << "log: Data::ReadFile: file path argument: "
              << filePath.toStdString() << std::endl;
#endif
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        std::cerr << "error: while executing Data::ReadFile: cannot open file: "
                  << filePath.toStdString() << std::endl;
        return;
    }
    QTextStream stream(&file);
    QString contents = stream.readAll();
    file.close();

    if (callback.isCallable())
    {
        // The following trick is needed since it is not possible to invoke
        // the `call` method for a const reference QJSValue.
        QJSValue callback_(callback);
        QJSValueList argList;
        argList.append(QJSValue::NullValue);
        argList.append(QJSValue(contents));
        callback_.call(argList);
    }
#ifdef DEBUG
    else
    {
        std::cerr << "error: while executing Data::ReadFile: "
                     "callback object is not callable.\n";
    }
#endif
}

