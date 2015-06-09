#include "color_palette.h"

#include <QDebug>

QVector<QRgb> ColorPalette::m_xtermColors {
    QColor(0,0,0).rgb(),    // index 16
    QColor(0,0,95).rgb(),
    QColor(0,0,135).rgb(),
    QColor(0,0,175).rgb(),
    QColor(0,0,215).rgb(),
    QColor(0,0,255).rgb(),
    QColor(0,95,0).rgb(),
    QColor(0,95,95).rgb(),
    QColor(0,95,135).rgb(),
    QColor(0,95,175).rgb(),
    QColor(0,95,215).rgb(),
    QColor(0,95,255).rgb(),
    QColor(0,135,0).rgb(),
    QColor(0,135,95).rgb(),
    QColor(0,135,135).rgb(),
    QColor(0,135,175).rgb(),
    QColor(0,135,215).rgb(),
    QColor(0,135,255).rgb(),
    QColor(0,175,0).rgb(),
    QColor(0,175,95).rgb(),
    QColor(0,175,135).rgb(),
    QColor(0,175,175).rgb(),
    QColor(0,175,215).rgb(),
    QColor(0,175,255).rgb(),
    QColor(0,215,0).rgb(),
    QColor(0,215,95).rgb(),
    QColor(0,215,135).rgb(),
    QColor(0,215,175).rgb(),
    QColor(0,215,215).rgb(),
    QColor(0,215,255).rgb(),
    QColor(0,255,0).rgb(),
    QColor(0,255,95).rgb(),
    QColor(0,255,135).rgb(),
    QColor(0,255,175).rgb(),
    QColor(0,255,215).rgb(),
    QColor(0,255,255).rgb(),
    QColor(95,0,0).rgb(),
    QColor(95,0,95).rgb(),
    QColor(95,0,135).rgb(),
    QColor(95,0,175).rgb(),
    QColor(95,0,215).rgb(),
    QColor(95,0,255).rgb(),
    QColor(95,95,0).rgb(),
    QColor(95,95,95).rgb(),
    QColor(95,95,135).rgb(),
    QColor(95,95,175).rgb(),
    QColor(95,95,215).rgb(),
    QColor(95,95,255).rgb(),
    QColor(95,135,0).rgb(),
    QColor(95,135,95).rgb(),
    QColor(95,135,135).rgb(),
    QColor(95,135,175).rgb(),
    QColor(95,135,215).rgb(),
    QColor(95,135,255).rgb(),
    QColor(95,175,0).rgb(),
    QColor(95,175,95).rgb(),
    QColor(95,175,135).rgb(),
    QColor(95,175,175).rgb(),
    QColor(95,175,215).rgb(),
    QColor(95,175,255).rgb(),
    QColor(95,215,0).rgb(),
    QColor(95,215,95).rgb(),
    QColor(95,215,135).rgb(),
    QColor(95,215,175).rgb(),
    QColor(95,215,215).rgb(),
    QColor(95,215,255).rgb(),
    QColor(95,255,0).rgb(),
    QColor(95,255,95).rgb(),
    QColor(95,255,135).rgb(),
    QColor(95,255,175).rgb(),
    QColor(95,255,215).rgb(),
    QColor(95,255,255).rgb(),
    QColor(135,0,0).rgb(),
    QColor(135,0,95).rgb(),
    QColor(135,0,135).rgb(),
    QColor(135,0,175).rgb(),
    QColor(135,0,215).rgb(),
    QColor(135,0,255).rgb(),
    QColor(135,95,0).rgb(),
    QColor(135,95,95).rgb(),
    QColor(135,95,135).rgb(),
    QColor(135,95,175).rgb(),
    QColor(135,95,215).rgb(),
    QColor(135,95,255).rgb(),
    QColor(135,135,0).rgb(),
    QColor(135,135,95).rgb(),
    QColor(135,135,135).rgb(),
    QColor(135,135,175).rgb(),
    QColor(135,135,215).rgb(),
    QColor(135,135,255).rgb(),
    QColor(135,175,0).rgb(),
    QColor(135,175,95).rgb(),
    QColor(135,175,135).rgb(),
    QColor(135,175,175).rgb(),
    QColor(135,175,215).rgb(),
    QColor(135,175,255).rgb(),
    QColor(135,215,0).rgb(),
    QColor(135,215,95).rgb(),
    QColor(135,215,135).rgb(),
    QColor(135,215,175).rgb(),
    QColor(135,215,215).rgb(),
    QColor(135,215,255).rgb(),
    QColor(135,255,0).rgb(),
    QColor(135,255,95).rgb(),
    QColor(135,255,135).rgb(),
    QColor(135,255,175).rgb(),
    QColor(135,255,215).rgb(),
    QColor(135,255,255).rgb(),
    QColor(175,0,0).rgb(),
    QColor(175,0,95).rgb(),
    QColor(175,0,135).rgb(),
    QColor(175,0,175).rgb(),
    QColor(175,0,215).rgb(),
    QColor(175,0,255).rgb(),
    QColor(175,95,0).rgb(),
    QColor(175,95,95).rgb(),
    QColor(175,95,135).rgb(),
    QColor(175,95,175).rgb(),
    QColor(175,95,215).rgb(),
    QColor(175,95,255).rgb(),
    QColor(175,135,0).rgb(),
    QColor(175,135,95).rgb(),
    QColor(175,135,135).rgb(),
    QColor(175,135,175).rgb(),
    QColor(175,135,215).rgb(),
    QColor(175,135,255).rgb(),
    QColor(175,175,0).rgb(),
    QColor(175,175,95).rgb(),
    QColor(175,175,135).rgb(),
    QColor(175,175,175).rgb(),
    QColor(175,175,215).rgb(),
    QColor(175,175,255).rgb(),
    QColor(175,215,0).rgb(),
    QColor(175,215,95).rgb(),
    QColor(175,215,135).rgb(),
    QColor(175,215,175).rgb(),
    QColor(175,215,215).rgb(),
    QColor(175,215,255).rgb(),
    QColor(175,255,0).rgb(),
    QColor(175,255,95).rgb(),
    QColor(175,255,135).rgb(),
    QColor(175,255,175).rgb(),
    QColor(175,255,215).rgb(),
    QColor(175,255,255).rgb(),
    QColor(215,0,0).rgb(),
    QColor(215,0,95).rgb(),
    QColor(215,0,135).rgb(),
    QColor(215,0,175).rgb(),
    QColor(215,0,215).rgb(),
    QColor(215,0,255).rgb(),
    QColor(215,95,0).rgb(),
    QColor(215,95,95).rgb(),
    QColor(215,95,135).rgb(),
    QColor(215,95,175).rgb(),
    QColor(215,95,215).rgb(),
    QColor(215,95,255).rgb(),
    QColor(215,135,0).rgb(),
    QColor(215,135,95).rgb(),
    QColor(215,135,135).rgb(),
    QColor(215,135,175).rgb(),
    QColor(215,135,215).rgb(),
    QColor(215,135,255).rgb(),
    QColor(215,175,0).rgb(),
    QColor(215,175,95).rgb(),
    QColor(215,175,135).rgb(),
    QColor(215,175,175).rgb(),
    QColor(215,175,215).rgb(),
    QColor(215,175,255).rgb(),
    QColor(215,215,0).rgb(),
    QColor(215,215,95).rgb(),
    QColor(215,215,135).rgb(),
    QColor(215,215,175).rgb(),
    QColor(215,215,215).rgb(),
    QColor(215,215,255).rgb(),
    QColor(215,255,0).rgb(),
    QColor(215,255,95).rgb(),
    QColor(215,255,135).rgb(),
    QColor(215,255,175).rgb(),
    QColor(215,255,215).rgb(),
    QColor(215,255,255).rgb(),
    QColor(255,0,0).rgb(),
    QColor(255,0,95).rgb(),
    QColor(255,0,135).rgb(),
    QColor(255,0,175).rgb(),
    QColor(255,0,215).rgb(),
    QColor(255,0,255).rgb(),
    QColor(255,95,0).rgb(),
    QColor(255,95,95).rgb(),
    QColor(255,95,135).rgb(),
    QColor(255,95,175).rgb(),
    QColor(255,95,215).rgb(),
    QColor(255,95,255).rgb(),
    QColor(255,135,0).rgb(),
    QColor(255,135,95).rgb(),
    QColor(255,135,135).rgb(),
    QColor(255,135,175).rgb(),
    QColor(255,135,215).rgb(),
    QColor(255,135,255).rgb(),
    QColor(255,175,0).rgb(),
    QColor(255,175,95).rgb(),
    QColor(255,175,135).rgb(),
    QColor(255,175,175).rgb(),
    QColor(255,175,215).rgb(),
    QColor(255,175,255).rgb(),
    QColor(255,215,0).rgb(),
    QColor(255,215,95).rgb(),
    QColor(255,215,135).rgb(),
    QColor(255,215,175).rgb(),
    QColor(255,215,215).rgb(),
    QColor(255,215,255).rgb(),
    QColor(255,255,0).rgb(),
    QColor(255,255,95).rgb(),
    QColor(255,255,135).rgb(),
    QColor(255,255,175).rgb(),
    QColor(255,255,215).rgb(),
    QColor(255,255,255).rgb(),
    QColor(8,8,8).rgb(),
    QColor(18,18,18).rgb(),
    QColor(28,28,28).rgb(),
    QColor(38,38,38).rgb(),
    QColor(48,48,48).rgb(),
    QColor(58,58,58).rgb(),
    QColor(68,68,68).rgb(),
    QColor(78,78,78).rgb(),
    QColor(88,88,88).rgb(),
    QColor(98,98,98).rgb(),
    QColor(108,108,108).rgb(),
    QColor(118,118,118).rgb(),
    QColor(128,128,128).rgb(),
    QColor(138,138,138).rgb(),
    QColor(148,148,148).rgb(),
    QColor(158,158,158).rgb(),
    QColor(168,168,168).rgb(),
    QColor(178,178,178).rgb(),
    QColor(188,188,188).rgb(),
    QColor(198,198,198).rgb(),
    QColor(208,208,208).rgb(),
    QColor(218,218,218).rgb(),
    QColor(228,228,228).rgb(),
    QColor(238,238,238).rgb()   // index 255
};


