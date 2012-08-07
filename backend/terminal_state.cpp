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
#include <QtCore/QElapsedTimer>

#include "tokenizer.h"

TerminalState::TerminalState()
    : QObject(0)
    , m_forground_color(Qt::black)
    , m_background_color(Qt::black)
    , m_pty(new YatPty(this))
    , m_screen(new TerminalScreen(this))
    , m_parser(new Tokenizer)
{
    connect(m_pty, &YatPty::readyRead,
            this, &TerminalState::readData);

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

void TerminalState::write(const QString &data)
{
    m_pty->write(data.toUtf8());
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
        switch(token.controllSequence()) {
        case TerminalState::NoControllSequence:
            m_screen->insertAtCursor(token.text(), m_forground_color, m_background_color);
            break;
        case TerminalState::NewLine:
            m_screen->newLine();
            break;
        case TerminalState::CursorHome:
            m_screen->moveCursorHome();
            break;
        case TerminalState::EraseLine:
            m_screen->eraseLine();
            break;
        case TerminalState::HorizontalTab: {
            int x = m_screen->cursorPosition().x();
            int spaces = 8 - (x % 8);
            m_screen->insertAtCursor(QString(spaces,' '),m_forground_color,m_background_color);
        }
            break;
        default:
            break;
        }

    }
    m_screen->dispatchChanges();
    m_parser->clear();
}
