#include "text_segment.h"

TextSegment::TextSegment(const QString &text, const QColor &forground, const QColor &background)
    : m_text(text)
    , m_forground_color(forground)
    , m_background_color(background)
{
}

TextSegment::TextSegment()
{
}

