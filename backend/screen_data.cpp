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

#include "screen_data.h"

#include "screen.h"

#include <stdio.h>

#include <QtGui/QGuiApplication>
#include <QtCore/QDebug>

ScreenData::ScreenData(Screen *screen, QTextDocument *document)
    : m_screen(screen)
    , m_document(document)
    , m_cursor(m_document)
    , m_scroll_start(0)
    , m_scroll_end(0)
    , m_scroll_area_set(false)
{
    document->setMaximumBlockCount(200);
}

ScreenData::~ScreenData()
{
}


int ScreenData::width() const
{
    return m_width;
}

void ScreenData::setWidth(int width)
{
    if (width == m_width)
        return;

    m_width = width;

}

int ScreenData::height() const
{
    return 0;
}

void ScreenData::setHeight(int height)
{
    if (!m_scroll_area_set)
        m_scroll_end = height - 1;
}

int ScreenData::scrollAreaStart() const
{
    return m_scroll_start;
}

int ScreenData::scrollAreaEnd() const
{
    return m_scroll_end;
}

void ScreenData::insertAtCursor(const QString &text)
{
    m_cursor.insertText(text);
}

void ScreenData::insertLineFeed()
{
    m_cursor.insertBlock();
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    Q_UNUSED(row);
    Q_UNUSED(from_char);
    Q_ASSERT(false);
}

void ScreenData::clearToEndOfScreen(int row)
{
    Q_UNUSED(row);
    Q_ASSERT(false);
}

void ScreenData::clearToBeginningOfScreen(int row)
{
    Q_ASSERT(false);
    Q_UNUSED(row);
}

void ScreenData::clearLine(int index)
{
    Q_UNUSED(index);
    Q_ASSERT(false);
}

void ScreenData::clear()
{
}

void ScreenData::setScrollArea(int from, int to)
{
    m_scroll_area_set = true;
    m_scroll_start = from;
    m_scroll_end = to;
}

void ScreenData::moveLine(int from, int to)
{
    Q_ASSERT(false);
    Q_UNUSED(from);
    Q_UNUSED(to);
}

void ScreenData::updateIndexes(int from, int to)
{
    Q_ASSERT(false);
    Q_UNUSED(from);
    Q_UNUSED(to);
}

void ScreenData::sendSelectionToClipboard(const QPointF &start, const QPointF &end, QClipboard::Mode clipboard)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(clipboard);

    //QGuiApplication::clipboard()->setText(data, clipboard);
}

void ScreenData::getDoubleClickSelectionArea(const QPointF &clicked, int *start_ret, int *end_ret) const
{
    static const QChar delimiter_list[] = {
        ' ',
        '<',
        '>',
        ')',
        '(',
        '{',
        '}',
        '[',
        ']'
    };
    Q_UNUSED(delimiter_list);
    Q_UNUSED(clicked);
    Q_UNUSED(start_ret);
    Q_UNUSED(end_ret);

    Q_ASSERT(false);
}

void ScreenData::printScreen() const
{
    Q_ASSERT(false);
}
