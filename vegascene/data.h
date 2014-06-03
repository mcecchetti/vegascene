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
    QString BaseURL;
};

#endif // DATA_H
