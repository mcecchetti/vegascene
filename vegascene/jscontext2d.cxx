/*=========================================================================

Program: Vegascene
Module: jscontext2d.cxx

Copyright (c) Marco Cecchetti
All rights reserved.
See Copyright.txt

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notice for more information.

=========================================================================*/


#include "jscontext2d.h"

#include <QFont>
#include <QFontMetrics>
#include <QRegExp>
#include <QStringList>
#include <QVariantMap>

#include <iostream>




//------------------------------------------------------------------------------
// Font properties legal values.
const char* JSContext2d::styles  = "normal|italic|oblique";
const char* JSContext2d::variants  = "normal|small-caps";
const char* JSContext2d::weights  = "normal|bold|bolder|lighter|[1-9]00";
const char* JSContext2d::sizes  = "xx-small|x-small|small|medium|large|x-large|xx-large";
const char* JSContext2d::units  = "px|pt|pc|in|cm|mm|%";
const char* JSContext2d::name  = "'[^']+'|\"[^\"]+\"|[\\w-]+";


//------------------------------------------------------------------------------
JSContext2d::JSContext2d()
    : Font()
{}


//------------------------------------------------------------------------------
void JSContext2d::SetFontFamily(const QString& value)
{
    if (value.isEmpty()) return;
    if (value == "serif") {
        this->Font.setStyleHint(QFont::Serif);
    } else if (value == "sans-serif") {
        this->Font.setStyleHint(QFont::SansSerif);
    } else if (value == "cursive") {
        this->Font.setStyleHint(QFont::Cursive);
    } else if (value == "fantasy") {
        this->Font.setStyleHint(QFont::Fantasy);
    } else if (value == "monospace") {
        this->Font.setStyleHint(QFont::Monospace);
    }

    this->Font.setFamily(value);
}


//------------------------------------------------------------------------------
void JSContext2d::SetFontVariant(const QString& value)
{
    this->Font.setCapitalization(QFont::MixedCase);
    if (value.isEmpty()) return;
    if (value == "small-caps") {
        this->Font.setCapitalization(QFont::SmallCaps);
    }
}


//------------------------------------------------------------------------------
void JSContext2d::SetFontStyle(const QString& value)
{
    this->Font.setStyle(QFont::StyleNormal);
    if (value.isEmpty()) return;
    if (value == "italic") {
        this->Font.setStyle(QFont::StyleItalic);
    } else if (value == "oblique") {
        this->Font.setStyle(QFont::StyleOblique);
    }
}


//------------------------------------------------------------------------------
void JSContext2d::SetFontWeight(const QString& value)
{
    this->Font.setWeight(QFont::Normal);
    if (value.isEmpty()) return;
    if (value == "bold") {
        this->Font.setWeight(QFont::Bold);
    } else if (value == "100") {
        this->Font.setWeight(12);
    } else if (value == "200") {
        this->Font.setWeight(25);
    } else if (value == "300") {
        this->Font.setWeight(37);
    } else if (value == "400") {
        this->Font.setWeight(QFont::Normal);
    } else if (value == "500") {
        this->Font.setWeight(62);
    } else if (value == "600") {
        this->Font.setWeight(75);
    } else if (value == "700") {
        this->Font.setWeight(87);
    } else if (value == "800") {
        this->Font.setWeight(99);
    } else if (value == "900") {
        this->Font.setWeight(99);
    }
}


//------------------------------------------------------------------------------
void JSContext2d::SetFontSize(const QString& value)
{
    this->Font.setPointSizeF(12);

    QString pattern;
    pattern += "([\\d\\.]+)(";
    pattern += JSContext2d::units;
    pattern += ")|(";
    pattern += JSContext2d::sizes;
    pattern += ")";

    QRegExp re(pattern);
    if (!re.exactMatch(value)) return;

    double size;
    QStringList captures = re.capturedTexts();
    if (!captures[1].isEmpty()) { // number + unit (e.g. `12pt`)
        size = captures[1].toDouble();
        const QString& unit = captures[2];
        if (unit == "in") {
            size *= 72.0;
        } else if (unit == "cm") {
            size *= (72.0/2.54);
        } else if (unit == "mm") {
            size *= (72.0/254.0);
        } else if (unit == "pc") {
            size *= 12.0;
        } else if (unit == "px") {
            size *= 0.75;
        }
    } else if (!captures[3].isEmpty()) { // predefined label (e.g. `x-large`)
        const QString& sz = captures[3];
        if (sz == "xx-small") {
            size = 7;
        } else if (sz == "x-small") {
            size = 8;
        } else if (sz == "small") {
            size = 9;
        } else if (sz == "medium") {
            size = 12;
        } else if (sz == "large") {
            size = 14;
        } else if (sz == "x-large") {
            size = 18;
        } else if (sz == "xx-large") {
            size = 24;
        }
    }

    this->Font.setPointSizeF(size);
}


