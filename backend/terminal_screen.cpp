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

TerminalScreen::TerminalScreen(QObject *parent)
    : QObject(parent)
{
    m_font.setPixelSize(10);
    m_font.setFamily(QStringLiteral("Monospace"));
}

QSize TerminalScreen::size() const
{
    return QSize(100,m_screen_lines.size());
}

QColor TerminalScreen::defaultForgroundColor()
{
    return QColor(Qt::white);
}

QColor TerminalScreen::defaultBackgroundColor()
{
    return QColor(Qt::black);
}


void TerminalScreen::resize(const QSize &size)
{
    bool emitGeometryChanged = m_screen_lines.size() != size.height();

    if (m_screen_lines.size() > size.height()) {
        int removeElements = m_screen_lines.size() - size.height();
        for (int i = 0; i < removeElements; i++) {
            delete m_screen_lines.at(m_screen_lines.size() - i - 1);
            m_screen_lines.remove(m_screen_lines.size() - i -1);
        }
    } else if (m_screen_lines.size() < size.height()){
        int rowsToAdd = size.height() - m_screen_lines.size();
        for (int i = 0; i < rowsToAdd; i++) {
            m_screen_lines.insert(0, new TextSegmentLine(this));
        }
    }
    if(m_cursor_pos.y() >= m_screen_lines.size())
        m_cursor_pos.setY(m_screen_lines.size()-1);

    if (emitGeometryChanged) {
        emit geometryChanged();
        emit heightChanged();
    }
}

int TerminalScreen::height() const
{
    return m_screen_lines.size();
}

QFont TerminalScreen::font() const
{
    return m_font;
}

void TerminalScreen::setFont(const QFont &font)
{
    m_font = font;
    emit fontChanged();
}

QPoint TerminalScreen::cursorPosition() const
{
    return m_cursor_pos;
}

void TerminalScreen::insertAtCursor(TextSegment *segment)
{
    TextSegmentLine *line = m_screen_lines.at(m_cursor_pos.y());
    line->insertAtPos(m_cursor_pos.x(), segment);
}

void TerminalScreen::newLine()
{
    delete m_screen_lines.at(m_screen_lines.size() -1);
    TextSegmentLine **lines = m_screen_lines.data() + m_cursor_pos.y();
    int rows_to_move = m_screen_lines.size() - m_cursor_pos.y() -1;
    memmove(lines+1,lines,sizeof(lines) * rows_to_move);
    m_screen_lines.replace(m_cursor_pos.y(), new TextSegmentLine());
    emit scrollUp(m_cursor_pos.y());
}

TextSegmentLine *TerminalScreen::at(int i) const
{
    return m_screen_lines.at(i);
}


