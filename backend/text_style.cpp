#include "text_style.h"

#include "screen.h"
#include "text.h"

#include <QtCore/QDebug>

TextStyle::TextStyle()
    : style(Normal)
    , foreground(0)
    , background(0)
{
}

bool TextStyle::isCompatible(const TextStyle &other) const
{
    return foreground == other.foreground
            && background == other.background
            && style == other.style;
}

QDebug operator<<(QDebug debug, TextStyleLine line)
{
    debug << "[" << line.start_index << "(" << line.style << ":" << line.foreground << ":" << line.background << ")" << line.end_index << "]";
    return debug;
}

void TextStyleLine::releaseTextSegment(Screen *screen)
{
    if (text_segment) {
        text_segment->setVisible(false);
        screen->releaseTextSegment(text_segment);
        text_segment = 0;
    }
}
