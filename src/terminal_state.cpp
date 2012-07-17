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

#include "terminal_state.h"

#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtCore/QPoint>

#include <QtCore/QDebug>

#include "tokenizer.h"

TerminalState::TerminalState(QObject *parent)
    : QObject(parent)
    , m_pty(new YatPty(this))
    , m_parser(new Tokenizer)
{
    connect(m_pty, &YatPty::readyRead,
            this, &TerminalState::readData);

    //make sure we have atleast one line
    m_screen_lines << TextSegmentLine();
    m_pty->write("ls /usr/lib\n");

}

TerminalState::~TerminalState()
{
}

void TerminalState::resetState()
{
    m_forground_color = defaultForgroundColor();
    m_background_color = defaultBackgroundColor();
}

QSize TerminalState::size() const
{
    return m_pty->size();
}

void TerminalState::resize(const QSize &size)
{
    if (m_pty->size().height() > size.height() &&
            m_screen_lines.size() > size.height()) {
        int removeElements = m_pty->size().height() - size.height();
        for (int i = 0; i < removeElements; i++)
            m_screen_lines.removeFirst();
    }

    m_pty->resizeTerminal(size);
}

QColor TerminalState::defaultForgroundColor()
{
    return QColor(Qt::white);
}

QColor TerminalState::defaultBackgroundColor()
{
    return QColor(Qt::black);
}

void TerminalState::readData()
{
    QByteArray data = m_pty->read();

    m_parser->addData(data);

    Token token;
    int new_lines = 0;
    foreach(token, m_parser->tokens()) {
        if (token.controllSequence() == TerminalState::NoControllSequence) {
            TextSegment textSegment(token.text(), m_forground_color, m_background_color);
            m_screen_lines[(m_cursor_pos.y())].append(textSegment);
        }

        if (token.controllSequence() == TerminalState::NewLine) {
            new_lines++;
            if (m_cursor_pos.y() == m_screen_lines.size() -1 &&
                    m_cursor_pos.y() == size().height()) {
                m_screen_lines.takeFirst();
                m_screen_lines.append(TextSegmentLine());
            } else {
                m_screen_lines.append(TextSegmentLine());
                m_cursor_pos.setY(m_cursor_pos.y()+1);
            }
            TextSegment textSegment;
            foreach(textSegment, m_screen_lines.at(m_screen_lines.size()-2  )) {
                fprintf(stderr, "%s", qPrintable(textSegment.text()));
            }
            fprintf(stderr, "\n");
        }
    }
    m_parser->clear();
    if (new_lines)
        emit linesAdded(new_lines);
}
