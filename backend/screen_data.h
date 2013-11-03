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

#ifndef SCREENDATA_H
#define SCREENDATA_H

#include "text_style.h"
#include "block.h"

#include <QtCore/QVector>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtGui/QClipboard>

#include <functional>

class Screen;
class Scrollback;

class CursorDiff
{
public:
    int line;
    int character;
};

class ScreenData : public QObject
{
Q_OBJECT
public:
    ScreenData(size_t max_scrollback, Screen *screen);
    ~ScreenData();

    int contentHeight() const;

    void clearToEndOfLine(int row, int from_char);
    void clearToEndOfScreen(int row);
    void clearToBeginningOfLine(int row, int from_char);
    void clearToBeginningOfScreen(int row);
    void clearLine(int index);
    void clear();
    void releaseTextObjects();

    void clearCharacters(int line, int from, int to);
    void deleteCharacters(int line, int from, int to);

    CursorDiff replace(int line, int from_char, const QString &text, const TextStyle &style);
    CursorDiff insert(int line, int from_char, const QString &text, const TextStyle &style);

    void moveLine(int from, int to);
    void insertLine(int row);

    void fill(const QChar &character);

    void dispatchLineEvents();

    void printRuler(QDebug &debug) const;
    void printStyleInformation() const;

    Screen *screen() const;

    void ensureVisiblePages(int top_line);

    Scrollback *scrollback() const;
public slots:
    void setHeight(int height, int currentCursorLine, int currentContentHeight);
    void setWidth(int width);

signals:
    void contentHeightChanged();

private:
    CursorDiff modify(int line, int from_char, const QString &text, const TextStyle &style, bool replace);
    inline std::list<Block *>::const_iterator it_for_row(int row) const;
    void clearBlock(std::list<Block *>::const_iterator line);
    std::list<Block *>::const_iterator it_for_row_ensure_single_line_block(int row);
    std::list<Block *>::const_iterator split_out_row_from_block(std::list<Block *>::const_iterator block_it, int row_in_block);
    void push_at_most_to_scrollback(int lines);
    void reclaim_at_least(int lines);
    void remove_lines_from_end(int lines);
    void ensure_at_least_height(int height);
    Screen *m_screen;
    Scrollback *m_scrollback;
    int m_screen_height;
    int m_height;
    int m_width;
    int m_block_count;
    int m_old_total_lines;

    std::list<Block *> m_screen_blocks;
};

std::list<Block *>::const_iterator ScreenData::it_for_row(int row) const
{
    auto it = m_screen_blocks.end();
    int line_for_block = m_screen_height;
    while (it != m_screen_blocks.begin()) {
        --it;
        int end_line = line_for_block - 1;
        line_for_block -= (*it)->lineCount();
        if (line_for_block <= row && end_line >= row) {
            (*it)->setIndex(line_for_block);
            return it;
        }

        if (end_line < row) {
            break;
        }
    }

    return m_screen_blocks.end();
}
#endif // SCREENDATA_H
