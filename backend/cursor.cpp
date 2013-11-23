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

#include "screen_data.h"
#include "scrollback.h"

#include <QTextCodec>

Cursor::Cursor(Screen* screen)
    : QObject(screen)
    , m_screen(screen)
    , m_screen_data(m_screen->currentScreenData())
    , m_current_text_style(screen->defaultTextStyle())
    , m_position(m_screen_data)
    , m_width(screen->width())
    , m_height(screen->height())
    , m_top_margin(0)
    , m_bottom_margin(0)
    , m_scroll_margins_set(false)
    , m_origin_at_margin(false)
    , m_notified(false)
    , m_visible(true)
    , m_new_visibillity(true)
    , m_blinking(false)
    , m_new_blinking(false)
    , m_wrap_around(true)
    , m_content_height_changed(false)
    , m_insert_mode(Replace)
{
    connect(screen, SIGNAL(widthAboutToChange(int)), this, SLOT(setDocumentWidth(int)));
    connect(screen, SIGNAL(heightAboutToChange(size_t , int, int)), this, SLOT(setDocumentHeight(size_t, int, int)));
    connect(screen, SIGNAL(contentHeightChanged()), this, SLOT(contentHeightChanged()));

    m_gl_text_codec = QTextCodec::codecForName("utf-8")->makeDecoder();
    m_gr_text_codec = QTextCodec::codecForName("utf-8")->makeDecoder();

    for (int i = 0; i < m_width; i++) {
        if (i % 8 == 0) {
            m_tab_stops.append(i);
        }
    }
}

Cursor::~Cursor()
{
}

void Cursor::setDocumentWidth(int width)
{
    if (width > m_width) {
        for (int i = m_width -1; i < width; i++) {
            if (i % 8 == 0) {
                m_tab_stops.append(i);
            }
        }
    }

    m_width = width;
    if (new_x() >= width) {
        m_position.setX(width - 1);
        notifyChanged();
    }
}

void Cursor::setDocumentHeight(size_t height, int currentCursorBlock, int currentScrollBackHeight)
{
    Q_UNUSED(currentCursorBlock);
    resetScrollArea();
    if (m_height > height) {
        const size_t to_remove = m_height - height;
        const int removeLinesBelowCursor =
            std::min(m_height - new_y(), to_remove);
        const int removeLinesAtTop = to_remove - removeLinesBelowCursor;
        if (!removeLinesAtTop) {
            m_position.linesAddedToEnd(removeLinesAtTop);
            notifyChanged();
        }
    } else {
        int height_diff = height - m_height;
        if (currentScrollBackHeight >= height_diff) {
            m_position.linesAddedTop(height_diff);
        } else if (currentScrollBackHeight > 0) {
            const int added_top = height_diff - currentScrollBackHeight;
            m_position.linesAddedTop(added_top);
        }
    }

    m_height = height;
    notifyChanged();
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
    if (m_scroll_margins_set) {
        if (new_y() < m_top_margin || new_y() > m_bottom_margin)
            return;
        for (int i = 0; i < lines; i++) {
            screen_data()->moveLine(m_bottom_margin, m_top_margin);
        }
    } else {
        for (int i = 0; i < lines; i++) {
            screen_data()->moveLine(m_height -1, 0);
        }
    }
}

void Cursor::scrollDown(int lines)
{
    if (m_scroll_margins_set) {
        if (new_y() < m_top_margin || new_y() > m_bottom_margin)
            return;
        for (int i = 0; i < lines; i++) {
            screen_data()->moveLine(m_top_margin, m_bottom_margin);
        }
    } else {
        for (int i = 0; i < lines; i++) {
            screen_data()->moveLine(0,m_height - 1);
        }
    }
}

void Cursor::setTextCodec(QTextCodec *codec)
{
    m_gl_text_codec = codec->makeDecoder();
}

