#include "text_style.h"

#include <QtCore/QDebug>

bool TextStyle::isCompatible(const TextStyle &other) const
{
    return foreground == other.foreground
            && background == other.background
            && style == other.style;
}
