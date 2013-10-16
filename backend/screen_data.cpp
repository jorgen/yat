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
#include "cursor.h"

#include <stdio.h>

#include <set>

#include <QtGui/QGuiApplication>
#include <QtCore/QDebug>

ScreenData::ScreenData(size_t max_scrollback, Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_total_lines(0)
    , m_total_lines_old(0)
    , m_viewport_height(0)
    , m_width(0)
{
    Q_UNUSED(max_scrollback);
    connect(screen, SIGNAL(heightAboutToChange(int, int, int)), this, SLOT(setHeight(int, int, int)));
    connect(screen, SIGNAL(widthAboutToChange(int)), this, SLOT(setWidth(int)));
}

ScreenData::~ScreenData()
{
    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
        delete *it;
    }
}


int ScreenData::contentHeight() const
{
    return m_total_lines;
}

void ScreenData::setHeight(int height, int currentCursorBlock, int currentContentHeight)
{
    Q_UNUSED(currentContentHeight);
    Q_UNUSED(currentCursorBlock);
    if (m_viewport_height > height) {
        const int to_remove = m_viewport_height - height;
        const int remove_from_end = std::min((m_viewport_height - 1) - currentCursorBlock, to_remove);
        if (remove_from_end > 0) {
            auto it = --m_blocks.end();
            for (int i = 0; i < remove_from_end; --it, i++) {
                m_total_lines -= (*it)->lines();
                delete *it;
            }
            m_blocks.erase(++it, m_blocks.end());
        }
    } else if (m_total_lines < height) {
        int rowsToAdd = height - m_total_lines;
        for (int i = 0; i < rowsToAdd; i++) {
            auto block = new Block(m_screen);
            m_total_lines++;
            m_blocks.push_back(block);
        }
    }

    m_viewport_height = height;
    m_top_of_viewport = m_blocks.end();
    std::advance(m_top_of_viewport, -height);
    m_pages.clear();
}

void ScreenData::setWidth(int width)
{
    m_total_lines = 0;
    m_width = width;
    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
        (*it)->setWidth(width);
        m_total_lines += (*it)->lines();
    }
}

Block *ScreenData::blockContainingLine(int line)
{
    auto it = it_for_row(line);
    if (it == m_blocks.end())
        return 0;
    return *it;
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    Block *block = blockContainingLine(row);
    block->clearToEnd(from_char);
}

void ScreenData::clearToEndOfScreen(int row)
{
    Q_UNUSED(row);
    Q_ASSERT(false);
    //auto it = --m_blocks.end();
    //int lines_back = m_total_lines - row;
    //for(int i = 0; i < lines_back; --it, i++) {
    //    (*it)->clear();
    //}
}

void ScreenData::clearToBeginningOfLine(int row, int from_char)
{
    blockContainingLine(row)->clearCharacters(0,from_char);
}
void ScreenData::clearToBeginningOfScreen(int row)
{
    Q_UNUSED(row);
    Q_ASSERT(false); //implement
//    for (int i = row; i >= 0; i--) {
//        Block *block = blockContainingLine(i);
//        block->clear();
//    }
}

void ScreenData::clearLine(int index)
{
    blockContainingLine(index)->clear();
}

void ScreenData::clear()
{
    auto it = --m_blocks.end();
    for (int i = 0; i < m_viewport_height; --it, i++) {
        (*it)->clear();
        if (it == m_blocks.begin())
            break;
    }
}

void ScreenData::releaseTextObjects()
{   //too heavy
    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
        (*it)->releaseTextObjects();
    }
}

void ScreenData::clearCharacters(int line, int from, int to)
{
    Block *block_item = blockContainingLine(line);
    block_item->clearCharacters(from,to);
}

void ScreenData::deleteCharacters(int line, int from, int to)
{
    Block *block_item = blockContainingLine(line);
    block_item->deleteCharacters(from,to);
}

CursorDiff ScreenData::replace(int line, int from_char, const QString &text, const TextStyle &style)
{
    auto it = it_for_row(line);
    Block *block = *it;
    size_t lines_before = block->lines();
    int start_char = (line - block->line()) * m_width + from_char;
    block->replaceAtPos(start_char, text, style);
    int lines_changed = block->lines() - lines_before;
    int end_char = (start_char + text.size()) % m_width;
    int removed = 0;
    for(auto to_remove = --m_blocks.end();
            lines_changed && removed <= lines_changed && to_remove != it; --to_remove) {
        if (removed + (*to_remove)->lines() <= lines_changed) {
            removed+= (*to_remove)->lines();
            delete *to_remove;
            to_remove = m_blocks.erase(to_remove);
        } else {
            break;
            --to_remove;
            removed+=1;
        }
    }
    m_total_lines += lines_changed;
    return { lines_changed, end_char - start_char };
}

CursorDiff ScreenData::insert(int line, int from_char, const QString &text, const TextStyle &style)
{
    auto it = it_for_row(line);
    Block *block = *it;
    size_t lines_before = block->lines();
    int start_char = (line - block->line()) * m_width + from_char;
    block->insertAtPos(start_char, text, style);
    int lines_changed = block->lines() - lines_before;
    int end_char = (start_char + text.size()) % m_width;
    int removed = 0;
    for(auto to_remove = --m_blocks.end();
            removed <= lines_changed && to_remove != it; --to_remove, removed++) {
        if (removed + (*to_remove)->lines() <= lines_changed) {
            to_remove = m_blocks.erase(to_remove);
        } else {
            qDebug() << "NOT HANDLED";
            --to_remove;
        }
    }
    m_total_lines += lines_changed;
    return { lines_changed, end_char - start_char };
}

