#ifndef TEXT_STYLE_H
#define TEXT_STYLE_H

#include <QtGui/QColor>

class TerminalScreen;

class TextStyle
{
public:
    enum Style {
        Normal            = 0x0000,
        Italic            = 0x0001,
        Underlined        = 0x0002,
        Blinking          = 0x0004,
        FastBlinking      = 0x0008,
        Gothic            = 0x0010,
        DoubleUnderlined  = 0x0020,
        Framed            = 0x0040,
        Overlined         = 0x0080,
        Encircled         = 0x0100
    };

    TextStyle(Style style, const QColor forground);

    Style style;
    QColor forground;
    QColor background;

    bool isCompatible(const TextStyle &other) const;
};

#endif // TEXT_STYLE_H
