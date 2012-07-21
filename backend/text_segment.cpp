/**************************************************************************************************
* Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute,
* sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
* OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
***************************************************************************************************/

#include "text_segment.h"

#include <QtCore/QDebug>

TextSegment::TextSegment(const QString &text, const QColor &forground, const QColor &background, QObject *parent)
    : QObject(parent)
    , m_text(text)
    , m_forground_color(forground)
    , m_background_color(background)
{
}

TextSegment::TextSegment(QObject *parent)
    : QObject(parent)
{
}

QString TextSegment::text() const
{
    return m_text;
}

QColor TextSegment::forgroundColor() const
{
    return m_forground_color;
}

void TextSegment::setForgroundColor(const QColor &color)
{
    m_forground_color = color;
}

QColor TextSegment::backgroundColor() const
{
    return m_background_color;
}

void TextSegment::setBackgroundColor(const QColor &color)
{
    m_background_color = color;
}

TextSegment *TextSegment::split(int i)
{
    QString str = m_text.right(i);
    TextSegment *right = new TextSegment(str,m_forground_color,m_background_color,parent());
    m_text.truncate(i);
    return right;
}

TextSegmentLine::TextSegmentLine(QObject *parent)
    : QObject(parent)
{
}

TextSegmentLine::~TextSegmentLine()
{
}

void TextSegmentLine::reset()
{
    for (int i = 0; i < m_segments.size(); i++) {
        delete m_segments.at(i);
    }
    m_segments.clear();
}

int TextSegmentLine::size() const
{
    return m_segments.size();
}

TextSegment *TextSegmentLine::at(int i) const
{
    return m_segments.at(i);
}

QList<TextSegment *> TextSegmentLine::segments() const
{
    return m_segments;
}

void TextSegmentLine::append(TextSegment *segment)
{
    segment->setParent(this);
    m_segments.append(segment);
    emit sizeChanged();
}

void TextSegmentLine::prepend(TextSegment *segment)
{
    segment->setParent(this);
    m_segments.prepend(segment);
    emit sizeChanged();
}

void TextSegmentLine::insertAtPos(int pos, TextSegment *segment)
{
    segment->setParent(this);
    if (m_segments.size() == 0 || pos == 0) {
        m_segments.prepend(segment);
        emit sizeChanged();
        return;
    }

    int char_count = 0;
    for (int i = 0; i < m_segments.size(); i++) {
        TextSegment *left = m_segments.at(i);
        if (pos < char_count + left->text().size()) {
            int split_position = pos - char_count;
            TextSegment *right = left->split(split_position);
            m_segments.insert(i+1,segment);
            m_segments.insert(i+1,right);
            emit sizeChanged();
            return;
        }
        char_count += m_segments.at(i)->text().size();
    }
    m_segments << segment;
    sizeChanged();
}
