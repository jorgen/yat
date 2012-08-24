#include "text_style.h"

#include <QtCore/QDebug>
TextStyle::TextStyle(TextStyle::Styles style, ColorPalette::Color forground)
    : style(style)
    , foreground(forground)
    , background(ColorPalette::DefaultBackground)
{
}

bool TextStyle::isCompatible(const TextStyle &other) const
{
    return foreground == other.foreground
            && background == other.background
            && style == other.style;
}
