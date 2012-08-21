#include "text_style.h"

#include <QtCore/QDebug>
TextStyle::TextStyle(TextStyle::Style style, const QColor forground)
    : style(style)
    , forground(forground)
    , background(Qt::transparent)
{
}

bool TextStyle::isCompatible(const TextStyle &other) const
{
    return forground == other.forground
            && background == other.background
            && style == other.style;
}
