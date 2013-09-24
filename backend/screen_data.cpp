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
    , m_blocks_moved(0)
{
    connect(screen, SIGNAL(widthAboutToChange(int)), this,  SLOT(setWidth(int)));
    connect(screen, SIGNAL(heightAboutToChange(int, int)), this, SLOT(setHeight(int, int)));
}

ScreenData::~ScreenData()
{
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        delete blockContainingLine(i);
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

//    for (int i = 0; i < m_screen_blocks.size(); i++) {
//        blockContainingLine(i)->setWidth(width);
//    }
}

int ScreenData::height() const
{
    return m_screen_blocks.size();
}

void ScreenData::setHeight(int height, int currentCursorBlock)
{
    const int old_height = m_screen_blocks.size();
    if (height == old_height)
        return;

    if (old_height > height) {
        const int to_remove = old_height - height;
        const int removeElementsBelowCursor =
            std::min(old_height - currentCursorBlock, to_remove);
        const int removeElementsAtTop = to_remove - removeElementsBelowCursor;
        if (removeElementsBelowCursor > 0) {
            for (int i = currentCursorBlock; i < currentCursorBlock + removeElementsBelowCursor; i++) {
                delete m_screen_blocks[i];
            }
            m_screen_blocks.remove(currentCursorBlock, removeElementsBelowCursor);
        }

        if (removeElementsAtTop > 0) {
            for (int i = 0; i < removeElementsAtTop; i++) {
                delete m_screen_blocks[i];
            }
            m_screen_blocks.remove(0, removeElementsAtTop);
        }
    } else {
        int rowsToAdd = height - m_screen_blocks.size();
        for (int i = 0; i < rowsToAdd; i++) {
            Block *newBlock = new Block(m_screen);
            m_screen_blocks.append(newBlock);
        }
    }
}

Block *ScreenData::blockContainingLine(int line) const
{
    return m_screen_blocks.at(line);
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    Block *block = blockContainingLine(row);
    block->clearCharacters(from_char, m_width -1);
}

void ScreenData::clearToEndOfScreen(int row)
{
    for(int i = row; i < m_screen_blocks.size(); i++) {
        Block *block = blockContainingLine(i);
        block->clear();
    }
}

void ScreenData::clearToBeginningOfLine(int row, int from_char)
{
    blockContainingLine(row)->clearCharacters(0,from_char);
}
void ScreenData::clearToBeginningOfScreen(int row)
{
    for (int i = row; i >= 0; i--) {
        Block *block = blockContainingLine(i);
        block->clear();
    }
}

void ScreenData::clearLine(int index)
{
    blockContainingLine(index)->clear();
}

void ScreenData::clear()
{
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        Block *block = blockContainingLine(i);
        block->clear();
    }
}

void ScreenData::releaseTextObjects()
{
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        Block *block = blockContainingLine(i);
        block->releaseTextObjects();
    }
}

void ScreenData::clearCharacters(int block, int from, int to)
{
    Block *block_item = blockContainingLine(block);
    block_item->clearCharacters(from,to);
}

void ScreenData::deleteCharacters(int block, int from, int to)
{
    Block *block_item = blockContainingLine(block);
    block_item->deleteCharacters(from,to);
}

void ScreenData::moveLine(int from, int to)
{
    if (from == to)
        return;

    if (from < to) {
        int blocks_to_shift = to - from;
        m_blocks_moved += blocks_to_shift;
        Block *from_block = blockContainingLine(from);
        Block **from_block_ptr = m_screen_blocks.data() + from;
        memmove(from_block_ptr, from_block_ptr+1, sizeof(from_block_ptr) * blocks_to_shift);
        from_block->clear();
        m_screen_blocks.replace(to,from_block);
    } else {
        int blocks_to_shift = from - to;
        m_blocks_moved += blocks_to_shift;
        Block *from_block = blockContainingLine(from);
        Block **to_block_ptr = const_cast<Block **>(m_screen_blocks.constData() + to);
        memmove(to_block_ptr + 1, to_block_ptr, sizeof(to_block_ptr) * blocks_to_shift);
        from_block->clear();
        m_screen_blocks.replace(to,from_block);
    }
    if (!m_screen->fastScroll() && m_blocks_moved > 6)
        m_screen->dispatchChanges();
}

void ScreenData::fill(const QChar &character)
{
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        Block *block = m_screen_blocks[i];
        QString fill_str(width(), character);
        block->replaceAtPos(0, fill_str, m_screen->defaultTextStyle());
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
    int end_block = qMin((int)end.y(), m_screen_blocks.size()-1);
    for (int i = start_block; i <= end_block; i++) {
        int char_start = 0;
        int char_end = m_width - 1;
        if (i == start_block)
            char_start = start.x();
        else
            data.append(QChar('\n'));
        if (i == end_block)
            char_end = end.x();
        data += blockContainingLine(i)->textLine()->mid(char_start, char_end - char_start).trimmed();
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

    QStringRef to_return(blockContainingLine(cliked.y())->textLine());

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
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        blockContainingLine(i)->setIndex(i);
        blockContainingLine(i)->dispatchEvents();
    }
    m_blocks_moved = 0;
}

void ScreenData::printStyleInformation() const
{
    for (int i = 0; i < m_screen_blocks.size(); i++) {
        const Block *block = blockContainingLine(i);
        QDebug debug = qDebug();
        if (i % 5 == 0) {
            debug << "Ruler:" << i;
            block->printRuler(debug);
        }
        debug << "Line: " << i;
        block->printStyleList(debug);
    }
}

Screen *ScreenData::screen() const
{
    return m_screen;
}
