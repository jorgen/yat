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

#include "screen.h"

#include "line.h"

#include "controll_chars.h"

#include <QtCore/QElapsedTimer>

#include <QtCore/QDebug>

Screen::Screen(QObject *parent)
    : QObject(parent)
    , m_parser(this)
    , m_cursor_visible(true)
    , m_cursor_visible_changed(false)
    , m_cursor_blinking(true)
    , m_cursor_blinking_changed(false)
    , m_font_metrics(m_font)
    , m_current_text_style(TextStyle(TextStyle::Normal,ColorPalette::DefaultForground))
    , m_flash(false)
    , m_cursor_changed(false)
    , m_reset(false)
{
    connect(&m_pty, &YatPty::readyRead,
            this, &Screen::readData);

    QFont font;
    font.setFamily(QStringLiteral("Courier"));
    font.setPointSize(10);
    font.setBold(true);
    font.setHintingPreference(QFont::PreferNoHinting);
    setFont(font);

    m_screen_stack.reserve(2);
    m_screen_stack << new ScreenData(this);

    m_cursor_stack << QPoint(0,0);

    setWidth(80);
    setHeight(25);

    m_current_text_style.foreground = ColorPalette::DefaultForground;
    m_current_text_style.background = ColorPalette::DefaultBackground;

}

Screen::~Screen()
{
}


QColor Screen::screenBackground()
{
    return QColor(Qt::black);
}

QColor Screen::defaultForgroundColor() const
{
    return m_palette.normalColor(ColorPalette::DefaultForground);
}

QColor Screen::defaultBackgroundColor() const
{
    return QColor(Qt::transparent);
}

void Screen::setHeight(int height)
{

    ScreenData *data = current_screen_data();
    int size_difference = data->height() - height;

    if (!size_difference)
        return;

    data->setHeight(height);

    if(current_cursor_y() >= height)
        current_cursor_pos().setY(height - 1);


    if (size_difference > 0) {
        emit linesRemoved(size_difference);
    } else {
        emit linesInserted(-size_difference);
    }

    m_pty.setHeight(height, height * lineHeight());
}

void Screen::setWidth(int width)
{
    current_screen_data()->setWidth(width);
    m_pty.setWidth(width, width * charWidth());

}

int Screen::width() const
{
    return m_pty.size().width();
}

void Screen::saveScreenData()
{
    m_screen_stack << new ScreenData(this);
    QSize pty_size = m_pty.size();
    current_screen_data()->setHeight(pty_size.height());
    current_screen_data()->setWidth(pty_size.width());
    m_reset = true;

}

void Screen::restoreScreenData()
{
    ScreenData *data = current_screen_data();
    m_screen_stack.remove(m_screen_stack.size()-1);
    delete data;
    QSize pty_size = m_pty.size();
    current_screen_data()->setHeight(pty_size.height());
    current_screen_data()->setWidth(pty_size.width());
    m_reset = true;
}

int Screen::height() const
{
    return m_screen_stack.last()->height();
}

QFont Screen::font() const
{
    return m_font;
}

void Screen::setFont(const QFont &font)
{
    qreal old_width = m_font_metrics.averageCharWidth();
    qreal old_height = m_font_metrics.lineSpacing();
    m_font = font;
    m_font_metrics = QFontMetricsF(font);
    emit fontChanged();
    if (m_font_metrics.averageCharWidth() != old_width)
        emit charWidthChanged();
    if (m_font_metrics.lineSpacing() != old_height)
        emit lineHeightChanged();
}

qreal Screen::charWidth() const
{
    return m_font_metrics.averageCharWidth();
}

qreal Screen::lineHeight() const
{
    return m_font_metrics.lineSpacing();
}

void Screen::setTextStyle(TextStyle::Style style, bool add)
{
    if (add) {
        m_current_text_style.style |= style;
    } else {
        m_current_text_style.style &= !style;
    }
}

void Screen::resetStyle()
{
    m_current_text_style.background = ColorPalette::DefaultBackground;
    m_current_text_style.foreground = ColorPalette::DefaultForground;
    m_current_text_style.style = TextStyle::Normal;
}

TextStyle Screen::currentTextStyle() const
{
    return m_current_text_style;
}

TextStyle Screen::defaultTextStyle() const
{
    return TextStyle(TextStyle::Normal,ColorPalette::DefaultForground);
}

QPoint Screen::cursorPosition() const
{
    return QPoint(current_cursor_x(),current_cursor_y());
}

