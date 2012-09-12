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

#include "text.h"

#include "screen.h"
#include "line.h"
#include <QtQuick/QQuickItem>

#include <QtCore/QDebug>

Text::Text(QString *text_line, Line *line)
    : QObject(line)
    , m_text_line(text_line)
    , m_start_index(0)
    , m_old_start_index(0)
    , m_end_index(0)
    , m_style(line->screen()->defaultTextStyle())
    , m_style_dirty(true)
    , m_text_dirty(true)
    , m_line(line)
    , m_item(m_line->screen()->createTextItem())
{
    connect(screen(), &Screen::dispatchTextSegmentChanges,
            this, &Text::dispatchEvents);
    m_item->setProperty("textSegment",QVariant::fromValue(this));
    m_item->setProperty("font", screen()->font());
    m_item->setProperty("parent", QVariant::fromValue(m_line->quickItem()));
}

Text::~Text()
{
    m_item->setProperty("textSegment", QVariant::fromValue(static_cast<Text *>(0)));
    m_item->setProperty("visible", false);
    m_line->screen()->destroyTextItem(m_item);
}

int Text::index() const
{
    return m_start_index;
}

QString Text::text() const
{
    return m_text;
}

QColor Text::foregroundColor() const
{
    if (m_style.style & TextStyle::Inverse) {
        if (m_style.background == ColorPalette::DefaultBackground)
            return screen()->screenBackground();
        return screen()->colorPalette()->color(m_style.background, m_style.style & TextStyle::Bold);
    }

    return screen()->colorPalette()->color(m_style.foreground, m_style.style & TextStyle::Bold);
}


QColor Text::backgroundColor() const
{
    if (m_style.style & TextStyle::Inverse)
        return screen()->colorPalette()->color(m_style.foreground, false);

    return screen()->colorPalette()->color(m_style.background, false);
}

void Text::setStringSegment(int start_index, int end_index)
{
    m_start_index = start_index;
    m_end_index = end_index;

    m_text_dirty = true;
}

void Text::setTextStyle(const TextStyle &style)
{
    m_style = style;
    m_style_dirty = true;
}

Screen *Text::screen() const
{
    return m_line->screen();
}

QObject *Text::item() const
{
    return m_item;
}

void Text::dispatchEvents()
{
    if (m_style_dirty) {
        m_style_dirty = false;
        emit styleChanged();
    }

    if (m_text_dirty) {
        m_text = m_text_line->mid(m_start_index, m_end_index + 1 - m_start_index);
        if (m_old_start_index != m_start_index) {
            m_old_start_index = m_start_index;
            emit indexChanged();
        }
        emit textChanged();
    }
}

