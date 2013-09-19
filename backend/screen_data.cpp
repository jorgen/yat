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

#include "block.h"
#include "screen.h"

#include <stdio.h>

#include <QtGui/QGuiApplication>
#include <QtCore/QDebug>

ScreenData::ScreenData(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_scroll_start(0)
    , m_scroll_end(0)
    , m_scroll_area_set(false)
    , m_blocks_moved(0)
{
    connect(screen, SIGNAL(widthAboutToChange(int)), this,  SLOT(setWidth(int)));
    connect(screen, SIGNAL(heightAboutToChange(int, int)), this, SLOT(setHeight(int, int)));
}

ScreenData::~ScreenData()
{
    for (auto it = m_screen_blocks.cbegin(); it != m_screen_blocks.cend(); ++it) {
        delete *it;
    }
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

    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.cend(); ++it) {
        (*it)->setWidth(width);
    }
}

int ScreenData::height() const
{
    return m_screen_blocks.size();
}

void ScreenData::setHeight(int height, int currentCursorBlock)
{
    const int old_height = m_height;
    if (height == old_height)
        return;

    m_height = height;

    if (old_height > height) {
        const int to_remove = old_height - height;
        const int removeElementsBelowCursor =
            std::min(old_height - currentCursorBlock, to_remove);
        const int removeElementsAtTop = to_remove - removeElementsBelowCursor;
        if (removeElementsBelowCursor > 0) {
            auto begin = m_screen_blocks.begin();
            std::advance(begin, currentCursorBlock);
            auto it = begin;
            for (int i = 0; i < removeElementsBelowCursor; i++, ++it) {
                delete *it;
            }
            m_screen_blocks.erase(begin, it);
        }

        if (removeElementsAtTop > 0) {
            auto it = m_screen_blocks.begin();
            for (int i = 0; i < removeElementsAtTop; i++, ++it) {
                delete *it;
            }
            m_screen_blocks.erase(m_screen_blocks.begin(), it);
        }
    } else {
        int rowsToAdd = height - m_screen_blocks.size();
        for (int i = 0; i < rowsToAdd; i++) {
            Block *newBlock = new Block(m_screen);
            m_screen_blocks.push_back(newBlock);
            newBlock->setIndex(m_screen_blocks.size()-1);
        }
    }
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

Block *ScreenData::at(int index) const
{
    if (index > m_height / 2) {
        auto it = m_screen_blocks.end();
        std::advance(it, -(m_height - index));
        return *it;
    } else {
        auto it = m_screen_blocks.begin();
        std::advance(it, index);
        return *it;
    }
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    Block *block = at(row);
    block->clearCharacters(from_char, block->width() -1);
}

void ScreenData::clearToEndOfScreen(int row)
{
    auto it = m_screen_blocks.begin();
    std::advance(it, row);
    while (it != m_screen_blocks.end()) {
        (*it)->clear();
        ++it;
    }
}

void ScreenData::clearToBeginningOfLine(int row, int from_char)
{
    at(row)->clearCharacters(0,from_char);
}
void ScreenData::clearToBeginningOfScreen(int row)
{
    auto it = m_screen_blocks.begin();

    for (int i = 0; i <= row; i++, ++it) {
        (*it)->clear();
    }
}

void ScreenData::clearLine(int index)
{
    at(index)->clear();
}

void ScreenData::clear()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        (*it)->clear();
    }
}

void ScreenData::releaseTextObjects()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        (*it)->releaseTextObjects();
    }
}

void ScreenData::clearCharacters(int block, int from, int to)
{
    Block *block_item = at(block);
    block_item->clearCharacters(from,to);
}

void ScreenData::deleteCharacters(int block, int from, int to)
{
    Block *block_item = at(block);
    block_item->deleteCharacters(from,to);
}

void ScreenData::setScrollArea(int from, int to)
{
    m_scroll_area_set = true;
    m_scroll_start = from;
    m_scroll_end = to;
}

