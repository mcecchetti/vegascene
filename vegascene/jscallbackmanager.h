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
