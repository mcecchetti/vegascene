/*=========================================================================

Program: Vegascene
Module: jscontext2d.h

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/
// .NAME JSContext2d - class used as a back-end by the JavaScript Vega module,
// .SECTION Description
// This class is used as a C++ back-end by the JavaScript Vega module,
// for supporting the `context2d.measureText` routine, used for computing text
// width.


#ifndef JSCONTEXT2D_H
#define JSCONTEXT2D_H


#include <QObject>
#include <QString>
#include <QFont>
#include <QVariantMap>




class JSContext2d : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString font READ GetFont WRITE SetFont)
    Q_PROPERTY(QString textAlign READ GetTextAlign WRITE SetTextAlign)
    Q_PROPERTY(QString textBaseline READ GetTextBaseline WRITE SetTextBaseline)
public:
    JSContext2d();

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

    // Description:
    // Compute the width of the passed text. It needs that an instance of
    // QGuiApplication (or of a derived class) exists before being invoked.
    // Returns an instance of QVariantMap, which has a "width" key with
    // associated the computed text value.
    // Note: a QVariantMap instance is mapped by the JavaScript engine to
    // a JavaScript object whose properties are the map keys.
    Q_INVOKABLE QVariantMap measureText(const QString& text);

private:
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