void ScreenData::moveLine(int from, int to)
{
    if (from == to)
        return;

    m_blocks_moved++;

    bool from_found = false;
    std::list<Block *>::iterator from_it = m_screen_blocks.end();
    bool to_found = false;
    std::list<Block *>::iterator to_it = m_screen_blocks.end();

    int i = 0;
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); i++, ++it) {
        if (i == from) {
            from_it = it;
            from_found = true;
        }
        if (i == to) {
            to_it = it;
            if (to > from)
                to_it++;
            to_found = true;
        }
        if (from_found && to_found)
            break;
    }
    if (!from_found || !to_found) {
        qDebug() << "Failed to find line in" << Q_FUNC_INFO;
        return;
    }

    (*from_it)->clear();
    m_screen_blocks.splice(to_it, m_screen_blocks, from_it);
    //m_screen_blocks.insert(to_it, *from_it);
    //m_screen_blocks.erase(from_it);
    if (!m_screen->fastScroll() && m_blocks_moved > 6)
        m_screen->dispatchChanges();
}

void ScreenData::fill(const QChar &character)
{
    const QString fill_str(width(), character);
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        (*it)->replaceAtPos(0, fill_str, m_screen->defaultTextStyle());
    }
}

void ScreenData::updateIndexes(int from, int to)
{
    if (to < 0) {
        to = m_screen_blocks.size() -1;
    }
    for (int i = from; i <= to; i++) {
    }
}

void ScreenData::sendSelectionToClipboard(const QPointF &start, const QPointF &end, QClipboard::Mode clipboard)
{
    QString data;
    int start_block = qMax((int)start.y(), 0);
    int end_block = qMin((int)end.y(), m_height - 1);
    auto it = m_screen_blocks.begin();
    std::advance(it, start_block);
    for (int i = start_block; i <= end_block; i++, ++it) {
        int char_start = 0;
        int char_end = m_width - 1;
        if (i == start_block)
            char_start = start.x();
        else
            data.append(QChar('\n'));
        if (i == end_block)
            char_end = end.x();
        data += (*it)->textLine()->mid(char_start, char_end - char_start).trimmed();
    }

    QGuiApplication::clipboard()->setText(data, clipboard);
}

void ScreenData::getDoubleClickSelectionArea(const QPointF &cliked, int *start_ret, int *end_ret) const
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
    static const int size_of_delimiter_list = sizeof delimiter_list / sizeof *delimiter_list;

    *start_ret = -1;
    *end_ret = -1;
    bool find_equals = false;

    QStringRef to_return(at(cliked.y())->textLine());

    QChar clicked_char = to_return.at(cliked.x());

    for (int i=0; i<size_of_delimiter_list; i++) {
        if (clicked_char == delimiter_list[i])
            find_equals = true;
    }

    for (int i = cliked.x() - 1; i >= 0; i--) {
        if (find_equals) {
            if (clicked_char != to_return.at(i)) {
                *start_ret = i + 1;
                break;
            }
        } else {
            for (int delimiter_i = 0; delimiter_i < size_of_delimiter_list; delimiter_i++) {
                if (to_return.at(i) == delimiter_list[delimiter_i]) {
                    *start_ret = i + 1;
                    i = -1;
                    break;
                }
            }
        }
    }
    if (*start_ret < 0)
        *start_ret = 0;

    for (int i = cliked.x() + 1; i < to_return.size(); i++) {
        if (find_equals) {
            if (clicked_char != to_return.at(i)) {
                *end_ret = i;
                break;
            }
        } else {
            for (int delimiter_i = 0; delimiter_i < size_of_delimiter_list; delimiter_i++) {
                if (to_return.at(i) == delimiter_list[delimiter_i]) {
                    *end_ret = i;
                    i = to_return.size();
                    break;
                }
            }
        }
    }
    if (*end_ret < 0)
        *end_ret = to_return.size();

}

void ScreenData::dispatchLineEvents()
{
    auto it = m_screen_blocks.begin();
    for (int i = 0; i < m_height; i++, ++it) {
        (*it)->setIndex(i);
        (*it)->dispatchEvents();
    }
    m_blocks_moved = 0;
}

void ScreenData::printStyleInformation() const
{

    auto it = m_screen_blocks.begin();
    for (int index = 0; index < m_height; index++, ++it) {
        if (index % 5 == 0) {
            QString ruler = QString("|----i----").repeated(m_width/10).append("|");
            qDebug() << "Ruler:" << index << "      " << (void *) this << ruler;
        }
        QDebug debug = qDebug();
        debug << "Line: " << index;
        (*it)->printStyleList(debug);
    }
}

Screen *ScreenData::screen() const
{
    return m_screen;
}