void Cursor::setInsertMode(InsertMode mode)
{
    m_insert_mode = mode;
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

//QPoint Cursor::position() const
//{
//    return m_position;
//}

int Cursor::x() const
{
    return m_position.x();
}

int Cursor::y() const
{
    return (m_screen->currentScreenData()->contentHeight() - m_screen->height()) + m_position.y();
}

void Cursor::moveOrigin()
{
    m_position.setPos(0,adjusted_top());
    notifyChanged();
}

void Cursor::moveBeginningOfLine()
{
    m_position.setX(0);
    notifyChanged();
}

void Cursor::moveUp(int lines)
{
    int adjusted_new_y = this->adjusted_new_y();
    if (!lines)
        return;

    if (lines < adjusted_new_y) {
        m_position.moveUp(lines);
    } else if (new_y() != adjusted_top()) {
        m_position.setY(adjusted_top());
    }
    notifyChanged();
}

void Cursor::moveDown(int lines)
{
    int bottom = adjusted_bottom();
    if (new_y() == bottom || !lines)
        return;

    if (new_y() + lines <= bottom) {
        m_position.moveDown(lines);
    } else if (new_y() != bottom) {
        m_position.setY(bottom);
    }
    notifyChanged();
}

void Cursor::moveLeft(int positions)
{
    if (!new_x() || !positions)
        return;
    if (positions < new_x()) {
        m_position.setX(m_position.x() - positions);
    } else {
        m_position.setX(0);
    }
    notifyChanged();
}

void Cursor::moveRight(int positions)
{
    int width = m_screen->width();
    if (new_x() == width -1 || !positions)
        return;
    if (positions < width - new_x()) {
        m_position.setX(m_position.x() + positions);
    } else {
        m_position.setX(width -1);
    }

    notifyChanged();
}

void Cursor::move(int new_x, int new_y)
{
    int width = m_screen->width();

    if (m_origin_at_margin) {
        new_y += m_top_margin;
    }

    if (new_x < 0) {
        new_x = 0;
    } else if (new_x >= width) {
        new_x = width - 1;
    }

    if (new_y < adjusted_top()) {
        new_y = adjusted_top();
    } else if (new_y > adjusted_bottom()) {
        new_y = adjusted_bottom();
    }

    if (this->new_y() != new_y || this->new_x() != new_x) {
        m_position.setPos(new_x, new_y);
    }
}

void Cursor::moveToLine(int line)
{
    const int height = m_screen->height();
    if (line < adjusted_top()) {
        line = 0;
    } else if (line > adjusted_bottom()) {
        line = height -1;
    }

    if (line != new_y()) {
        m_position.setY(line);
    }

    notifyChanged();
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
        m_position.setX(character);
        notifyChanged();
    }
}

void Cursor::moveToNextTab()
{
    for (int i = 0; i < m_tab_stops.size(); i++) {
        if (new_x() < m_tab_stops.at(i)) {
            moveToCharacter(std::min(m_tab_stops.at(i), m_width -1));
            return;
        }
    }
    moveToCharacter(m_width - 1);
}

void Cursor::setTabStop()
{
    int i;
    for (i = 0; i < m_tab_stops.size(); i++) {
        if (new_x() == m_tab_stops.at(i))
            return;
        if (new_x() > m_tab_stops.at(i)) {
            continue;
        } else {
            break;
        }
    }
    m_tab_stops.insert(i,new_x());
}

void Cursor::removeTabStop()
{
    for (int i = 0; i < m_tab_stops.size(); i++) {
        if (new_x() == m_tab_stops.at(i)) {
            m_tab_stops.remove(i);
            return;
        } else if (new_x() < m_tab_stops.at(i)) {
            return;
        }
    }
}

void Cursor::clearTabStops()
{
    m_tab_stops.clear();
}

void Cursor::clearToBeginningOfLine()
{
    screen_data()->clearToBeginningOfLine(this);
}

void Cursor::clearToEndOfLine()
{
    screen_data()->clearToEndOfLine(this);
}

void Cursor::clearToBeginningOfScreen()
{
    clearToBeginningOfLine();
    if (new_y() > 0)
        screen_data()->clearToBeginningOfScreen(this);
}

void Cursor::clearToEndOfScreen()
{
    clearToEndOfLine();
    if (size_t(new_y()) < m_screen->height() -1)
        screen_data()->clearToEndOfScreen(this);
}

void Cursor::clearLine()
{
    screen_data()->clearLine(this);
}

void Cursor::deleteCharacters(int characters)
{
    screen_data()->deleteCharacters(this, new_x() + characters -1);
}

