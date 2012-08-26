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

#include "terminal_screen.h"

#include "text_segment_line.h"

#include "controll_chars.h"

#include <QtCore/QElapsedTimer>

#include <QtCore/QDebug>

TerminalScreen::TerminalScreen(QObject *parent)
    : QObject(parent)
    , m_parser(this)
    , m_cursor_visible(true)
    , m_cursor_blinking(true)
    , m_font_metrics(m_font)
    , m_current_text_style(TextStyle(TextStyle::Normal,ColorPalette::DefaultForground))
    , m_flash(false)
    , m_cursor_changed(false)
{
    connect(&m_pty, &YatPty::readyRead,
            this, &TerminalScreen::readData);

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

TerminalScreen::~TerminalScreen()
{
}


QColor TerminalScreen::screenBackground()
{
    return QColor(Qt::black);
}

QColor TerminalScreen::defaultForgroundColor() const
{
    return m_palette.normalColor(ColorPalette::DefaultForground);
}

QColor TerminalScreen::defaultBackgroundColor() const
{
    return QColor(Qt::transparent);
}

void TerminalScreen::setHeight(int height)
{

    ScreenData *data = current_screen_data();
    int size_difference = data->height() - height;

    if (!size_difference)
        return;

    data->setHeight(height);

    if (size_difference > 0) {
        emit linesRemoved(size_difference);
    } else {
        emit linesInserted(-size_difference);
    }

    m_pty.setHeight(height, height * lineHeight());
}

void TerminalScreen::setWidth(int width)
{
    current_screen_data()->setWidth(width);
    m_pty.setWidth(width, width * charWidth());

}

int TerminalScreen::width() const
{
    return m_pty.size().width();
}

int TerminalScreen::height() const
{
    return m_screen_stack.last()->height();
}

QFont TerminalScreen::font() const
{
    return m_font;
}

void TerminalScreen::setFont(const QFont &font)
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

qreal TerminalScreen::charWidth() const
{
    return m_font_metrics.averageCharWidth();
}

qreal TerminalScreen::lineHeight() const
{
    return m_font_metrics.lineSpacing();
}

void TerminalScreen::setTextStyle(TextStyle::Style style, bool add)
{
    if (add) {
        m_current_text_style.style |= style;
    } else {
        m_current_text_style.style &= !style;
    }
}

void TerminalScreen::resetStyle()
{
    m_current_text_style.background = ColorPalette::DefaultBackground;
    m_current_text_style.foreground = ColorPalette::DefaultForground;
    m_current_text_style.style = TextStyle::Normal;
}

TextStyle TerminalScreen::currentTextStyle() const
{
    return m_current_text_style;
}

TextStyle TerminalScreen::defaultTextStyle() const
{
    return TextStyle(TextStyle::Normal,ColorPalette::DefaultForground);
}

QPoint TerminalScreen::cursorPosition() const
{
    return m_screen_stack.last()->cursorPosition();
}

void TerminalScreen::moveCursorHome()
{
    current_cursor_pos().setX(0);
    m_cursor_changed = true;
}

void TerminalScreen::moveCursorTop()
{
    current_cursor_pos().setY(0);
    m_cursor_changed = true;
}

void TerminalScreen::moveCursorUp()
{
    current_cursor_pos().ry() -= 1;
    m_cursor_changed = true;
}

void TerminalScreen::moveCursorDown()
{
    current_cursor_pos().ry() += 1;
    m_cursor_changed = true;
}

void TerminalScreen::moveCursorLeft()
{
    current_cursor_pos().rx() -= 1;
    m_cursor_changed = true;
}

void TerminalScreen::moveCursorRight(int n_positions)
{
    current_cursor_pos().rx() += n_positions;
    m_cursor_changed = true;
}

void TerminalScreen::moveCursor(int x, int y)
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

void TerminalScreen::setCursorVisible(bool visible)
{
    int emitChange = visible != m_cursor_visible;
    m_cursor_visible = visible;
    if (emitChange)
        cursorVisibleChanged();
}

bool TerminalScreen::cursorVisible()
{
    return m_cursor_visible;
}

void TerminalScreen::setBlinkingCursor(bool blinking)
{
    int emitChange = blinking != m_cursor_blinking;
    m_cursor_blinking = blinking;
    if (emitChange)
        emit cursorBlinkingChanged();
}

bool TerminalScreen::cursorBlinking()
{
    return m_cursor_blinking;
}

void TerminalScreen::insertAtCursor(const QString &text)
{
    QPoint new_cursor_pos = current_screen_data()->insertText(current_cursor_pos(),text);
    current_cursor_pos() = new_cursor_pos;
    m_cursor_changed = true;
}

void TerminalScreen::backspace()
{
    current_cursor_pos().rx()--;
    m_cursor_changed = true;
}

void TerminalScreen::eraseLine()
{
    current_screen_data()->clearLine(current_cursor_y());
}

void TerminalScreen::eraseFromCursorPositionToEndOfLine()
{
    current_screen_data()->clearToEndOfLine(current_cursor_y(), current_cursor_x());
}

void TerminalScreen::eraseFromCurrentLineToEndOfScreen()
{
    current_screen_data()->clearToEndOfScreen(current_cursor_y());

}

void TerminalScreen::eraseFromCurrentLineToBeginningOfScreen()
{
    current_screen_data()->clearToBeginningOfScreen(current_cursor_y());

}

void TerminalScreen::eraseToCursorPosition()
{
    qDebug() << "eraseToCursorPosition NOT IMPLEMENTED!";
}

void TerminalScreen::eraseScreen()
{
    current_screen_data()->clear();
}

void TerminalScreen::setTextStyleColor(ushort color)
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

const ColorPalette *TerminalScreen::colorPalette() const
{
    return &m_palette;
}

void TerminalScreen::lineFeed()
{
    int cursor_y = current_cursor_y();
    if(cursor_y == current_screen_data()->height() -1) {
        current_screen_data()->scrollOneLineUp(cursor_y);
        doScrollOneLineUpAt(cursor_y);
    } else {
        moveCursor(0,cursor_y+2);
        m_cursor_changed = true;
    }
}

void TerminalScreen::setTitle(const QString &title)
{
    m_title = title;
    emit screenTitleChanged();
}

QString TerminalScreen::title() const
{
    return m_title;
}

void TerminalScreen::scheduleFlash()
{
    m_flash = true;
}

TextSegmentLine *TerminalScreen::at(int i) const
{
    return current_screen_data()->at(i);
}

void TerminalScreen::printScreen() const
{
    current_screen_data()->printScreen();
}

void TerminalScreen::write(const QString &data)
{
    m_pty.write(data.toUtf8());
}

void TerminalScreen::dispatchChanges()
{
    for (int i = 0; i < m_update_actions.size(); i++) {
        UpdateAction action = m_update_actions.at(i);
        switch(action.action) {
        case UpdateAction::ScrollUp: {
            int lines_to_move = action.count % (action.from_line + 1);
            if (lines_to_move)
                emit scrollUp(action.from_line, lines_to_move);
        }
            break;
        case UpdateAction::ScrollDown: {
            int lines_to_move = action.count % (height() - action.from_line);
            if (lines_to_move)
                emit scrollDown(action.from_line, lines_to_move);
        }
            break;
        default:
            qDebug() << "unhandeled UpdatAction in TerminalScreen";
            break;
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

    m_update_actions.clear();

    emit dispatchLineChanges();
    emit dispatchTextSegmentChanges();
}

void TerminalScreen::sendPrimaryDA()
{
    m_pty.write(QByteArrayLiteral("\033[?6c"));

}

void TerminalScreen::sendSecondaryDA()
{
    m_pty.write(QByteArrayLiteral("\033[>1;95;0c"));
}

void TerminalScreen::setCharacterMap(const QString &string)
{
    m_character_map = string;
}

QString TerminalScreen::characterMap() const
{
    return m_character_map;
}

void TerminalScreen::readData()
{
    for (int i = 0; i < 20; i++) {
        QByteArray data = m_pty.read();

        m_parser.addData(data);

        if (!m_pty.moreInput())
            break;
    }

    dispatchChanges();
}

void TerminalScreen::doScrollOneLineUpAt(int line)
{
    if (m_update_actions.size() &&
            m_update_actions.last().action == UpdateAction::ScrollUp &&
            m_update_actions.last().from_line == line) {
        m_update_actions.last().count++;
    } else {
        m_update_actions << UpdateAction(UpdateAction::ScrollUp, line, 1);
    }
}

void TerminalScreen::doScrollOneLineDownAt(int line)
{
    if (m_update_actions.size() &&
            m_update_actions.last().action == UpdateAction::ScrollDown &&
            m_update_actions.last().from_line == line) {
        m_update_actions.last().count++;
    } else {
        m_update_actions << UpdateAction(UpdateAction::ScrollDown,line,1);
    }
}
