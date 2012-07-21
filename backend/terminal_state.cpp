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

TerminalState::TerminalState()
    : QObject(0)
    , m_pty(new YatPty(this))
    , m_screen(new TerminalScreen(this))
    , m_parser(new Tokenizer)
{
    connect(m_pty, &YatPty::readyRead,
            this, &TerminalState::readData);

    m_pty->write("ls /usr/lib\n");
    m_screen->resize(m_pty->size());
}

TerminalState::~TerminalState()
{
    delete m_parser;
    delete m_screen;
    delete m_pty;
}

void TerminalState::resetState()
{
    m_forground_color = m_screen->defaultForgroundColor();
    m_background_color = m_screen->defaultBackgroundColor();
}

int TerminalState::width() const
{
    return size().width();
}

void TerminalState::setWidth(int width)
{
    resize(QSize(width,size().height()));
}

int TerminalState::height() const
{
    return size().height();
}

void TerminalState::setHeight(int height)
{
    resize(QSize(size().width(), height));
}

QSize TerminalState::size() const
{
    return m_pty->size();
}

TerminalScreen *TerminalState::screen() const
{
    return m_screen;
}

void TerminalState::resize(const QSize &size)
{
    m_screen->resize(size);
    m_pty->resizeTerminal(size);
}


void TerminalState::readData()
{
    QByteArray data = m_pty->read();

    m_parser->addData(data);

    Token token;
    foreach(token, m_parser->tokens()) {
        if (token.controllSequence() == TerminalState::NoControllSequence) {
            TextSegment *textSegment = new TextSegment(token.text(), m_forground_color, m_background_color);
            m_screen->insertAtCursor(textSegment);
        }

        if (token.controllSequence() == TerminalState::NewLine) {
            m_screen->newLine();
        }
    }
    m_parser->clear();
}
