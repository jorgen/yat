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

#include "controll_chars.h"

#include <QtCore/QSocketNotifier>
#include <QtGui/QGuiApplication>

#include <QtCore/QDebug>

#include <float.h>

Screen::Screen(QTextDocument *primary, QTextDocument *secondary, QObject *parent)
    : QObject(parent)
    , m_parser(this)
    , m_cursor_visible(true)
    , m_cursor_visible_changed(false)
    , m_cursor_blinking(true)
    , m_cursor_blinking_changed(false)
    , m_font_metrics(m_font)
    , m_flash(false)
    , m_cursor_changed(false)
    , m_reset(false)
    , m_application_cursor_key_mode(false)
{
    m_screen_stack << new ScreenData(this, primary);
    m_screen_stack << new ScreenData(this, secondary);
    connect(&m_pty, &YatPty::readyRead, this, &Screen::readData);

    m_screen_stack.reserve(2);

    m_cursor_stack << QPoint(0,0);

    m_current_text_style.style = TextStyle::Normal;
    m_current_text_style.foreground = ColorPalette::DefaultForground;
    m_current_text_style.background = ColorPalette::DefaultBackground;

    connect(&m_pty, SIGNAL(hangupReceived()),qGuiApp, SLOT(quit()));
}

Screen::~Screen()
{
    for (int i = 0; i < m_screen_stack.size(); i++) {
        delete m_screen_stack.at(i);

    }
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
    m_pty.setHeight(height, height * lineHeight());
    if (!m_screen_stack.size())
        return;

    ScreenData *data = current_screen_data();
    int size_difference = data->height() - height;

    if (!size_difference)
        return;

    data->setHeight(height);

    if (size_difference > 0) {
        if (current_cursor_y() > 0)
            current_cursor_pos().ry()--;
    }
}

void Screen::setWidth(int width)
{
    m_pty.setWidth(width, width * charWidth());

    if (!m_screen_stack.size())
        return;

    current_screen_data()->setWidth(width);

}

int Screen::width() const
{
    return m_pty.size().width();
}

void Screen::saveScreenData()
{
    Q_ASSERT(false);
    QSize pty_size = m_pty.size();
    current_screen_data()->setHeight(pty_size.height());
    current_screen_data()->setWidth(pty_size.width());
}

void Screen::restoreScreenData()
{
    ScreenData *data = current_screen_data();
    m_screen_stack.remove(m_screen_stack.size()-1);
    delete data;

    QSize pty_size = m_pty.size();
    current_screen_data()->setHeight(pty_size.height());
    current_screen_data()->setWidth(pty_size.width());
}

int Screen::height() const
{
    return current_screen_data()->height();
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
    return { TextStyle::Normal, ColorPalette::DefaultForground, ColorPalette::DefaultBackground };
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
    //fprintf(stderr, "%s", qPrintable(text));
    current_screen_data()->insertAtCursor(text);
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
}

void Screen::eraseFromCurrentLineToEndOfScreen()
{
}

void Screen::eraseFromCurrentLineToBeginningOfScreen()
{
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
    current_screen_data()->insertLineFeed();
}

void Screen::reverseLineFeed()
{
    int cursor_y = current_cursor_y();
    if (cursor_y == current_screen_data()->scrollAreaStart()) {
        moveLine(current_screen_data()->scrollAreaEnd(), cursor_y);
    } else {
        current_cursor_pos().ry()--;
        m_cursor_changed = true;
    }
}

void Screen::insertLines(int count)
{
    for (int i = 0; i < count; i++) {
        moveLine(current_screen_data()->scrollAreaEnd(),current_cursor_y());
    }
}

void Screen::deleteLines(int count)
{
    for (int i = 0; i < count; i++) {
        moveLine(current_cursor_y(),current_screen_data()->scrollAreaEnd());
    }

}

void Screen::setScrollArea(int from, int to)
{
    from--;
    to--;
    current_screen_data()->setScrollArea(from,to);
}

void Screen::pasteFromSelection()
{
    m_pty.write(QGuiApplication::clipboard()->text(QClipboard::Selection).toUtf8());
}

void Screen::pasteFromClipboard()
{
    m_pty.write(QGuiApplication::clipboard()->text(QClipboard::Clipboard).toUtf8());
}

