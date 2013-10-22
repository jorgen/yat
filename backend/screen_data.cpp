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

ScreenData::ScreenData(size_t max_scrollback, Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_scrollback(new Scrollback(max_scrollback, this))
    , m_screen_height(0)
    , m_height(0)
    , m_width(0)
    , m_block_count(0)
    , m_old_total_lines(0)
{
    connect(screen, SIGNAL(heightAboutToChange(int, int, int)), this, SLOT(setHeight(int, int, int)));
    connect(screen, SIGNAL(widthAboutToChange(int)), this, SLOT(setWidth(int)));
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
    return m_height + m_scrollback->height();
}

void ScreenData::setHeight(int height, int currentCursorBlock, int currentContentHeight)
{
    qDebug() << "HEIGHT IS:" << height;
    Q_UNUSED(currentContentHeight);
    m_screen_height = height;
    if (height == m_height)
        return;

    if (m_height > height) {
        const int to_remove = m_block_count - height;
        const int remove_from_end = std::min((m_block_count - 1) - currentCursorBlock, to_remove);
        const int remove_from_begin = to_remove - remove_from_end;
        if (remove_from_end > 0) {
            auto it = --m_screen_blocks.end();
            for (int i = 0; i < remove_from_end; --it) {
                i += (*it)->lineCount();
                delete *it;
                m_block_count--;
            }
            m_screen_blocks.erase(++it, m_screen_blocks.end());
        }
        if (remove_from_begin > 0) {
            auto it = m_screen_blocks.begin();
            for (int i = 0; i < remove_from_begin; ++it) {
                i += (*it)->lineCount();
                m_scrollback->addBlock(*it);
                m_block_count--;
            }
            m_screen_blocks.erase(m_screen_blocks.begin(), it);
        }
    } else {
        int rowsToAdd = height - m_block_count;
        m_block_count += rowsToAdd;
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
    m_height = height;
}

void ScreenData::setWidth(int width)
{
    qDebug() << "Width is:" << width;
    m_width = width;
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        (*it)->setWidth(width);
    }
    m_scrollback->setWidth(width);

}


void ScreenData::clearToEndOfLine(int row, int from_char)
{
    auto it = it_for_row_ensure_single_line_block(row);
    (*it)->clearToEnd(from_char);
}

void ScreenData::clearToEndOfScreen(int row)
{
    auto it = it_for_row_ensure_single_line_block(row);
    while(it != m_screen_blocks.end()) {
        clearBlock(it);
        ++it;
    }
}

void ScreenData::clearToBeginningOfLine(int row, int from_char)
{
    auto it = it_for_row_ensure_single_line_block(row);
    (*it)->clearCharacters(0,from_char);
}

void ScreenData::clearToBeginningOfScreen(int row)
{
    auto it = it_for_row_ensure_single_line_block(row);
    if (it != m_screen_blocks.end())
        (*it)->clear();
    while(it != m_screen_blocks.begin()) {
        --it;
        clearBlock(it);
    }
}

void ScreenData::clearLine(int index)
{
    (*it_for_row_ensure_single_line_block(index))->clear();
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

void ScreenData::clearCharacters(int block, int from, int to)
{
    auto it = it_for_row_ensure_single_line_block(block);
    (*it)->clearCharacters(from,to);
}

void ScreenData::deleteCharacters(int block, int from, int to)
{
    auto it = it_for_row(block);
    if (it  == m_screen_blocks.end())
        return;

    int line_in_block = block - (*it)->index();
    int chars_to_line = line_in_block * m_width;

    (*it)->deleteCharacters(chars_to_line + from, chars_to_line + to);
}

CursorDiff ScreenData::replace(int line, int from_char, const QString &text, const TextStyle &style)
{
    return modify(line,from_char,text,style,true);
}

CursorDiff ScreenData::insert(int line, int from_char, const QString &text, const TextStyle &style)
{
    return modify(line,from_char,text,style,false);
}


void ScreenData::moveLine(int from, int to)
{
    if (from == to)
        return;

    auto from_it = it_for_row(from);
    auto to_it = it_for_row(to);

    if ((*to_it)->lineCount() > 1 && (*to_it)->index() != to) {
        auto after_block = (*to_it)->split(to - (*to_it)->index());
        ++to_it;
        to_it = m_screen_blocks.insert(to_it, after_block);
    }

    Block *from_block = nullptr;
    if ((*from_it)->lineCount() > 1) {
        from_block = (*from_it)->takeLine(from - (*from_it)->index());
        from_block->clear();
        m_screen_blocks.insert(to_it, from_block);
        m_block_count++;
    } else {
        (*from_it)->clear();
        m_screen_blocks.splice(to_it, m_screen_blocks, from_it);

    }

}

void ScreenData::insertLine(int row)
{
    auto row_it = it_for_row(row + 1);

    Block *block_to_insert = new Block(m_screen);
    m_screen_blocks.insert(row_it,block_to_insert);
    m_height++;
    m_block_count++;

    if (m_height - m_screen_blocks.front()->lineCount() >= m_screen_height) {
        m_block_count--;
        m_height -= m_screen_blocks.front()->lineCount();
        m_scrollback->addBlock(m_screen_blocks.front());
        m_screen_blocks.pop_front();
    }
}


void ScreenData::fill(const QChar &character)
{
    auto it = --m_screen_blocks.end();
    for (int i = 0; i < m_block_count; --it, i++) {
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
    if (!m_block_count)
        return;
    const int content_height = contentHeight();
    int i = 0;
    for (auto it = m_screen_blocks.begin(); it != m_screen_blocks.end(); ++it) {
        int line = content_height - m_height + i;
        (*it)->setIndex(line);
        (*it)->dispatchEvents();
        i+= (*it)->lineCount();
    }

    if (content_height != m_old_total_lines) {
        m_old_total_lines = contentHeight();
        emit contentHeightChanged();
    }
}

void ScreenData::printStyleInformation() const
{
    auto it = m_screen_blocks.end();
    std::advance(it, -m_block_count);
    for (int i = 0; it != m_screen_blocks.end(); ++it, i++) {
        if (i % 5 == 0) {
            QDebug debug = qDebug();
            debug << "Ruler:";
            (*it)->printRuler(debug);
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

CursorDiff ScreenData::modify(int line, int from_char, const QString &text, const TextStyle &style, bool replace)
{
    auto it = it_for_row(line);
    Block *block = *it;
    size_t lines_before = block->lineCount();
    int start_char = (line - block->index()) * m_width + from_char;
    if (replace) {
        block->replaceAtPos(start_char, text, style);
    } else {
        block->insertAtPos(start_char, text, style);
    }
    int lines_changed = block->lineCount() - lines_before;
    int end_char = (start_char + text.size()) % m_width;
    if (lines_changed > 0) {
        int removed = 0;
        for(auto to_remove = --m_screen_blocks.end();
                removed < lines_changed && to_remove != it; --to_remove) {
            if (removed + (*to_remove)->lineCount() <= lines_changed) {
                removed += (*to_remove)->lineCount();
                delete *to_remove;
                to_remove = m_screen_blocks.erase(to_remove);
                m_block_count--;
            } else {
                int lines_to_remove = (removed + (*to_remove)->lineCount()) - lines_changed;
                for (int i = 0; i < lines_to_remove; i++) {
                    (*it)->removeLine((*it)->lineCount()-1);
                }
                removed += lines_to_remove;
                break;
            }
        }
        for (auto to_remove = m_screen_blocks.begin();
                removed < lines_changed && to_remove != m_screen_blocks.end();) {
            if (removed + (*to_remove)->lineCount() <= lines_changed) {
                removed += (*to_remove)->lineCount();
                m_scrollback->addBlock(*to_remove);
                m_block_count--;
                to_remove = m_screen_blocks.erase(to_remove);
            } else {
                break;
            }
        }
        m_height += lines_changed - removed;
    }
    //qDebug() << "modify:" << line << block->index() << lines_changed  << block->textLine()->size() << text.size() << text << "AFTER:" <<  *block->textLine();
    return { lines_changed, end_char - from_char};
}

void ScreenData::clearBlock(std::list<Block *>::const_iterator line)
{
    int before_count = (*line)->lineCount();
    (*line)->clear();
    int diff_line = before_count - (*line)->lineCount();
    if (diff_line > 0) {
        ++line;
        for (int i = 0; i < diff_line; i++) {
            m_screen_blocks.insert(line, new Block(m_screen));
        }
    }
}

std::list<Block *>::const_iterator ScreenData::it_for_row_ensure_single_line_block(int row)
{
    auto it = it_for_row(row);
    int index = (*it)->index();
    int lines = (*it)->lineCount();

    if (index == row && lines == 1) {
        return it;
    }

    int line_diff = row - index;
    return split_out_row_from_block(it, line_diff);

}

std::list<Block *>::const_iterator ScreenData::split_out_row_from_block(std::list<Block *>::const_iterator it, int row_in_block)
{
    int lines = (*it)->lineCount();

    if (row_in_block == 0 && lines == 1)
        return it;

    if (row_in_block == 0) {
        auto insert_before = (*it)->takeLine(0);
        insert_before->setIndex(row_in_block);
        return m_screen_blocks.insert(it,insert_before);
    } else if (row_in_block == lines -1) {
        auto insert_after = (*it)->takeLine(lines -1);
        insert_after->setIndex(row_in_block);
        ++it;
        return m_screen_blocks.insert(it, insert_after);
    }

    auto half = (*it)->split(row_in_block);
    ++it;
    auto it_width_first = m_screen_blocks.insert(it, half);
    auto the_one = half->takeLine(0);
    return m_screen_blocks.insert(it_width_first,the_one);
}
