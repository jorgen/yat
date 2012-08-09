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

#include <QtCore/QElapsedTimer>

#include <QtCore/QDebug>

TerminalScreen::TerminalScreen(QObject *parent)
    : QObject(parent)
    , m_current_text_style(TextStyle(TextStyle::Normal,Qt::white))
{
    connect(&m_pty, &YatPty::readyRead,
            this, &TerminalScreen::readData);

    m_font.setPixelSize(14);
    m_font.setFamily(QStringLiteral("Courier"));

    setWidth(80);
    setHeight(25);
}

TerminalScreen::~TerminalScreen()
{
}

QColor TerminalScreen::defaultForgroundColor()
{
    return QColor(Qt::white);
}

QColor TerminalScreen::defaultBackgroundColor()
{
    return QColor(Qt::black);
}

void TerminalScreen::setHeight(int height)
{
    if (m_screen_lines.size() > height) {
        int removeElements = m_screen_lines.size() - height;
        for (int i = 0; i < removeElements; i++) {
            delete m_screen_lines[i];
        }
        m_screen_lines.remove(0, removeElements);
        emit linesRemoved(removeElements);
    } else if (m_screen_lines.size() < height){
        int rowsToAdd = height - m_screen_lines.size();
        for (int i = 0; i < rowsToAdd; i++) {
            TextSegmentLine *newLine = new TextSegmentLine(this);
            m_screen_lines.append(newLine);
        }
        emit linesInserted(rowsToAdd);
    }
    if(m_cursor_pos.y() >= m_screen_lines.size())
        m_cursor_pos.setY(m_screen_lines.size()-1);

    m_pty.setHeight(height);
}

void TerminalScreen::setWidth(int width)
{
    m_pty.setWidth(width);
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

void TerminalScreen::resetStyle()
{
    m_current_text_style.background = defaultBackgroundColor();
    m_current_text_style.forground = defaultForgroundColor();
    m_current_text_style.style = TextStyle::Normal;
}

TextStyle TerminalScreen::currentTextStyle() const
{
    return m_current_text_style;
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

void TerminalScreen::insertAtCursor(const QString &text)
{
    TextSegmentLine *line = line_at_cursor();
    m_cursor_pos.setX(m_cursor_pos.x() + text.size());
    line->insertAtPos(m_cursor_pos.x(), text, m_current_text_style);
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

void TerminalScreen::write(const QString &data)
{
    m_pty.write(data.toUtf8());
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

void TerminalScreen::readData()
{
    for (int i = 0; i < 20; i++) {
        QByteArray data = m_pty.read();

        m_parser.addData(data);

        Token token;
        foreach(token, m_parser.tokens()) {
            switch(token.controllSequence()) {
            case Token::NoControllSequence:
                insertAtCursor(token.text());
                break;
            case Token::NewLine:
                newLine();
                break;
            case Token::CursorHome:
                moveCursorHome();
                break;
            case Token::EraseOnLine:
               if (!token.parameters().size() || token.parameters().at(0) == 0) {
                   eraseFromPresentationPositionToEndOfLine();
                } else {
                    eraseLine();
                }
                break;
            case Token::HorizontalTab: {
                int x = cursorPosition().x();
                int spaces = 8 - (x % 8);
                insertAtCursor(QString(spaces,' '));
            }
            case Token::Backspace:
                backspace();
                break;
            case Token::SetAttributeMode:
                if (token.parameters().size()) {
                    switch(token.parameters().at(0)) {
                    case 0:
                        resetStyle();
                        break;
                    case 1:

                    case 39:
                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                        break;
                    }
                } else {
                    resetStyle();
                }

                break;
            default:
                break;
            }
        }

        m_parser.clearTokensList();

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