void Screen::moveCursorHome()
{
    current_cursor_pos().setX(0);
    m_cursor_changed = true;
}

void Screen::moveCursorTop()
{
    current_cursor_pos().setY(0);
    m_cursor_changed = true;
}

void Screen::moveCursorUp()
{
    current_cursor_pos().ry() -= 1;
    m_cursor_changed = true;
}

void Screen::moveCursorDown()
{
    current_cursor_pos().ry() += 1;
    m_cursor_changed = true;
}

void Screen::moveCursorLeft()
{
    current_cursor_pos().rx() -= 1;
    m_cursor_changed = true;
}

void Screen::moveCursorRight(int n_positions)
{
    current_cursor_pos().rx() += n_positions;
    m_cursor_changed = true;
}

void Screen::moveCursor(int x, int y)
{
    if (x != 0)
        x--;
    if (y != 0)
        y--;
    current_cursor_pos().setX(x);
    int height = this->height();
    if (y >= height) {
        qDebug() << "This is wrong";
        current_cursor_pos().setY(height-1);
    } else {
        current_cursor_pos().setY(y);
    }
    m_cursor_changed = true;
}

void Screen::setCursorVisible(bool visible)
{
    m_cursor_visible = visible;
    m_cursor_visible_changed = true;
}

bool Screen::cursorVisible()
{
    return m_cursor_visible;
}

void Screen::setCursorBlinking(bool blinking)
{
    m_cursor_blinking = blinking;
    m_cursor_blinking_changed = true;
}

bool Screen::cursorBlinking()
{
    return m_cursor_blinking;
}

void Screen::saveCursor()
{
    QPoint point = current_cursor_pos();
    m_cursor_stack << point;
}

void Screen::restoreCursor()
{
    if (m_cursor_stack.size() <= 1)
        return;

    m_cursor_stack.remove(m_screen_stack.size()-1);
}

void Screen::insertAtCursor(const QString &text)
{
    Line *line;

    if (current_cursor_x() + text.size() <= width()) {
        line = current_screen_data()->at(current_cursor_y());
        line->insertAtPos(current_cursor_x(), text, m_current_text_style);
        current_cursor_pos().rx() += text.size();
    } else {
        for (int i = 0; i < text.size();) {
            if (current_cursor_x() == width()) {
                current_cursor_pos().setX(0);
                lineFeed();
            }
            QString toLine = text.mid(i,current_screen_data()->width() - current_cursor_x());
            line = current_screen_data()->at(current_cursor_y());
            line->insertAtPos(current_cursor_x(),toLine,m_current_text_style);
            i+= toLine.size();
            current_cursor_pos().rx() += toLine.size();
        }
    }

    m_cursor_changed = true;
}

void Screen::backspace()
{
    current_cursor_pos().rx()--;
    m_cursor_changed = true;
}

void Screen::eraseLine()
{
    current_screen_data()->clearLine(current_cursor_y());
}

void Screen::eraseFromCursorPositionToEndOfLine()
{
    current_screen_data()->clearToEndOfLine(current_cursor_y(), current_cursor_x());
}

void Screen::eraseFromCurrentLineToEndOfScreen()
{
    current_screen_data()->clearToEndOfScreen(current_cursor_y());

}

void Screen::eraseFromCurrentLineToBeginningOfScreen()
{
    current_screen_data()->clearToBeginningOfScreen(current_cursor_y());

}

void Screen::eraseToCursorPosition()
{
    qDebug() << "eraseToCursorPosition NOT IMPLEMENTED!";
}

void Screen::eraseScreen()
{
    current_screen_data()->clear();
}

void Screen::setTextStyleColor(ushort color)
{
    Q_ASSERT(color >= 30 && color < 50);
    if (color < 38) {
        m_current_text_style.foreground = ColorPalette::Color(color - 30);
    } else if (color == 39) {
        m_current_text_style.foreground = ColorPalette::DefaultForground;
    } else if (color >= 40 && color < 48) {
        m_current_text_style.background = ColorPalette::Color(color - 40);
    } else if (color == 49) {
        m_current_text_style.background = ColorPalette::DefaultBackground;
    } else {
        qDebug() << "Failed to set color";
    }
}

const ColorPalette *Screen::colorPalette() const
{
    return &m_palette;
}

void Screen::lineFeed()
{
    int cursor_y = current_cursor_y();
    if(cursor_y == current_screen_data()->scrollAreaEnd()) {
            current_screen_data()->moveLine(current_screen_data()->scrollAreaStart(),cursor_y);
        scheduleMoveSignal(current_screen_data()->scrollAreaStart(),cursor_y);
    } else {
        current_cursor_pos().ry()++;
        m_cursor_changed = true;
    }
}

