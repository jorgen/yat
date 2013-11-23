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
#include "cursor.h"

#include <stdio.h>

#include <QtGui/QGuiApplication>
#include <QtCore/QDebug>

ScreenData::ScreenData(size_t max_scrollback, Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_scrollback(new Scrollback(size_t(0) -1, this))
    , m_screen_height(0)
    , m_height(0)
    , m_width(0)
    , m_block_count(0)
    , m_old_total_lines(0)
{
    connect(screen, SIGNAL(heightAboutToChange(size_t , int, int)), this, SLOT(setHeight(size_t, int, int)));
    connect(screen, SIGNAL(widthAboutToChange(int)), this, SLOT(setWidth(int)));
}

ScreenData::~ScreenData()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        delete *it;
    }
    delete m_scrollback;
}

void ScreenData::setHeight(size_t height, int currentCursorLine, int currentContentHeight)
{
    Q_UNUSED(currentContentHeight);
    m_screen_height = height;
    if (height == m_height)
        return;

    if (m_height > height) {
        const size_t to_remove = m_height - height;
        const int remove_from_end = std::min((m_height -1) - currentCursorLine, to_remove);
        const int remove_from_start = to_remove - remove_from_end;

        if (remove_from_end) {
            remove_lines_from_end(remove_from_end);
        }
        if (remove_from_start) {
            push_at_most_to_scrollback(remove_from_start);
        }
    } else {
        ensure_at_least_height(height);
    }
}

void ScreenData::setWidth(int width)
{
    m_width = width;

    for (Block *block : m_screen_blocks) {
        int before_count = block->lineCount();
        block->setWidth(width);
        m_height += block->lineCount() - before_count;
    }

    if (m_height > m_screen_height) {
        push_at_most_to_scrollback(m_height - m_screen_height);
    } else {
        ensure_at_least_height(m_screen_height);
    }

    m_scrollback->setWidth(width);
}


void ScreenData::clearToEndOfLine(Cursor *cursor)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    (*it)->clearToEnd(cursor->new_x());
}

void ScreenData::clearToEndOfScreen(Cursor *cursor)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    while(it != m_screen_blocks.end()) {
        clearBlock(it);
        ++it;
    }
}

void ScreenData::clearToBeginningOfLine(Cursor *cursor)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    (*it)->clearCharacters(0,cursor->new_x());
}

void ScreenData::clearToBeginningOfScreen(Cursor *cursor)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    if (it != m_screen_blocks.end())
        (*it)->clear();
    while(it != m_screen_blocks.begin()) {
        --it;
        clearBlock(it);
    }
}

void ScreenData::clearLine(Cursor *cursor)
{
    (*it_for_cursor_ensure_single_line_block(cursor))->clear();
}

void ScreenData::clear()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        clearBlock(it);
    }
}

void ScreenData::releaseTextObjects()
{
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        (*it)->releaseTextObjects();
    }
}

void ScreenData::clearCharacters(Cursor *cursor, int to)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    (*it)->clearCharacters(cursor->new_x(),to);
}

void ScreenData::deleteCharacters(Cursor *cursor, int to)
{
    auto it = it_for_cursor_ensure_single_line_block(cursor);
    if (it  == m_screen_blocks.end())
        return;

    int line_in_block = cursor->new_y() - (*it)->index();
    int chars_to_line = line_in_block * m_width;

    (*it)->deleteCharacters(chars_to_line + cursor->new_x(), chars_to_line + to);
}

CursorDiff ScreenData::replace(Cursor *cursor, const QString &text, const TextStyle &style)
{
    return modify(cursor,text,style,true);
}

CursorDiff ScreenData::insert(Cursor *cursor, const QString &text, const TextStyle &style)
{
    return modify(cursor,text,style,false);
}


void ScreenData::moveLine(int from, int to)
{
    if (from == to)
        return;

    int actual_to = to;
    if (to > from)
        actual_to++;
    auto from_it = it_for_row_ensure_single_line_block(from);
    auto to_it = it_for_row_ensure_single_line_block(to);

    auto start_it =  to < from? to_it : from_it;
    auto start_index = (*start_it)->index();
    if (from < to)
        ++start_index;

    (*from_it)->clear();
    m_screen_blocks.splice(to_it, m_screen_blocks, from_it);

    while (start_it != m_screen_blocks.end()) {
        (*start_it)->setIndex(start_index);
        start_index++;
        ++start_it;
    }
}

