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

#ifndef SCREEN_DATA_H
#define SCREEN_DATA_H

#include <QtCore/QVector>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtGui/QClipboard>

#include <functional>

#include "text_style.h"

class Block;
class Screen;

class Page
{
public:
    size_t index;
    size_t line;
    std::list<Block *>::iterator it;
    size_t size;
};

class CursorDiff
{
public:
    int line_diff;
    int char_diff;
};

class ScreenData : public QObject
{
Q_OBJECT
public:
    ScreenData(size_t max_scrollback, Screen *screen);
    ~ScreenData();

    int contentHeight() const;

    Block *blockContainingLine(int line);

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

    void fill(int start_line, int size, const QChar &character);

    void sendSelectionToClipboard(const QPointF &start, const QPointF &end, QClipboard::Mode clipboard);

    void getDoubleClickSelectionArea(const QPointF &cliked, int *start_ret, int *end_ret);

    void dispatchLineEvents();

    void printStyleInformation() const;

    Screen *screen() const;

    void ensureVisiblePages(int top_line);
public slots:
    void setHeight(int height, int currentCursorLine, int currentContentHeight);
    void setWidth(int width);

signals:
    void contentHeightChanged();

private:
    std::list<Block *>::iterator it_for_row(int row);
    void addLines(int lines);
    void removeLines(int lines, int cursorLine);
    void handleVisiblePage(Page &page);
    void handleRemovePage(Page &page);
    void markPageForRowDirty(int row);
    Screen *m_screen;
    int m_total_lines;
    int m_total_lines_old;
    int m_viewport_height;
    int m_last_top_line;
    int m_width;

    std::list<Block *> m_blocks;
    std::list<Block *>::iterator m_top_of_viewport;
    std::list<Page> m_pages;
};

#endif // SCREEN_DATA_H