void Screen::reverseLineFeed()
{
    int cursor_y = current_cursor_y();
    if (cursor_y == current_screen_data()->scrollAreaStart()) {
        current_screen_data()->moveLine(current_screen_data()->scrollAreaEnd(), cursor_y);
        scheduleMoveSignal(current_screen_data()->scrollAreaEnd(), cursor_y);
    } else {
        current_cursor_pos().ry()--;
        m_cursor_changed = true;
    }
}

void Screen::insertLines(int count)
{
    for (int i = 0; i < count; i++) {
        current_screen_data()->moveLine(current_screen_data()->scrollAreaEnd(),current_cursor_y());
        scheduleMoveSignal(current_screen_data()->scrollAreaEnd(),current_cursor_y());
    }
}

void Screen::deleteLines(int count)
{
    for (int i = 0; i < count; i++) {
        current_screen_data()->moveLine(current_cursor_y(),current_screen_data()->scrollAreaEnd());
        scheduleMoveSignal(current_cursor_y(),current_screen_data()->scrollAreaEnd());
    }

}

void Screen::setScrollArea(int from, int to)
{
    from--;
    to--;
    current_screen_data()->setScrollArea(from,to);
}

void Screen::setTitle(const QString &title)
{
    m_title = title;
    emit screenTitleChanged();
}

QString Screen::title() const
{
    return m_title;
}

void Screen::scheduleFlash()
{
    m_flash = true;
}

Line *Screen::at(int i) const
{
    return current_screen_data()->at(i);
}

void Screen::printScreen() const
{
    current_screen_data()->printScreen();
}

void Screen::write(const QString &data)
{
    m_pty.write(data.toUtf8());
}

void Screen::dispatchChanges()
{
    if (m_reset) {
        emit reset();
        m_update_actions.clear();
        m_reset = false;
    } else {
        for (int i = 0; i < m_update_actions.size(); i++) {
            UpdateAction action = m_update_actions.at(i);
            switch(action.action) {
            case UpdateAction::MoveLine: {
                if (action.from_line > action.to_line) {
                    int lines_to_move = action.count % ((action.from_line - action.to_line) + 1);
                    if (lines_to_move)
                        emit moveLines(action.from_line - (lines_to_move -1), action.to_line, lines_to_move);
                } else {
                    int lines_to_move = action.count % ((action.to_line - action.from_line) + 1);
                    if (lines_to_move)
                        emit moveLines(action.from_line, action.to_line - (lines_to_move -1), lines_to_move);
                }
            }
                break;
            default:
                qDebug() << "unhandeled UpdatAction in TerminalScreen";
                break;
            }
        }
    }

    if (m_flash) {
        m_flash = false;
        emit flash();
    }

    if (m_cursor_changed) {
        m_cursor_changed = false;
        emit cursorPositionChanged(current_cursor_x(), current_cursor_y());
    }

    if (m_cursor_visible_changed) {
        m_cursor_visible_changed = false;
        emit cursorVisibleChanged();
    }

    if (m_cursor_blinking_changed) {
        m_cursor_blinking_changed = false;
        emit cursorBlinkingChanged();
    }

    m_update_actions.clear();

    emit dispatchLineChanges();
    emit dispatchTextSegmentChanges();
}

void Screen::sendPrimaryDA()
{
    m_pty.write(QByteArrayLiteral("\033[?6c"));

}

void Screen::sendSecondaryDA()
{
    m_pty.write(QByteArrayLiteral("\033[>1;95;0c"));
}

void Screen::setCharacterMap(const QString &string)
{
    m_character_map = string;
}

QString Screen::characterMap() const
{
    return m_character_map;
}

void Screen::readData()
{
    for (int i = 0; i < 60; i++) {
        QByteArray data = m_pty.read();

        m_parser.addData(data);

        if (!m_pty.moreInput())
            break;
    }

    dispatchChanges();
}

void Screen::scheduleMoveSignal(qint16 from, qint16 to)
{
    if (m_update_actions.size() &&
            m_update_actions.last().action == UpdateAction::MoveLine &&
            m_update_actions.last().from_line == from &&
            m_update_actions.last().to_line == to) {
        m_update_actions.last().count++;
    } else {
        m_update_actions << UpdateAction(UpdateAction::MoveLine, from, to, 1);
    }
}
