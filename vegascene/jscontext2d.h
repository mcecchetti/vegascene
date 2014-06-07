#ifndef JSCONTEXT2D_H
#define JSCONTEXT2D_H


#include <QObject>
#include <QString>
#include <QFont>
#include <QJSEngine>
#include <QJSValue>



typedef QJSEngine JSEngine;


class JSContext2d : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString font READ GetFont WRITE SetFont)
    Q_PROPERTY(QString textAlign READ GetTextAlign WRITE SetTextAlign)
    Q_PROPERTY(QString textBaseline READ GetTextBaseline WRITE SetTextBaseline)
public:
    JSContext2d(JSEngine& engine);

    void SetFontFamily(const QString& value);
    void SetFontVariant(const QString& value);
    void SetFontStyle(const QString& value);
    void SetFontWeight(const QString& value);
    void SetFontSize(const QString& value);

    QString GetFont() const;
    void SetFont(const QString& value);

    const QString& GetTextAlign() const;
    void SetTextAlign(const QString& value);

    const QString& GetTextBaseline() const;
    void SetTextBaseline(const QString& value);

public slots:
    QJSValue measureText(const QString& text);

private:
    JSEngine& Engine;
    QFont Font;
    QString TextAlign;
    QString TextBaseline;

private:
    static const char* styles;
    static const char* variants;
    static const char* weights;
    static const char* sizes;
    static const char* units;
    static const char* name;
};


#endif // JSCONTEXT2D_H
