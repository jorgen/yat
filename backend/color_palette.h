#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <QtCore/QVector>

#include <QtGui/QColor>

class ColorPalette
{
public:
    ColorPalette();


    enum Color {
        black,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        defaultForground,
        defaultBackground,
        numberOfColors
    };

    QColor normalColor(Color color) const;
    QColor lightColor(Color color) const;
    QColor intenseColor(Color color) const;

private:
    QVector<QColor> m_normalColors;
    QVector<QColor> m_lightColors;
    QVector<QColor> m_intenseColors;
};

#endif // COLOR_PALETTE_H