ColorPalette::ColorPalette(QObject *parent)
    : QObject(parent)
    , m_normalColors(numberOfColors)
    , m_lightColors(numberOfColors)
    , m_intenseColors(numberOfColors)
    , m_inverse_default(false)
{
    m_normalColors[0].setRgb(0,0,0);
    m_normalColors[1].setRgb(194,54,33);
    m_normalColors[2].setRgb(37,188,36);
    m_normalColors[3].setRgb(173,173,39);
    m_normalColors[4].setRgb(63,84,255);
    m_normalColors[5].setRgb(211,56,211);
    m_normalColors[6].setRgb(51,187,199);
    m_normalColors[7].setRgb(229,229,229);
    m_normalColors[8].setRgb(178,178,178);
    m_normalColors[9].setRgb(0,0,0);

    m_lightColors[0].setRgb(129,131,131);
    m_lightColors[1].setRgb(252,57,31);
    m_lightColors[2].setRgb(49,231,34);
    m_lightColors[3].setRgb(234,236,35);
    m_lightColors[4].setRgb(88,51,255);
    m_lightColors[5].setRgb(249,53,248);
    m_lightColors[6].setRgb(20,240,240);
    m_lightColors[7].setRgb(233,233,233);
    m_lightColors[8].setRgb(220,220,220);
    m_lightColors[9].setRgb(50,50,50);

}

QColor ColorPalette::color(ColorPalette::Color color, bool bold) const
{
    if (m_inverse_default) {
        if (color == DefaultForground)
            color = DefaultBackground;
        else if (color == DefaultBackground)
            color = DefaultForground;
    }
    if (bold)
        return m_lightColors.at(color);

    return m_normalColors.at(color);
}

QColor ColorPalette::normalColor(ColorPalette::Color color) const
{
    return this->color(color, false);
}

QColor ColorPalette::lightColor(ColorPalette::Color color) const
{
    return this->color(color,true);
}

QColor ColorPalette::xtermColor(int index)
{
    Q_ASSERT(index >= 16);
    return QColor(m_xtermColors[index - 16]);
}

QRgb ColorPalette::xtermRgb(int index)
{
    Q_ASSERT(index >= 16);
    return m_xtermColors[index - 16];
}

void ColorPalette::setInverseDefaultColors(bool inverse)
{
    bool emit_changed = inverse != m_inverse_default;
    if (emit_changed) {
        m_inverse_default = inverse;
        emit changed();
        emit defaultBackgroundColorChanged();
    }
}

QColor ColorPalette::defaultBackground() const
{
    return normalColor(DefaultBackground);
}