void Screen::doubleClicked(const QPointF &clicked)
{
    int start, end;
    current_screen_data()->getDoubleClickSelectionArea(clicked, &start, &end);
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

void Screen::printScreen() const
{
    current_screen_data()->printScreen();
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

void Screen::setApplicationCursorKeysMode(bool enable)
{
    m_application_cursor_key_mode = enable;
}

bool Screen::applicationCursorKeyMode() const
{
    return m_application_cursor_key_mode;
}

void Screen::sendKey(const QString &text, Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    /// UGH, this functions should be re-written
    char escape = '\0';
    char  control = '\0';
    char  code = '\0';
    QVector<ushort> parameters;
    bool found = true;

    switch(key) {
    case Qt::Key_Up:
        escape = C0::ESC;
        if (m_application_cursor_key_mode)
            control = C1_7bit::SS3;
        else
            control = C1_7bit::CSI;

        code = 'A';
        break;
    case Qt::Key_Right:
        escape = C0::ESC;
        if (m_application_cursor_key_mode)
            control = C1_7bit::SS3;
        else
            control = C1_7bit::CSI;

        code = 'C';
        break;
    case Qt::Key_Down:
        escape = C0::ESC;
        if (m_application_cursor_key_mode)
            control = C1_7bit::SS3;
        else
            control = C1_7bit::CSI;

            code = 'B';
        break;
    case Qt::Key_Left:
        escape = C0::ESC;
        if (m_application_cursor_key_mode)
            control = C1_7bit::SS3;
        else
            control = C1_7bit::CSI;

        code = 'D';
        break;
    case Qt::Key_Insert:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(2);
        code = '~';
        break;
    case Qt::Key_Delete:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(3);
        code = '~';
        break;
    case Qt::Key_Home:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(1);
        code = '~';
        break;
    case Qt::Key_End:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(4);
        code = '~';
        break;
    case Qt::Key_PageUp:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(5);
        code = '~';
        break;
    case Qt::Key_PageDown:
        escape = C0::ESC;
        control = C1_7bit::CSI;
        parameters.append(6);
        code = '~';
        break;
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
        if (m_application_cursor_key_mode) {
            parameters.append((key & 0xff) - 37);
            escape = C0::ESC;
            control = C1_7bit::CSI;
            code = '~';
        }
        break;
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12:
        if (m_application_cursor_key_mode) {
            parameters.append((key & 0xff) - 36);
            escape = C0::ESC;
            control = C1_7bit::CSI;
            code = '~';
        }
        break;
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
        return;
        break;
    default:
        found = false;
    }

    if (found) {
        int term_mods = 0;
        if (modifiers & Qt::ShiftModifier)
            term_mods |= 1;
        if (modifiers & Qt::AltModifier)
            term_mods |= 2;
        if (modifiers & Qt::ControlModifier)
            term_mods |= 4;

        QByteArray toPty;

        if (term_mods) {
            term_mods++;
            parameters.append(term_mods);
        }
        if (escape)
            toPty.append(escape);
        if (control)
            toPty.append(control);
        if (parameters.size()) {
            for (int i = 0; i < parameters.size(); i++) {
                if (i)
                    toPty.append(';');
                toPty.append(QByteArray::number(parameters.at(i)));
            }
        }
        if (code)
            toPty.append(code);
        m_pty.write(toPty);

    } else {
        QString verifiedText = text;
        if (text.isEmpty()) {
            switch (key) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                verifiedText = "\r";
                break;
            case Qt::Key_Backspace:
                verifiedText = "\010";
                break;
            case Qt::Key_Tab:
                verifiedText = "\t";
                break;
            default:
                qDebug() << "Could not find string info for: " << key;
                return;
            }
        }
        QByteArray to_pty;
        QByteArray key_text;
        if (modifiers & Qt::ControlModifier) {
            char key_char = verifiedText.toLocal8Bit().at(0);
            key_text.append(key_char & 0x1F);

        } else {
            key_text = verifiedText.toUtf8();
        }

        if (modifiers &  Qt::AltModifier) {
            to_pty.append(C0::ESC);
        } else if (modifiers & Qt::MetaModifier) {
            to_pty.append(C0::ESC);
            to_pty.append('@');
            to_pty.append(FinalBytesNoIntermediate::Reserved3);
        }

        to_pty.append(key_text);
        m_pty.write(to_pty);
    }
}

YatPty *Screen::pty()
{
    return &m_pty;
}

void Screen::readData(const QByteArray &data)
{
    m_parser.addData(data);
}

void Screen::moveLine(qint16 from, qint16 to)
{
    current_screen_data()->moveLine(from,to);
}

