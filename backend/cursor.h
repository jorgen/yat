/******************************************************************************
* Copyright (c) 2013 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
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

#ifndef CURSOR_H
#define CURSOR_H

#include "text_style.h"
#include "screen.h"
#include "screen_data.h"
#include "scrollback.h"

#include <list>
#include <QtCore/QObject>
#include <QtCore/QPoint>

class Block;

class Position
{
public:
    Position(ScreenData *data)
        : m_screen_data(data)
        , m_it(data->itForBegin())
    {
    }

    int x() const { return m_pos.x(); }
    int y() const { return m_pos.y(); }

    void move(int lines)
    {
        if (!lines)
            return;

        if (y() + lines < 0)
            lines = -y();
        if (abs_screen_start() + y() + lines > contentHeight()) {
            lines = (contentHeight() - 1) - abs_y();
        }
        const size_t abs_end_line = abs_y() + lines;
        m_pos.ry() += lines;
        if (lines < 0) {
            while ((*m_it)->index() > abs_end_line) {
                --m_it;
            }
        } else {
            while ( (*m_it)->index() + (*m_it)->lineCount() <= abs_end_line) {
                ++m_it;
            }
        }

    }

    void moveDown(int lines)
    {
        move(lines);
    }
    void moveUp(int lines)
    {
        move(-lines);
    }

    void setX(int x) {
        m_pos.setX(x);
    }

    void moveX(int x) {
        m_pos.rx() += x;
    }

    void setY(int y) {
        setPos(x(), y);
    }

    void setPos(int x, int y)
    {
        if (y < 0)
            y = 0;
        if (y > height() -1)
            y = height() -1;

        size_t to_abs_y = abs_screen_start() + y;
        int move_lines = to_abs_y - abs_y();

        if (move_lines)
            move(move_lines);
        m_pos.setX(x);
    }

    void linesAddedToEnd(int lines)
    {
        lines = std::min(lines,m_pos.ry());
        m_pos.ry() -= lines;
    }

    void linesAddedTop(int lines)
    {
        m_pos.ry() += lines;
    }

    QPoint point() const { return m_pos; }

    std::list<Block *>::iterator it() const { return m_it; }

    void assert_cursor_position() const
    {
        size_t cursor_it_abs_y = (*m_it)->index();
        size_t cursor_abs_y = abs_screen_start() + y();
        Q_ASSERT(cursor_abs_y >= cursor_it_abs_y);
        Q_ASSERT(cursor_abs_y < cursor_it_abs_y + (*m_it)->lineCount());
    }
public:
    size_t scrollbackHeight() const { return m_screen_data->scrollback()->height(); }
    size_t contentHeight() const { return m_screen_data->contentHeight(); }
    int height() const { return m_screen_data->dataHeight(); }
    size_t abs_y() const { return abs_screen_start() + y(); }
    size_t abs_screen_start() const { return contentHeight() - m_screen_data->screen()->height(); }

    ScreenData *m_screen_data;
    std::list<Block *>::iterator m_it;
    QPoint m_pos;
};

class Cursor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibilityChanged)
    Q_PROPERTY(bool blinking READ blinking WRITE setBlinking NOTIFY blinkingChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
public:
    enum InsertMode {
        Insert,
        Replace
    };

    Cursor(Screen *screen);
    ~Cursor();

    bool visible() const;
    void setVisible(bool visible);

    bool blinking() const;
    void setBlinking(bool blinking);

    void setTextStyle(TextStyle::Style style, bool add = true);
    void resetStyle();
    TextStyle currentTextStyle() const;

    void setTextStyleColor(ushort color);
    ColorPalette *colorPalette() const;

    //QPoint position() const;
    int x() const;
    int y() const;
    int new_x() const { return m_position.x(); }
    int new_y() const { return m_position.y(); }

    void moveOrigin();
    void moveBeginningOfLine();
    void moveUp(int lines = 1);
    void moveDown(int lines = 1);
    void moveLeft(int positions = 1);
    void moveRight(int positions = 1);
    void move(int new_x, int new_y);
    void moveToLine(int line);
    void moveToCharacter(int character);

    void moveToNextTab();
    void setTabStop();
    void removeTabStop();
    void clearTabStops();

    void clearToBeginningOfLine();
    void clearToEndOfLine();
    void clearToBeginningOfScreen();
    void clearToEndOfScreen();
    void clearLine();

    void deleteCharacters(int characters);

    void setWrapAround(bool wrap);
    void addAtCursor(const QByteArray &text);
    void insertAtCursor(const QByteArray &text);
    void replaceAtCursor(const QByteArray &text);

    void lineFeed();
    void reverseLineFeed();

    void setOriginAtMargin(bool atMargin);
    void setScrollArea(size_t from, size_t to);
    void resetScrollArea();

    void scrollUp(int lines);
    void scrollDown(int lines);

    void setTextCodec(QTextCodec *codec);

    void setInsertMode(InsertMode mode);

    inline void notifyChanged();
    void dispatchEvents();

    std::list<Block *>::iterator it();
public slots:
    void setDocumentWidth(int width);
    void setDocumentHeight(size_t height, int currentCursorBlock, int currentScrollBackHeight);

signals:
    void xChanged();
    void yChanged();
    void visibilityChanged();
    void blinkingChanged();

private slots:
    void contentHeightChanged();

public:
    void setScreenData(ScreenData *data);
private:
    ScreenData *screen_data() const { return m_screen->currentScreenData(); }
    int adjusted_new_y() const { return m_origin_at_margin ?
        new_y() - m_top_margin : new_y(); }
    int adjusted_top() const { return m_origin_at_margin ? m_top_margin : 0; }
    int adjusted_bottom() const { return m_origin_at_margin ? m_bottom_margin : m_height - 1; }
    int get_current_line_in_block(std::list<Block *>::iterator it, int y_pos) const
    { return (m_screen_data->scrollback()->height() + y_pos) - (*it)->index(); }
    void setCursorIterator(int y);
    void moveCursorIterator(int lines, int current_line_in_block);
    void moveCursorIteratorToLine(int line);
    Screen *m_screen;
    ScreenData *m_screen_data;
    TextStyle m_current_text_style;
    QPoint m_old_position;
    Position m_position;

    int m_width;
    size_t m_height;

    int m_top_margin;
    int m_bottom_margin;
    bool m_scroll_margins_set;
    bool m_origin_at_margin;

    QVector<int> m_tab_stops;

    bool m_notified;
    bool m_visible;
    bool m_new_visibillity;
    bool m_blinking;
    bool m_new_blinking;
    bool m_wrap_around;
    bool m_content_height_changed;

    QTextDecoder *m_gl_text_codec;
    QTextDecoder *m_gr_text_codec;

    InsertMode m_insert_mode;
};

void Cursor::notifyChanged()
{
    if (!m_notified) {
        m_notified = true;
        m_screen->scheduleEventDispatch();
    }
}

#endif //CUROSOR_H