//------------------------------------------------------------------------------
// We do not really use this method.
QString JSContext2d::GetFont() const
{
    return this->Font.family();
}


//------------------------------------------------------------------------------
void JSContext2d::SetFont(const QString& value)
{
    this->SetFontWeight("normal");
    this->SetFontStyle("normal");
    this->SetFontVariant("normal");


    QString optional;
    optional += "(?:(?:(";
    optional += JSContext2d::weights;
    optional += ")|(";
    optional += JSContext2d::styles;
    optional += ")|(";
    optional += JSContext2d::variants;
    optional += ")) *)?";

    // pattern for matching the `font` property of `context2d`
    QString pattern;
    // font weight, style and variant
    pattern += "^ *";
    pattern += optional;
    pattern += optional;
    pattern += optional;
    // font size
    pattern += "(?:((?:[\\d\\.]+";
    pattern += "(?:";
    pattern += JSContext2d::units;
    pattern += "))";
    pattern += "|";
    pattern += "(?:";
    pattern += JSContext2d::sizes;
    pattern += ")) *)";
    // font family list
    pattern += "((?:";
    pattern += JSContext2d::name;
    pattern += ")(?: *, *(?:";
    pattern += JSContext2d::name;
    pattern += "))*)";

    QRegExp re(pattern);
    bool isValid = re.exactMatch(value);
    QStringList captures = re.capturedTexts();

#ifdef DEBUG_FONT_PROPERTY
    std::cout << "font property is "
              << ((isValid ? "valid" : "not valid")) << "\n";
    for (int i = 0; i < captures.size(); ++i) {
        std::cout << i << ": >" << captures.at(i).toStdString() << "<\n";
    }
#endif

    if (!isValid) return;

    // There are 3 optional font properties: weight, style and variant.
    // We do not not in which order they are specified, so we have 3 captures
    // for each font property: captures 1, 4, 7 if not empty define a weight
    // property; captures 2, 5, 8 if not empty define a style property; finally
    // captures 3, 6, 9 if not empty define a variant property.
    // In case more than a single property of the same type is specified the
    // value of the last one is used for setting the related font property.
    for (int i = 0; i < 9; i+=3)
    {
        if (!captures[1+i].isEmpty()) {
            this->SetFontWeight(captures[1+i]);
        } else if (!captures[2+i].isEmpty()) {
            this->SetFontStyle(captures[2+i]);
        } else if (!captures[3+i].isEmpty()) {
            this->SetFontVariant(captures[3+i]);
        }
    }

    if (!captures[10].isEmpty()) {
        this->SetFontSize(captures[10]);
    }

    if (!captures[11].isEmpty()) {
        QString family = captures[11].split(',')[0];
        family.remove(QRegExp("^ *[\"'']"));
        family.remove(QRegExp("[\"''] *$"));
#ifdef DEBUG_FONT_PROPERTY
        std::cout << "font family: >" << family.toStdString() << "<\n";
#endif
        this->SetFontFamily(family);
    }
#ifdef DEBUG_FONT_PROPERTY
    std::cout << "font: " << this->Font.toString().toStdString() << std::endl;
#endif
}


//------------------------------------------------------------------------------
const QString& JSContext2d::GetTextAlign() const
{
    return this->TextAlign;
}


//------------------------------------------------------------------------------
void JSContext2d::SetTextAlign(const QString& value)
{
    this->TextAlign = value;
}


//------------------------------------------------------------------------------
const QString& JSContext2d::GetTextBaseline() const
{
    return this->TextBaseline;
}


//------------------------------------------------------------------------------
void JSContext2d::SetTextBaseline(const QString& value)
{
    this->TextBaseline = value;
}


//------------------------------------------------------------------------------
QVariantMap JSContext2d::measureText(const QString& text)
{
    int textWidth = 0;
    if (!text.isEmpty())
    {
        QFontMetrics fm(this->Font);
        textWidth = fm.width(text);
    }
#if 0
    std::cout << "text: >" << text.toStdString()
              << "<, width: " << textWidth << "\n";
#endif
    QVariantMap textExtents;
    textExtents.insert("width", textWidth);
    return textExtents;
}
