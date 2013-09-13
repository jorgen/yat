#include "text_style.h"

#include <QtCore/QDebug>

TextStyle::TextStyle()
    : style(Normal)
    , forground(ColorPalette::DefaultForground)
    , background(ColorPalette::DefaultBackground)
{

}

bool TextStyle::isCompatible(const TextStyle &other) const
{
    return forground == other.forground
            && background == other.background
            && style == other.style;
}

QDebug operator<<(QDebug debug, TextStyleLine line)
{
    debug << "[" << line.start_index << "(" << line.style << ")" << line.end_index << "]"; 
    return debug;
}