void ScreenData::insertLine(Cursor *cursor)
{
    auto row_it = ++cursor->it();
    int index = (*cursor->it())->index() + (*cursor->it())->lineCount();

    auto to_index_update = m_screen_blocks.insert(row_it,new Block(m_screen, index));
    adjust_indexes(++to_index_update, index + 1);
    m_height++;
    m_block_count++;

    if (m_height > m_screen_height)
        push_at_most_to_scrollback(m_height - m_screen_height);
}


void ScreenData::fill(const QChar &character)
{
    clear();
    auto it = --m_screen_blocks.end();
    for (int i = 0; i < m_block_count; --it, i++) {
        QString fill_str(m_screen->width(), character);
        (*it)->replaceAtPos(0, fill_str, m_screen->defaultTextStyle());
    }
}

void ScreenData::dispatchLineEvents()
{
    if (!m_block_count)
        return;
    const int content_height = contentHeight();
    for (Block *block : m_screen_blocks) {
        block->dispatchEvents();
    }

    if (content_height != m_old_total_lines) {
        m_old_total_lines = contentHeight();
        emit contentHeightChanged();
    }
}

void ScreenData::printRuler(QDebug &debug) const
{
    QString ruler = QString("|----i----").repeated((m_width/10)+1).append("|");
    debug << "  " << (void *) this << ruler;
}

