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

#include <QtCore/QObject>

class Cursor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibilityChanged)
    Q_PROPERTY(bool blinking READ blinking WRITE setBlinking NOTIFY blinkingChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
public:
    Cursor(Screen *screen);

    void setDocumentWidth(int width);
    void setDocumentHeight(int height);

    bool visible() const;
    void setVisible(bool visible);

    bool blinking() const;
    void setBlinking(bool blinking);

    void setTextStyle(TextStyle::Style style, bool add = true);
    void resetStyle();
    TextStyle currentTextStyle() const;

    void setTextStyleColor(ushort color);
    ColorPalette *colorPalette() const;

    QPoint position() const;
    int x() const;
    int y() const;
    QPoint rawPosition() const;

    void moveOrigin();
    void moveBeginningOfLine();
    void moveUp(int lines = 1);
    void moveDown(int lines = 1);
    void moveLeft(int positions = 1);
    void moveRight(int positions = 1);
    void move(int new_x, int new_y);
    void moveToLine(int line);
    void moveToCharacter(int character);

    void clearToBeginningOfLine();
    void clearToEndOfLine();
    void clearToBeginningOfScreen();
    void clearToEndOfScreen();
    void clearLine();

    void deleteCharacters(int characters);

    void replaceAtCursor(const QString &text);
    void insertAtCursor(const QString &text);

    void lineFeed();
    void reverseLineFeed();

    void setScrollArea(int from, int to);
    void resetScrollArea();

    void scrollUp(int lines);
    void scrollDown(int lines);

    inline void notifyChanged();
    void dispatchEvents();

signals:
    void xChanged();
    void yChanged();
    void visibilityChanged();
    void blinkingChanged();

private:
    ScreenData *screen_data() const { return m_screen->currentScreenData(); }
    int new_x() const { return m_new_position.x(); }
    int &new_rx() { return m_new_position.rx(); }
    int new_y() const { return m_new_position.y(); };
    int &new_ry() { return m_new_position.ry(); }
    Screen *m_screen;
    TextStyle m_current_text_style;
    QPoint m_position;
    QPoint m_new_position;

    int m_document_width;
    int m_document_height;

    int m_top_margin;
    int m_bottom_margin;
    bool m_scroll_margins_set;

    bool m_notified;
    bool m_visible;
    bool m_new_visibillity;
    bool m_blinking;
    bool m_new_blinking;
    bool m_insert_mode;
};

void Cursor::notifyChanged()
{
    if (!m_notified) {
        m_notified = true;
        m_screen->scheduleEventDispatch();
    }
}

#endif //CUROSOR_H
