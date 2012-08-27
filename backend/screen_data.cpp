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

#include "screen_data.h"

#include "text_segment_line.h"
#include "screen.h"

#include <stdio.h>

ScreenData::ScreenData(Screen *screen)
    :m_screen(screen)
{
}

int ScreenData::width() const
{
    return m_width;
}

void ScreenData::setWidth(int width)
{
    m_width = width;

    for (int i = 0; i < m_screen_lines.size(); i++) {
        m_screen_lines.at(i)->setWidth(width);
    }
}

int ScreenData::height() const
{
    return m_screen_lines.size();
}

void ScreenData::setHeight(int height)
{
    if (height == m_screen_lines.size())
        return;

    if (m_screen_lines.size() > height) {
        int removeElements = m_screen_lines.size() - height;
        for (int i = 0; i < removeElements; i++) {
            delete m_screen_lines[i];
        }
        m_screen_lines.remove(0, removeElements);
    } else if (m_screen_lines.size() < height){
        int rowsToAdd = height - m_screen_lines.size();
        for (int i = 0; i < rowsToAdd; i++) {
            TextSegmentLine *newLine = new TextSegmentLine(m_screen);
            m_screen_lines.append(newLine);
        }
    }
    if(m_cursor_pos.y() >= m_screen_lines.size())
        m_cursor_pos.setY(m_screen_lines.size()-1);
}

TextSegmentLine *ScreenData::at(int index) const
{
    return m_screen_lines.at(index);
}

QPoint ScreenData::cursorPosition() const
{
    return m_cursor_pos;
}

void ScreenData::clearToEndOfLine(int row, int from_char)
{
    m_screen_lines.at(row)->clearToEndOfLine(from_char);
}

void ScreenData::clearToEndOfScreen(int row)
{
    for(int i = row; i < m_screen_lines.size(); i++) {
        TextSegmentLine *line = m_screen_lines.at(i);
        line->clear();
    }
}

void ScreenData::clearToBeginningOfScreen(int row)
{
    for (int i = row; i >= 0; i--) {
        TextSegmentLine *line = m_screen_lines.at(i);
        line->clear();
    }
}

void ScreenData::clearLine(int index)
{
    m_screen_lines.at(index)->clear();
}

void ScreenData::clear()
{
    for (int i = 0; i < m_screen_lines.size(); i++) {
        TextSegmentLine *line = m_screen_lines.at(i);
        line->clear();
    }
}

QPoint ScreenData::insertText(const QPoint &cursor, const QString &text)
{
    TextSegmentLine *line;
    QPoint new_cursor_pos = cursor;

    if (cursor.x() + text.size() <= width()) {
        line = m_screen_lines.at(cursor.y());
        line->insertAtPos(cursor.x(), text, m_screen->currentTextStyle());
        new_cursor_pos.rx() += text.size();
    } else {
        for (int i = 0; i < text.size();) {
            if (new_cursor_pos.x() == width()) {
                new_cursor_pos.setX(0);
                //dont do linefeed but scroll instead
                m_screen->lineFeed();
            }
            QString toLine = text.mid(i,width() - new_cursor_pos.x());
            line = m_screen_lines.at(new_cursor_pos.y());
            line->insertAtPos(new_cursor_pos.x(),toLine,m_screen->currentTextStyle());
            i+= toLine.size();
            new_cursor_pos.rx() += toLine.size();
        }
    }
    return new_cursor_pos;

}

void ScreenData::scrollOneLineUp(int from_row)
{
    TextSegmentLine *firstLine = m_screen_lines.at(0);
    TextSegmentLine **lines = m_screen_lines.data();
    memmove(lines,lines+1,sizeof(lines) * from_row);
    firstLine->clear();
    m_screen_lines.replace(from_row,firstLine);
}

void ScreenData::printScreen() const
{
    for (int line = 0; line < m_screen_lines.size(); line++) {
        for (int i = 0; i < m_screen_lines.at(line)->size(); i++) {
            fprintf(stderr, "%s", qPrintable(m_screen_lines.at(line)->at(i)->text()));
        }
        fprintf(stderr, "\n");
    }
}
