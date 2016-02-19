#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <QtCore/QVector>
#include <QtCore/QObject>

#include <QtGui/QColor>



class ColorPalette : public QObject
{
Q_OBJECT
    Q_PROPERTY(QColor defaultBackground READ defaultBackground NOTIFY defaultBackgroundColorChanged)
public:
    ColorPalette(QObject *parent = 0);

    enum Color {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        DefaultForeground,
        DefaultBackground,
        numberOfColors
    };
    Q_ENUM(Color)

    QColor color(Color color, bool bold = false) const;
    QColor normalColor(Color color) const;
    QColor lightColor(Color color) const;
    static QColor xtermColor(int index);
    static QRgb xtermRgb(int index);

    void setInverseDefaultColors(bool inverse);

    QColor defaultBackground() const;
signals:
    void changed();
    void defaultBackgroundColorChanged();

private:
    QVector<QColor> m_normalColors;
    QVector<QColor> m_lightColors;
    QVector<QColor> m_intenseColors;
    static QVector<QRgb> m_xtermColors;

    bool m_inverse_default;
};

#endif // COLOR_PALETTE_H
