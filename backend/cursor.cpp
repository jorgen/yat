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

#include "cursor.h"

#include "line.h"

Cursor::Cursor(Screen* screen)
    : m_screen(screen)
    , m_current_text_style(screen->defaultTextStyle())
    , m_position(0,0)
    , m_new_position(0,0)
    , m_document_width(0)
    , m_document_height(0)
    , m_top_margin(-1)
    , m_bottom_margin(-1)
    , m_notified(false)
    , m_visible(true)
    , m_new_visibillity(true)
    , m_blinking(false)
    , m_new_blinking(false)
{
    std::function<void(int)> set_width_function = std::bind(&Cursor::setDocumentWidth, this, std::placeholders::_1);
    std::function<void(int)> set_height_function = std::bind(&Cursor::setDocumentHeight, this, std::placeholders::_1);
    QObject::connect(screen, &Screen::widthAboutToChange, set_width_function);
    QObject::connect(screen, &Screen::heightAboutToChange, set_height_function);
}

void Cursor::setDocumentWidth(int width)
{
    m_document_width = width;
}

void Cursor::setDocumentHeight(int height)
{
    m_document_height = height;
}

bool Cursor::visible() const
{
    return m_visible;
}
void Cursor::setVisible(bool visible)
{
    m_new_visibillity = visible;
}

bool Cursor::blinking() const
{
    return m_blinking;
}

void Cursor::setBlinking(bool blinking)
{
    m_new_blinking = blinking;
}

void Cursor::setTextStyle(TextStyle::Style style, bool add)
{
    if (add) {
        m_current_text_style.style |= style;
    } else {
        m_current_text_style.style &= !style;
    }
}

void Cursor::resetStyle()
{
    m_current_text_style.background = ColorPalette::DefaultBackground;
    m_current_text_style.forground = ColorPalette::DefaultForground;
    m_current_text_style.style = TextStyle::Normal;
}

void Cursor::scrollUp(int lines)
{
    if (new_y() < m_top_margin || new_y() > m_bottom_margin)
        return;
    int from = m_scroll_margins_set ? m_top_margin : 0;
    for (int i = 0; i < lines; i++) {
        screen_data()->moveLine(from, new_y());
    }
}

void Cursor::scrollDown(int lines)
{
    if (new_y() < m_top_margin || new_y() > m_bottom_margin)
        return;

    int to = m_scroll_margins_set ? m_bottom_margin : m_document_height - 1;
    for (int i = 0; i < lines; i++) {
        screen_data()->moveLine(new_y(), to);
    }
}

TextStyle Cursor::currentTextStyle() const
{
    return m_current_text_style;
}

void Cursor::setTextStyleColor(ushort color)
{
    Q_ASSERT(color >= 30 && color < 50);
    if (color < 38) {
        m_current_text_style.forground = ColorPalette::Color(color - 30);
    } else if (color == 39) {
        m_current_text_style.forground = ColorPalette::DefaultForground;
    } else if (color >= 40 && color < 48) {
        m_current_text_style.background = ColorPalette::Color(color - 40);
    } else if (color == 49) {
        m_current_text_style.background = ColorPalette::DefaultBackground;
    } else {
        qDebug() << "Failed to set color";
    }
}

ColorPalette *Cursor::colorPalette() const
{
    return m_screen->colorPalette();
}

QPoint Cursor::position() const
{
    return m_position;
}

int Cursor::x() const
{
    return m_position.x();
}

int Cursor::y() const
{
    return m_position.y();
}

QPoint Cursor::rawPosition() const
{
    return m_new_position;
}

void Cursor::moveOrigin()
{
    m_new_position = QPoint(0,0);
    notifyChanged();
}

void Cursor::moveBeginningOfLine()
{
    new_rx() = 0;
    notifyChanged();
}

void Cursor::moveUp(int lines)
{
    if (!new_y() || !lines)
        return;

    if (lines < new_y()) {
        new_ry() -= lines;
    } else {
        new_ry() = 0;
    }
    notifyChanged();
}

void Cursor::moveDown(int lines)
{
    int height = m_screen->height();
    if (new_y() == height -1 || !lines)
        return;

    if (new_y() + lines < height) {
        new_ry() += lines;
    } else {
        new_ry() = height - 1;
    }
    notifyChanged();
}

void Cursor::moveLeft(int positions)
{
    if (!new_x() || !positions)
        return;
    if (positions < new_x()) {
        new_rx() -= positions;
    } else {
        new_rx() = 0;
    }
    notifyChanged();
}

void Cursor::moveRight(int positions)
{
    int width = m_screen->width();
    if (new_x() == width -1 || !positions)
        return;
    if (positions < width - new_x()) {
        new_rx() += positions;
    } else {
        new_rx() = width -1;
    }

    notifyChanged();
}

void Cursor::move(int new_x, int new_y)
{
    int width = m_screen->width();
    int height = m_screen->height();

    if (new_x < 0) {
        new_x = 0;
    } else if (new_x >= width) {
        new_x = width - 1;
    }

    if (new_y < 0) {
        new_y = 0;
    } else if (new_y >= height) {
        new_y = height - 1;
    }

    if (this->new_y() != new_y || this->new_x() != new_x) {
        m_new_position = QPoint(new_x, new_y);
        notifyChanged();
    }
}

