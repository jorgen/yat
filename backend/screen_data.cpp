/*******************************************************************************
 * Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*******************************************************************************/

#include "screen_data.h"

#include "block.h"
#include "screen.h"
#include "scrollback.h"

#include <stdio.h>

#include <QtGui/QGuiApplication>
#include <QtCore/QDebug>

ScreenData::ScreenData(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_scrollback(new Scrollback(this))
    , m_height(0)
    , m_total_lines(0)
{
    connect(screen, SIGNAL(heightAboutToChange(int, int, int)), this, SLOT(setHeight(int, int, int)));
}

ScreenData::~ScreenData()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        delete *it;
    }
    delete m_scrollback;
}


int ScreenData::contentHeight() const
{
    return m_screen->height() + m_scrollback->height();
}

void ScreenData::setHeight(int height, int currentCursorBlock, int currentContentHeight)
{
    Q_UNUSED(currentContentHeight);
    const int old_height = m_height;
    if (height == old_height)
        return;

    if (old_height > height) {
        const int to_remove = old_height - height;
        const int remove_from_end = std::min((old_height - 1) - currentCursorBlock, to_remove);
        const int remove_from_begin = to_remove - remove_from_end;
        if (remove_from_end > 0) {
            auto it = --m_screen_blocks.end();
            for (int i = 0; i < remove_from_end; --it, i++) {
                delete *it;
            }
            m_screen_blocks.erase(++it, m_screen_blocks.end());
        }
        if (remove_from_begin > 0) {
            auto it = m_screen_blocks.begin();
            for (int i = 0; i < remove_from_begin; ++it, i++) {
                m_scrollback->addBlock(*it);
            }
            m_screen_blocks.erase(m_screen_blocks.begin(), it);
        }
        m_height -= to_remove;
    } else {
        int rowsToAdd = height - old_height;
        m_height += rowsToAdd;
        int reclaimed = 0;
        for (int i = 0; i < rowsToAdd; i++) {
            Block *newBlock = m_scrollback->reclaimBlock();
            if (newBlock) {
                reclaimed ++;
                m_screen_blocks.push_front(newBlock);
            } else {
                m_screen_blocks.push_back(new Block(m_screen));
            }
        }
    }
}

Block *ScreenData::blockContainingLine(int line) const
{
    if (line == m_height - 1)
        return m_screen_blocks.back();
    auto it = m_screen_blocks.end();
    int move_back = line - m_height;
    std::advance(it, move_back);
    return *it;
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    Block *block = blockContainingLine(row);
    block->clearToEnd(from_char);
}

void ScreenData::clearToEndOfScreen(int row)
{
    auto it = --m_screen_blocks.end();
    int lines_back = m_height - row;
    for(int i = 0; i < lines_back; --it, i++) {
        (*it)->clear();
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
    auto it = --m_screen_blocks.end();
    for (int i = 0; i < m_height; --it, i++) {
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

    auto from_it = m_screen_blocks.end();
    std::advance(from_it, from - m_height);
    (*from_it)->clear();

    auto to_it = m_screen_blocks.end();
    if (to < from)
        --to_it;
    std::advance(to_it, to - m_height);

    m_screen_blocks.splice(++to_it, m_screen_blocks, from_it);
}

void ScreenData::insertLine(int row)
{
    auto row_it = m_screen_blocks.end();
    std::advance(row_it, row - (m_height - 1));

    Block *block_to_insert = new Block(m_screen);
    m_screen_blocks.insert(row_it,block_to_insert);

    m_scrollback->addBlock(*m_screen_blocks.begin());
    m_screen_blocks.pop_front();
}


void ScreenData::fill(const QChar &character)
{
    auto it = --m_screen_blocks.end();
    for (int i = 0; i < m_height; --it, i++) {
        QString fill_str(m_screen->width(), character);
        (*it)->replaceAtPos(0, fill_str, m_screen->defaultTextStyle());
    }
}

void ScreenData::sendSelectionToClipboard(const QPointF &start, const QPointF &end, QClipboard::Mode clipboard)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(clipboard);
    //QString data;
    //int start_block = qMax((int)start.y(), 0);
    //int end_block = qMin((int)end.y(), m_screen_blocks.size()-1);
    //for (int i = start_block; i <= end_block; i++) {
    //    int char_start = 0;
    //    int char_end = m_width - 1;
    //    if (i == start_block)
    //        char_start = start.x();
    //    else
    //        data.append(QChar('\n'));
    //    if (i == end_block)
    //        char_end = end.x();
    //    data += blockContainingLine(i)->textLine()->mid(char_start, char_end - char_start).trimmed();
    //}

    //QGuiApplication::clipboard()->setText(data, clipboard);
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
    if (!m_height)
        return;
    auto it = --m_screen_blocks.end();
    const int content_height = contentHeight();
    for (int i = 0; i < m_height; --it, i++) {
        int line = (content_height - 1) - i;
        (*it)->setIndex(line);
        (*it)->dispatchEvents();
        if (it == m_screen_blocks.begin())
            break;
    }

    if (content_height != m_total_lines) {
        m_total_lines = contentHeight();
        emit contentHeightChanged();
    }
}

void ScreenData::printStyleInformation() const
{
    auto it = m_screen_blocks.end();
    std::advance(it, -m_height);
    for (int i = 0; it != m_screen_blocks.end(); ++it, i++) {
        QDebug debug = qDebug();
        if (i % 5 == 0) {
            debug << "Ruler:" << i;
            (*it)->printRuler(debug);
        }
        debug << "Line: " << i;
        (*it)->printStyleList(debug);
    }
}

Screen *ScreenData::screen() const
{
    return m_screen;
}

void ScreenData::ensureVisiblePages(int top_line)
{
    m_scrollback->ensureVisiblePages(top_line);
}

Scrollback *ScreenData::scrollback() const
{
    return m_scrollback;
}

std::list<Block *>::iterator ScreenData::it_for_row(int row)
{
    int advance = row - m_height;
    auto it = m_screen_blocks.end();
    std::advance(it, advance);
    return it;
}
