#include "text_style.h"

TextStyle::TextStyle(TextStyle::Style style, const QColor forground)
    : style(style)
    , forground(forground)
    , background(Qt::transparent)
{
}
