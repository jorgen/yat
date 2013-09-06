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
#include "cursor.h"

#include "controll_chars.h"

#include <QtCore/QTimer>
#include <QtCore/QSocketNotifier>
#include <QtGui/QGuiApplication>

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlComponent>

#include <QtCore/QDebug>

#include <float.h>

Screen::Screen(QObject *parent)
    : QObject(parent)
    , m_palette(new ColorPalette(this))
    , m_parser(this)
    , m_timer_event_id(0)
    , m_width(0)
    , m_height(0)
    , m_insert_mode(Replace)
    , m_selection_valid(false)
    , m_selection_moved(0)
    , m_flash(false)
    , m_cursor_changed(false)
    , m_application_cursor_key_mode(false)
    , m_fast_scroll(true)
{
    connect(&m_pty, &YatPty::readyRead, this, &Screen::readData);

    m_screen_stack.reserve(2);

    Cursor *cursor = new Cursor(this);
    m_cursor_stack << cursor;
    m_new_cursors << cursor;

    connect(&m_pty, SIGNAL(hangupReceived()),qGuiApp, SLOT(quit()));
}

Screen::~Screen()
{
    for (int i = 0; i < m_screen_stack.size(); i++) {
        delete m_screen_stack.at(i);
    }
}


QColor Screen::defaultForgroundColor() const
{
    return m_palette->normalColor(ColorPalette::DefaultForground);
}

QColor Screen::defaultBackgroundColor() const
{
    return m_palette->normalColor(ColorPalette::DefaultBackground);
}

void Screen::setHeight(int height)
{
    return setHeight(height, false);
}

void Screen::setHeight(int height, bool emitChanged)
{
    if (height == m_height)
        return;

    m_height = height;

    if (!m_screen_stack.size()) {
            m_screen_stack << new ScreenData(this);
    }

    for (int i = 0; i < m_screen_stack.size(); i++) {
        m_screen_stack[i]->setHeight(height);
    }

    for (int i = 0; i< m_cursor_stack.size(); i++) {
        m_cursor_stack[i]->setDocumentHeight(height);
    }

    m_pty.setHeight(height, height * 10);

    if (emitChanged)
        emit heightChanged();
}

int Screen::height() const
{
    return m_height;
}

void Screen::setWidth(int width)
{
    return setWidth(width, false);
}

void Screen::setWidth(int width, bool emitChanged)
{
    if (width == m_width)
        return;

    m_width = width;

    if (!m_screen_stack.size())
        m_screen_stack << new ScreenData(this);

    emit widthAboutToChange(width);

    m_pty.setWidth(width, width * 10);

    if (emitChanged)
        emit widthChanged();
}

int Screen::width() const
{
    return m_width;
}

void Screen::saveScreenData()
{
    ScreenData *new_data = new ScreenData(this);
    QSize pty_size = m_pty.size();
    new_data->setHeight(pty_size.height());
    new_data->setWidth(pty_size.width());

    for (int i = 0; i < new_data->height(); i++) {
        currentScreenData()->at(i)->setVisible(false);
    }

    m_screen_stack << new_data;

    setSelectionEnabled(false);

}

void Screen::restoreScreenData()
{
    ScreenData *data = currentScreenData();
    m_screen_stack.remove(m_screen_stack.size()-1);
    delete data;

    for (int i = 0; i < currentScreenData()->height(); i++) {
        currentScreenData()->at(i)->setVisible(true);
    }

    setSelectionEnabled(false);
}

void Screen::setInsertMode(InsertMode mode)
{
    m_insert_mode = mode;
}



TextStyle Screen::defaultTextStyle() const
{
    TextStyle style;
    style.style = TextStyle::Normal;
    style.forground = ColorPalette::DefaultForground;
    style.background = ColorPalette::DefaultBackground;
    return style;
}

void Screen::saveCursor()
{
    Cursor *new_cursor = new Cursor(this);
    if (m_cursor_stack.size())
        m_cursor_stack.last()->setVisible(false);
    m_cursor_stack << new_cursor;
    m_new_cursors << new_cursor;
}

void Screen::restoreCursor()
{
    if (m_cursor_stack.size() <= 1)
        return;

    Cursor *to_be_deleted = m_cursor_stack.takeLast();
    delete to_be_deleted;
    m_cursor_stack.last()->setVisible(true);
}

void Screen::clearScreen()
{
    currentScreenData()->clear();
}


ColorPalette *Screen::colorPalette() const
{
    return m_palette;
}

void Screen::fill(const QChar character)
{
    currentScreenData()->fill(character);
}

void Screen::clear()
{
    fill(QChar(' '));
}

void Screen::setFastScroll(bool fast)
{
    m_fast_scroll = fast;
}

