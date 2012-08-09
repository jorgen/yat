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

#include <QtCore/QDebug>

TerminalScreen::TerminalScreen(QObject *parent)
    : QObject(parent)
{
    m_font.setPixelSize(14);
    m_font.setFamily(QStringLiteral("Courier"));
}

TerminalScreen::~TerminalScreen()
{
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
    if (m_screen_lines.size() > size.height()) {
        int removeElements = m_screen_lines.size() - size.height();
        for (int i = 0; i < removeElements; i++) {
            delete m_screen_lines[i];
        }
        m_screen_lines.remove(0, removeElements);
        emit linesRemoved(removeElements);
    } else if (m_screen_lines.size() < size.height()){
        int rowsToAdd = size.height() - m_screen_lines.size();
        for (int i = 0; i < rowsToAdd; i++) {
            TextSegmentLine *newLine = new TextSegmentLine(this);
            m_screen_lines.append(newLine);
        }
        emit linesInserted(rowsToAdd);
    }
    if(m_cursor_pos.y() >= m_screen_lines.size())
        m_cursor_pos.setY(m_screen_lines.size()-1);

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

void TerminalScreen::moveCursorHome()
{
    m_cursor_pos.setX(0);
}

void TerminalScreen::moveCursorUp()
{
    m_cursor_pos.ry() -= 1;
}

void TerminalScreen::moveCursorDown()
{
    m_cursor_pos.ry() += 1;
}

void TerminalScreen::moveCursorLeft()
{
    m_cursor_pos.rx() += 1;
}

void TerminalScreen::moveCursorRight()
{
    m_cursor_pos.rx() -= 1;
}

void TerminalScreen::insertAtCursor(const QString &text, const QColor &bg, const QColor &fg)
{
    TextSegment *segment = new TextSegment(text,bg,fg, this);
    TextSegmentLine *line = line_at_cursor();
    m_cursor_pos.setX(m_cursor_pos.x() + segment->text().size());
    line->insertAtPos(m_cursor_pos.x(), segment);
}

void TerminalScreen::backspace()
{
    if (m_cursor_pos.x() > 0)
        m_cursor_pos.rx()--;
}

void TerminalScreen::eraseLine()
{
    TextSegmentLine *line = line_at_cursor();
    line->clear();
}

void TerminalScreen::eraseFromPresentationPositionToEndOfLine()
{
    int active_presentation_pos = m_cursor_pos.x();
    TextSegmentLine *line = line_at_cursor();
    line->removeCharFromPos(active_presentation_pos);
}

void TerminalScreen::newLine()
{
    if(m_cursor_pos.y() == m_screen_lines.size() -1) {
        TextSegmentLine *firstLine = m_screen_lines.at(0);
        TextSegmentLine **lines = m_screen_lines.data();
        int rows_to_move = m_cursor_pos.y();
        memmove(lines,lines+1,sizeof(lines) * rows_to_move);
        firstLine->clear();
        m_screen_lines.replace(m_cursor_pos.y(),firstLine);
        doScrollOneLineUpAt(m_cursor_pos.y());
    } else {
        moveCursorDown();
        moveCursorHome();
    }
}

TextSegmentLine *TerminalScreen::at(int i) const
{
    return m_screen_lines.at(i);
}

void TerminalScreen::printScreen() const
{
    for (int line = 0; line < m_screen_lines.size(); line++) {
        for (int i = 0; i < m_screen_lines.at(line)->size(); i++) {
            fprintf(stderr, "%s", qPrintable(m_screen_lines.at(line)->at(i)->text()));
        }
        fprintf(stderr, "\n");
    }
}


TextSegmentLine *TerminalScreen::line_at_cursor()
{
    return m_screen_lines.at(m_cursor_pos.y());
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

    m_update_actions.clear();

    emit dispatchLineChanges();
    emit dispatchTextSegmentChanges();
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