void Cursor::moveToLine(int line)
{
    const int height = m_screen->height();
    if (line < 0) {
        line = 0;
    } else if (line >= m_screen->height()) {
        line = height -1;
    }

    if (line != new_y()) {
        new_rx() = line;
        notifyChanged();
    }
}

void Cursor::moveToCharacter(int character)
{
    const int width = m_screen->width();
    if (character < 0) {
        character = 1;
    } else if (character > width) {
        character = width;
    }
    if (character != new_x()) {
        new_rx() = character;
        notifyChanged();
    }
}

void Cursor::clearToBeginningOfLine()
{
    screen_data()->clearToBeginningOfLine(new_y(),new_x());
}

void Cursor::clearToEndOfLine()
{
    screen_data()->clearToEndOfLine(new_y(), new_x());
}

void Cursor::clearToBeginningOfScreen()
{
    clearToBeginningOfLine();
    if (new_y() > 0)
        screen_data()->clearToBeginningOfScreen(new_y() - 1);
}

void Cursor::clearToEndOfScreen()
{
    clearToEndOfLine();
    if (new_y() < m_screen->height() -1)
        screen_data()->clearToEndOfScreen(new_y() + 1);
}

void Cursor::clearLine()
{
    screen_data()->clearLine(new_y());
}

void Cursor::deleteCharacters(int characters)
{
//    switch (m_insert_mode) {
//        case Insert:
//            screen_data()->clearCharacters(new_y(), new_x(), new_x() + characters -1);
//            break;
//        case Replace:
            screen_data()->deleteCharacters(new_y(), new_x(), new_x() + characters -1);
//            break;
//        default:
//            break;
//    }
}

void Cursor::replaceAtCursor(const QString &text)
{
    //if (m_selection_valid ) {
    //    if (current_cursor_new_y() >= m_selection_start.new_y() && current_cursor_new_y() <= m_selection_end.new_y())
    //        //don't need to schedule as event since it will only happen once
    //        setSelectionEnabled(false);
    //}

    if (new_x() + text.size() <= m_screen->width()) {
        Line *line = screen_data()->at(new_y());
        line->replaceAtPos(new_x(), text, m_current_text_style);
        new_rx() += text.size();
    } else {
        for (int i = 0; i < text.size();) {
            if (new_x() == m_screen->width()) {
                new_rx() = 0;
                lineFeed();
            }
            QString toLine = text.mid(i,screen_data()->width() - new_x());
            Line *line = screen_data()->at(new_y());
            line->replaceAtPos(new_x(),toLine, m_current_text_style);
            i+= toLine.size();
            new_rx() += toLine.size();
        }
    }
    notifyChanged();
}

void Cursor::insertAtCursor(const QString &text)
{
    //if (m_selection_valid) {
    //    if (current_cursor_new_y() >= m_selection_start.new_y() && current_cursor_new_y() <= m_selection_end.new_y())
    //        //don't need to schedule as event since it will only happen once
    //        setSelectionEnabled(false);
    //}

    //This is a bug as it does not bounds checking
    Line *line = screen_data()->at(new_y());
    line->insertAtPos(new_x(), text, m_screen->defaultTextStyle());

}

void Cursor::lineFeed()
{
    int bottom = m_scroll_margins_set ? m_bottom_margin : m_document_height - 1;
    if(new_y() == bottom) {
        //m_selection_start.new_ry()--;
        //m_selection_end.new_ry()--;
        //m_selection_moved = true;
        int move_to = m_scroll_margins_set ? m_top_margin : 0;
        screen_data()->moveLine(move_to,new_y());
    } else {
        new_ry()++;
        notifyChanged();
    }
}

void Cursor::reverseLineFeed()
{
    int top = m_scroll_margins_set ? m_top_margin : 0;
    if (new_y() == top) {
        //m_selection_start.new_ry()++;
        //m_selection_end.new_ry()++;
        //m_selection_moved = true;
        int move_from = m_scroll_margins_set ? m_bottom_margin : m_document_height - 1;
        screen_data()->moveLine(move_from, new_y());
    } else {
        new_ry()--;
        notifyChanged();
    }
}

void Cursor::setScrollArea(int from, int to)
{
    m_top_margin = from;
    m_bottom_margin = to;
    m_scroll_margins_set = true;
}

void Cursor::resetScrollArea()
{
    m_top_margin = -1;
    m_bottom_margin = -1;
    m_scroll_margins_set = false;
}

void Cursor::dispatchEvents()
{
    if (m_new_position != m_position) {
        bool emit_x_changed = m_new_position.x() != m_position.x();
        bool emit_y_changed = m_new_position.y() != m_position.y();
        m_position = m_new_position;
        if (emit_x_changed)
            emit xChanged();
        if (emit_y_changed)
            emit yChanged();
    }

    if (m_new_visibillity != m_visible) {
        m_visible = m_new_visibillity;
        emit visibilityChanged();
    }

    if (m_new_blinking != m_blinking) {
        m_blinking = m_new_blinking;
        emit blinkingChanged();
    }
}