void ScreenData::moveLine(int from, int to)
{
    if (from == to)
        return;

    auto from_it = m_blocks.end();
    std::advance(from_it, from - m_total_lines);
    (*from_it)->clear();

    auto to_it = m_blocks.end();
    std::advance(to_it, to - (m_total_lines-1));

    m_blocks.splice(to_it, m_blocks, from_it);
}

void ScreenData::insertLine(int row)
{
    auto insert_it = it_for_row(row);
    auto it = insert_it;

    Block *block_to_insert = new Block(m_screen);
    m_blocks.insert(insert_it, block_to_insert);

    m_total_lines++;

    for (; it != m_blocks.end(); ++it) {
        (*it)->incLine();
    }

    (*m_top_of_viewport)->releaseTextObjects();
    ++m_top_of_viewport;
}


void ScreenData::fill(int start_line, int size, const QChar &character)
{
    int advance = m_total_lines - (start_line + size);
    auto it = m_blocks.end();
    std::advance(it, advance);
    for (int i = 0; i < size; --it, i++) {
        QString fill_str(m_screen->width(), character);
        (*it)->clear();
        (*it)->replaceAtPos(0, fill_str, m_screen->defaultTextStyle());
        if (it == m_blocks.begin()) {
            qDebug() << "SIZE is bigger than n lines";
            break;
        }
    }
}

void ScreenData::sendSelectionToClipboard(const QPointF &start, const QPointF &end, QClipboard::Mode clipboard)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(clipboard);
    //QString data;
    //int start_block = qMax((int)start.y(), 0);
    //int end_block = qMin((int)end.y(), m_blocks.size()-1);
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

void ScreenData::getDoubleClickSelectionArea(const QPointF &cliked, int *start_ret, int *end_ret)
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
    if (m_total_lines_old != m_total_lines) {
        m_total_lines_old = m_total_lines;
        emit contentHeightChanged();
    }

    if (!m_viewport_height)
        return;

    auto it = --m_blocks.end();
    int block_line = m_total_lines;
    for (int i = 0; i < m_viewport_height; --it) {
        Block *block = *it;
        block_line -= block->lines();
        i += block->lines();
        block->setLine(block_line);
        block->dispatchEvents();
        if (it == m_blocks.begin())
            break;
    }
}

void ScreenData::printStyleInformation() const
{
    auto it = m_blocks.end();
    std::advance(it, -m_viewport_height);
    for (int i = 0; it != m_blocks.end(); ++it, i++) {
        if (i % 5 == 0) {
            QDebug debug = qDebug();
            debug << "Ruler:" << i;
            (*it)->printRuler(debug);
            debug << "\n";
        }
        QDebug debug = qDebug();
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
    if (top_line + m_viewport_height == m_total_lines)
        return;
    if (top_line < 0) {
        top_line = 0;
    } else if (top_line > m_total_lines - m_viewport_height) {
        top_line = m_total_lines - m_viewport_height;
    }

    std::set<size_t> visible_pages;

    size_t top_page = top_line / m_viewport_height;
    visible_pages.insert(top_page);

    if ((top_line % (m_viewport_height -1)) != 0) {
        visible_pages.insert(top_page + 1);
    }

    auto page_it = m_pages.begin();
    while(page_it != m_pages.end()) {
        if (visible_pages.count(page_it->index)) {
            visible_pages.erase(page_it->index);
            handleVisiblePage(*page_it);
            ++page_it;
        } else {
            handleRemovePage(*page_it);
            page_it = m_pages.erase(page_it);
        }
    }

    for (auto new_indexs_it = visible_pages.begin(); new_indexs_it != visible_pages.end(); ++new_indexs_it) {
        auto it_for_page = it_for_row((*new_indexs_it) * m_viewport_height);
        m_pages.push_back( { *new_indexs_it, (*it_for_page)->line(), it_for_page, 0 } );
    }

    m_last_top_line = top_line;
}

std::list<Block *>::iterator ScreenData::it_for_row(int row)
{
    auto it = m_blocks.end();
    int line_for_block = m_total_lines;
    while (it != m_blocks.begin()) {
        --it;
        int end_line = line_for_block - 1;
        line_for_block -= (*it)->lines();
        if (line_for_block <= row && end_line >= row) {
            (*it)->setLine(line_for_block);
            return it;
        }

        if (end_line < row) {
            break;
        }
    }

    ++it;

    return it;
}

void ScreenData::handleVisiblePage(Page &page)
{
    size_t total_pages = m_total_lines / m_viewport_height;
    size_t height;
    if (page.index < total_pages) {
        height = m_viewport_height;
    } else {
        height = m_total_lines - page.line;
    }

    if (height <= page.size)
        return;

    auto it = page.it;
    for (size_t i = 0; i < height; ++it, i++) {
        (*it)->setLine(i + page.line);
        (*it)->dispatchEvents();
    }

    page.size = height;
}

void ScreenData::handleRemovePage(Page &page)
{
    auto it = page.it;
    for (size_t i = 0; i < page.size; ++it, i++) {
        (*it)->releaseTextObjects();
    }
    page.size = 0;
}

