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

#include "terminal_screen.h"
#include <QtCore/QDebug>

TextSegment::TextSegment(QString *text_line, TerminalScreen *terminalScreen)
    : QObject(terminalScreen)
    , m_text_line(text_line)
    , m_start_index(0)
    , m_old_start_index(0)
    , m_end_index(0)
    , m_style(terminalScreen->defaultTextStyle())
    , m_dirty(true)
    , m_screen(terminalScreen)
{
    connect(terminalScreen, &TerminalScreen::dispatchTextSegmentChanges,
            this, &TextSegment::dispatchEvents);
}

TextSegment::~TextSegment()
{
}

int TextSegment::index() const
{
    return m_start_index;
}

QString TextSegment::text() const
{
    return m_text;
}

QColor TextSegment::forgroundColor() const
{
    return m_style.forground;
}

QColor TextSegment::backgroundColor() const
{
    return m_style.background;
}

void TextSegment::setStringSegment(int start_index, int end_index)
{
    m_start_index = start_index;
    m_end_index = end_index;

    m_dirty = true;
}

void TextSegment::setTextStyle(const TextStyle &style)
{
    m_style = style;
    m_dirty = true;
}

TerminalScreen *TextSegment::screen() const
{
    return m_screen;
}

void TextSegment::dispatchEvents()
{
    if (m_dirty) {
        m_dirty = false;
        m_text = m_text_line->mid(m_start_index, m_end_index + 1 - m_start_index);
        if (m_old_start_index != m_start_index) {
            m_old_start_index = m_start_index;
            emit indexChanged();
        }
        emit textChanged();
    }
}