bool Screen::fastScroll() const
{
    return m_fast_scroll;
}

QPointF Screen::selectionAreaStart() const
{
    return m_selection_start;
}

void Screen::setSelectionAreaStart(const QPointF &start)
{
    bool emitChanged = m_selection_start != start;
    m_selection_start = start;
    setSelectionValidity();
    if (emitChanged)
        emit selectionAreaStartChanged();
}

QPointF Screen::selectionAreaEnd() const
{
    return m_selection_end;
}

void Screen::setSelectionAreaEnd(const QPointF &end)
{
    bool emitChanged = m_selection_end != end;
    m_selection_end = end;
    setSelectionValidity();
    if (emitChanged)
        emit selectionAreaEndChanged();
}

bool Screen::selectionEnabled() const
{
    return m_selection_valid;
}

void Screen::setSelectionEnabled(bool enabled)
{
    bool emitchanged = m_selection_valid != enabled;
    m_selection_valid = enabled;
    if (emitchanged)
        emit selectionEnabledChanged();
}

void Screen::sendSelectionToClipboard() const
{
    currentScreenData()->sendSelectionToClipboard(m_selection_start, m_selection_end, QClipboard::Clipboard);
}

void Screen::sendSelectionToSelection() const
{
    currentScreenData()->sendSelectionToClipboard(m_selection_start, m_selection_end, QClipboard::Selection);
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
    currentScreenData()->getDoubleClickSelectionArea(clicked, &start, &end);
    setSelectionAreaStart(QPointF(start,clicked.y()));
    setSelectionAreaEnd(QPointF(end,clicked.y()));
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
    return currentScreenData()->at(i);
}

void Screen::printScreen() const
{
    currentScreenData()->printStyleInformation();
}

void Screen::scheduleEventDispatch()
{
    if (!m_timer_event_id)
        m_timer_event_id = startTimer(3);

    m_time_since_parsed.restart();
}

void Screen::dispatchChanges()
{
    currentScreenData()->dispatchLineEvents();
    emit dispatchTextSegmentChanges();

    if (m_flash) {
        m_flash = false;
        emit flash();
    }

    for (int i = 0; i < m_new_cursors.size(); i++) {
        emit cursorCreated(m_new_cursors.at(i));
    }
    m_new_cursors.clear();

    for (int i = 0; i < m_cursor_stack.size(); i++) {
        m_cursor_stack[i]->dispatchEvents();
    }

    if (m_selection_valid && m_selection_moved) {
        if (m_selection_start.y() < 0 ||
                m_selection_end.y() >= currentScreenData()->height()) {
            setSelectionEnabled(false);
        } else {
            emit selectionAreaStartChanged();
            emit selectionAreaEndChanged();
        }
    }
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

static bool hasControll(Qt::KeyboardModifiers modifiers)
{
#ifdef Q_OS_MAC
    return modifiers & Qt::MetaModifier;
#else
    return modifiers & Qt::ControlModifier;
#endif
}

static bool hasMeta(Qt::KeyboardModifiers modifiers)
{
#ifdef Q_OS_MAC
    return modifiers & Qt::ControlModifier;
#else
    return modifiers & Qt::MetaModifier;
#endif
}

void Screen::sendKey(const QString &text, Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    /// UGH, this function should be re-written
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
        QString verifiedText = text.simplified();
        if (verifiedText.isEmpty()) {
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
            case Qt::Key_Control:
            case Qt::Key_Meta:
            case Qt::Key_Alt:
            case Qt::Key_Shift:
                return;
            case Qt::Key_Space:
                verifiedText = " ";
                break;
            default:
                return;
            }
        }
        QByteArray to_pty;
        QByteArray key_text;
        if (hasControll(modifiers)) {
            char key_char = verifiedText.toLocal8Bit().at(0);
            key_text.append(key_char & 0x1F);
        } else {
            key_text = verifiedText.toUtf8();
        }

        if (modifiers &  Qt::AltModifier) {
            to_pty.append(C0::ESC);
        }

        if (hasMeta(modifiers)) {
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

    scheduleEventDispatch();
}

void Screen::setSelectionValidity()
{
    if (m_selection_end.y() > m_selection_start.y() ||
            (m_selection_end.y() == m_selection_start.y() &&
             m_selection_end.x() > m_selection_start.x())) {
        setSelectionEnabled(true);
    } else {
        setSelectionEnabled(false);
    }
}


void Screen::timerEvent(QTimerEvent *)
{
    if (m_timer_event_id && m_time_since_parsed.elapsed() > 2) {
        killTimer(m_timer_event_id);
        m_timer_event_id = 0;
        dispatchChanges();
    }
}
