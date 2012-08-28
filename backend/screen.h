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

#ifndef TERMINALSCREEN_H
#define TERMINALSCREEN_H

#include <QObject>

#include "text.h"
#include "color_palette.h"
#include "parser.h"
#include "yat_pty.h"
#include "update_action.h"
#include "screen_data.h"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QStack>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtCore/QVarLengthArray>

class Line;

class Screen : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(qreal charWidth READ charWidth NOTIFY charWidthChanged)
    Q_PROPERTY(qreal lineHeight READ lineHeight NOTIFY lineHeightChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY screenTitleChanged)
    Q_PROPERTY(bool cursorVisible READ cursorVisible NOTIFY cursorVisibleChanged)
    Q_PROPERTY(bool cursorBlinking READ cursorBlinking NOTIFY cursorBlinkingChanged)

public:
    explicit Screen(QObject *parent = 0);
    ~Screen();
    
    Q_INVOKABLE void setHeight(int height);
    Q_INVOKABLE int height() const;
    Q_INVOKABLE void setWidth(int width);
    Q_INVOKABLE int width() const;

    void saveScreenData();
    void restoreScreenData();

    QFont font() const;
    void setFont(const QFont &font);
    qreal charWidth() const;
    qreal lineHeight() const;

    void setTextStyle(TextStyle::Style style, bool add = true);
    void resetStyle();
    TextStyle currentTextStyle() const;
    TextStyle defaultTextStyle() const;

    Q_INVOKABLE QColor screenBackground();
    QColor defaultForgroundColor() const;
    QColor defaultBackgroundColor() const;

    void setTextStyleColor(ushort color);
    const ColorPalette *colorPalette() const;

    QPoint cursorPosition() const;
    void moveCursorHome();
    void moveCursorTop();
    void moveCursorUp();
    void moveCursorDown();
    void moveCursorLeft();
    void moveCursorRight(int n_positions);
    void moveCursor(int x, int y);
    void setCursorVisible(bool visible);
    bool cursorVisible();
    void setCursorBlinking(bool blinking);
    bool cursorBlinking();
    void saveCursor();
    void restoreCursor();

    void insertAtCursor(const QString &text);

    void backspace();

    void eraseLine();
    void eraseFromCursorPositionToEndOfLine();
    void eraseFromCurrentLineToEndOfScreen();
    void eraseFromCurrentLineToBeginningOfScreen();
    void eraseToCursorPosition();
    void eraseScreen();

    void lineFeed();
    void reverseLineFeed();
    void insertLines(int count);
    void deleteLines(int count);

    void setScrollArea(int from, int to);

    void setTitle(const QString &title);
    QString title() const;

    void scheduleFlash();

    Q_INVOKABLE Line *at(int i) const;

    Q_INVOKABLE void printScreen() const;

    Q_INVOKABLE void write(const QString &data);

    void dispatchChanges();

    void sendPrimaryDA();
    void sendSecondaryDA();

    void setCharacterMap(const QString &string);
    QString characterMap() const;

signals:
    void moveLines(int from_line, int to_line, int count);

    void linesInserted(int count);
    void linesRemoved(int count);

    void reset();

    void flash();

    void fontChanged();
    void charWidthChanged();
    void lineHeightChanged();


    void dispatchLineChanges();
    void dispatchTextSegmentChanges();

    void screenTitleChanged();

    void cursorPositionChanged(int x, int y);
    void cursorVisibleChanged();
    void cursorBlinkingChanged();
private:

    void readData();
    void scheduleMoveSignal(qint16 from, qint16 to);

    Line *line_at_cursor();
    ScreenData *current_screen_data() const { return m_screen_stack[m_screen_stack.size()-1]; }
    QPoint &current_cursor_pos() { return m_cursor_stack[m_cursor_stack.size()-1]; }
    int current_cursor_x() const { return m_cursor_stack.at(m_cursor_stack.size()-1).x(); }
    int current_cursor_y() const { return m_cursor_stack.at(m_cursor_stack.size()-1).y(); }

    ColorPalette m_palette;
    YatPty m_pty;
    Parser m_parser;

    QVector<ScreenData *> m_screen_stack;
    QVector<QPoint> m_cursor_stack;

    bool m_cursor_visible;
    bool m_cursor_visible_changed;
    bool m_cursor_blinking;
    bool m_cursor_blinking_changed;

    QFont m_font;
    QFontMetricsF m_font_metrics;
    TextStyle m_current_text_style;
    QString m_title;

    QString m_character_map;

    QList<UpdateAction> m_update_actions;
    bool m_flash;
    bool m_cursor_changed;
    bool m_reset;
};

#endif // TERMINALSCREEN_H