void Cursor::setWrapAround(bool wrap)
{
    m_wrap_around = wrap;
}

void Cursor::addAtCursor(const QByteArray &data)
{
    if (m_insert_mode == Replace) {
        replaceAtCursor(data);
    } else {
        insertAtCursor(data);
    }
}

void Cursor::replaceAtCursor(const QByteArray &data)
{
    m_position.assert_cursor_position();
    const QString text = m_gl_text_codec->toUnicode(data);
    size_t y_after_insert = new_y();
    int x_after_insert = new_x();
    if (!m_wrap_around && new_x() + text.size() > m_screen->width()) {
        const int size = m_width - new_x();
        QString toBlock = text.mid(0,size);
        toBlock.replace(toBlock.size() - 1, 1, text.at(text.size()-1));
        screen_data()->replace(this, toBlock, m_current_text_style);
        x_after_insert += toBlock.size();
    } else {
        auto diff = screen_data()->replace(this, text, m_current_text_style);
        x_after_insert += diff.character;
        y_after_insert += diff.line;
    }

    if (y_after_insert >= m_height)
        y_after_insert = m_height - 1;
    if (x_after_insert >= m_width)
        x_after_insert = m_width - 1;

    m_position.setPos(x_after_insert, y_after_insert);

    m_position.assert_cursor_position();


    if (m_screen_data->m_height > m_screen_data->screen()->height()) {
        auto end_it = m_position.m_it;
        ++end_it;
        Q_ASSERT(end_it == m_screen_data->m_screen_blocks.end());
    }
    notifyChanged();
}

void Cursor::insertAtCursor(const QByteArray &data)
{
    size_t y_after_insert = new_y();
    int x_after_insert = new_x();
    const QString text = m_gl_text_codec->toUnicode(data);
    auto diff = screen_data()->insert(this, text, m_current_text_style);
    x_after_insert += diff.character;
    y_after_insert += diff.line;
    if (y_after_insert >= m_height)
        y_after_insert = m_height - 1;
    if (x_after_insert >= m_width)
        x_after_insert = m_width - 1;

    m_position.setPos(x_after_insert, y_after_insert);
}

void Cursor::lineFeed()
{
    int bottom = m_scroll_margins_set ? m_bottom_margin : m_height - 1;
    if(new_y() == bottom) {
        screen_data()->insertLine(this);
        m_position.linesAddedToEnd(1);
    }

    m_position.moveDown(1);
    if (m_screen_data->m_height > m_screen_data->screen()->height()) {
        auto end_it = m_position.m_it;
        ++end_it;
        if (end_it != m_screen_data->m_screen_blocks.end()) {
        }
        Q_ASSERT(end_it == m_screen_data->m_screen_blocks.end());
    }
    notifyChanged();
}

void Cursor::reverseLineFeed()
{
    int top = m_scroll_margins_set ? m_top_margin : 0;
    if (new_y() == top) {
        scrollUp(1);
    }
    m_position.moveUp(1);
}

void Cursor::setOriginAtMargin(bool atMargin)
{
    m_origin_at_margin = atMargin;
    m_position.setY(adjusted_top());
    notifyChanged();
}

void Cursor::setScrollArea(size_t from, size_t to)
{
    m_top_margin = from;
    m_bottom_margin = std::min(to,m_height -1);
    m_scroll_margins_set = true;
}

void Cursor::resetScrollArea()
{
    m_top_margin = 0;
    m_bottom_margin = 0;
    m_scroll_margins_set = false;
}

void Cursor::dispatchEvents()
{
    if (x() != new_x() || y() != new_y() || m_content_height_changed) {
        bool emit_x_changed = m_position.x() != m_old_position.x();
        bool emit_y_changed = m_position.y() != m_old_position.y();
        m_old_position = m_position.point();
        if (emit_x_changed)
            emit xChanged();
        if (emit_y_changed || m_content_height_changed)
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

std::list<Block *>::iterator Cursor::it()
{
    return m_position.it();
}

void Cursor::contentHeightChanged()
{
    m_content_height_changed = true;
}

void Cursor::setScreenData(ScreenData *data)
{
    m_screen_data = data;
    m_position = Position(data);
}