void ScreenData::printStyleInformation() const
{
    auto it = m_screen_blocks.end();
    std::advance(it, -m_block_count);
    for (int i = 0; it != m_screen_blocks.end(); ++it, i++) {
        if (i % 5 == 0) {
            QDebug debug = qDebug();
            debug << "Ruler:";
            printRuler(debug);
        }
        QDebug debug = qDebug();
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

CursorDiff ScreenData::modify(Cursor *cursor, const QString &text, const TextStyle &style, bool replace)
{
    auto it = cursor->it();
    Block *block = *it;
    int line_in_block = cursor->new_y() - (block->index() - abs_screen_start());
    int start_char = line_in_block * m_width + cursor->new_x();
    size_t lines_before = block->lineCount();
    int lines_changed =
        block->lineCountAfterModified(start_char, text.size(), replace) - lines_before;
    m_height += lines_changed;
    if (replace) {
        block->replaceAtPos(start_char, text, style);
    } else {
        block->insertAtPos(start_char, text, style);
    }

    if (lines_changed > 0) {
        int removed = 0;
        auto to_merge_inn = it;
        ++to_merge_inn;
        while(removed < lines_changed && to_merge_inn != m_screen_blocks.end()) {
            Block *to_be_reduced = *to_merge_inn;
            bool remove_block = removed + to_be_reduced->lineCount() <= lines_changed;
            int lines_to_remove = remove_block ? to_be_reduced->lineCount() : to_be_reduced->lineCount() - (lines_changed - removed);
            block->moveLinesFromBlock(to_be_reduced, 0, lines_to_remove);
            removed += lines_to_remove;
            if (remove_block) {
                delete to_be_reduced;
                to_merge_inn = m_screen_blocks.erase(to_merge_inn);
                m_block_count--;
            } else {
                ++to_merge_inn;
            }
        }

        m_height -= removed;
    }
    if (m_height > m_screen_height)
        push_at_most_to_scrollback(m_height - m_screen_height);
    Q_ASSERT(contentHeight() >= (*cursor->it())->index() + (*cursor->it())->lineCount());

    int end_char = (start_char + text.size()) % m_width;
    return { lines_changed, end_char - cursor->new_x()};
}

void ScreenData::clearBlock(std::list<Block *>::iterator line)
{
    int before_count = (*line)->lineCount();
    (*line)->clear();
    int diff_line = before_count - (*line)->lineCount();
    if (diff_line > 0) {
        int index = (*line)->index() + before_count;
        ++line;
        auto last_inserted = line;
        for (int i = 0; i < diff_line; i++, index++) {
            last_inserted = m_screen_blocks.insert(line, new Block(m_screen, index));
        }
        m_block_count+=diff_line;
        ++last_inserted;
        for (auto it = last_inserted; it != m_screen_blocks.end(); ++it) {
            (*it)->setIndex((*it)->index() + diff_line);
        }
    }
}

std::list<Block *>::iterator ScreenData::it_for_row_ensure_single_line_block(int row)
{
    auto it = itForRow(row);
    int index = (*it)->index();
    int lines = (*it)->lineCount();

    if (index == row && lines == 1) {
        return it;
    }

    int line_diff = row - index;
    return split_out_row_from_block(it, line_diff);
}

std::list<Block *>::iterator ScreenData::it_for_cursor_ensure_single_line_block(Cursor *cursor)
{
    auto it = cursor->it();
    int index = (*it)->index();
    int lines = (*it)->lineCount();

    if (index == cursor->new_y() && lines == 1) {
        return it;
    }

    int line_diff = cursor->new_y() - index;
    return split_out_row_from_block(it, line_diff);
}

std::list<Block *>::iterator ScreenData::split_out_row_from_block(std::list<Block *>::iterator it, int row_in_block)
{
    int lines = (*it)->lineCount();

    if (row_in_block == 0 && lines == 1)
        return it;

    if (row_in_block == 0) {
        auto insert_before = (*it)->takeLine(0);
        m_block_count++;
        auto insert_index = m_screen_blocks.insert(it,insert_before);
        adjust_indexes(insert_index, (*it)->index());
        return insert_index;
    } else if (row_in_block == lines -1) {
        auto insert_after = (*it)->takeLine(lines -1);
        ++it;
        m_block_count++;
        auto inserted_it = m_screen_blocks.insert(it, insert_after);
        adjust_indexes(inserted_it, row_in_block);
        return inserted_it;
    }

    auto index_to_adjust_from = (*it)->index();
    auto it_to_adjust_from = it;
    auto half = (*it)->split(row_in_block);
    ++it;
    auto it_width_first = m_screen_blocks.insert(it, half);
    auto the_one = half->takeLine(0);
    m_block_count+=2;
    auto to_return = m_screen_blocks.insert(it_width_first,the_one);
    adjust_indexes(it_to_adjust_from, index_to_adjust_from);
    return to_return;
}

void ScreenData::push_at_most_to_scrollback(size_t lines)
{
    if (lines >= m_height)
        lines = m_height -1;
    size_t pushed = 0;
    auto it = m_screen_blocks.begin();
    while (pushed + (*it)->lineCount() <= lines) {
        m_block_count--;
        pushed += (*it)->lineCount();
        m_scrollback->addBlock(*it);
        it = m_screen_blocks.erase(it);
    }
    m_height -= pushed;
}

void ScreenData::reclaim_at_least(int lines)
{
    int lines_reclaimed = 0;
    while (m_scrollback->blockCount() && lines_reclaimed < lines) {
        Block *block = m_scrollback->reclaimBlock();
        m_height += block->lineCount();
        lines_reclaimed += block->lineCount();
        m_block_count++;
        m_screen_blocks.push_front(block);
    }
}

void ScreenData::remove_lines_from_end(int lines)
{
    int removed = 0;
    auto it = m_screen_blocks.end();
    while (it != m_screen_blocks.begin() && removed < lines) {
        --it;
        const int block_height = (*it)->lineCount();
        if (removed + block_height <= lines) {
            removed += block_height;
            m_height -= block_height;
            m_block_count--;
            delete (*it);
            it = m_screen_blocks.erase(it);
        } else {
            const int to_remove = lines - removed;
            removed += to_remove;
            m_height -= to_remove;
            Block *block = *it;
            for (int i = 0; i < to_remove; i++) {
                block->removeLine(block->lineCount()-1);
            }
        }
    }
}

void ScreenData::ensure_at_least_height(size_t height)
{
    if (m_height > height)
        return;

    int to_grow = height - m_height;
    reclaim_at_least(to_grow);

    int last_index = m_screen_blocks.empty() ? 0 :
            m_screen_blocks.back()->index() + 1;

    if (height > m_height) {
        int to_insert = height - m_height;
        for (int i = 0; i < to_insert; i++, last_index++) {
            m_screen_blocks.push_back(new Block(m_screen, last_index));
        }
        m_height += to_insert;
        m_block_count += to_insert;
    }
}

void ScreenData::adjust_indexes(std::list<Block *>::iterator it, int startIndex)
{
    while (it != m_screen_blocks.end()) {
        (*it)->setIndex(startIndex);
        startIndex += (*it)->lineCount();
        ++it;
    }
}
