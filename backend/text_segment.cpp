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

#include "text_segment.h"

#include "terminal_screen.h"
#include <QtCore/QDebug>

TextSegment::TextSegment(const QString &text, const TextStyle &style, TerminalScreen *terminalScreen)
    : QObject(terminalScreen)
    , m_text(text)
    , m_style(style)
    , m_screen(terminalScreen)
{
    connect(terminalScreen, &TerminalScreen::dispatchTextSegmentChanges,
            this, &TextSegment::dispatchEvents);
}

TextSegment::TextSegment(TerminalScreen *terminalScreen)
    : QObject(terminalScreen)
    , m_style(terminalScreen->currentTextStyle())
    , m_screen(terminalScreen)
{
    connect(terminalScreen, &TerminalScreen::dispatchTextSegmentChanges,
            this, &TextSegment::dispatchEvents);
}

TextSegment::~TextSegment()
{
}

QString TextSegment::text() const
{
    return m_text;
}

QColor TextSegment::forgroundColor() const
{
    return m_style.forground;
}

QColor TextSegment::backgroundColor() const
{
    return m_style.background;
}

TextSegment *TextSegment::split(int i)
{
    TextSegment *right = 0;
    QString str = m_text.mid(i);
    if (str.size()) {
        right = new TextSegment(str,m_style,m_screen);
        right->setParent(parent());
        m_text.truncate(i);
        m_dirty = true;
    }

    return right;
}

bool TextSegment::isCompatible(const TextStyle &style)
{
    return m_style.forground == style.forground &&
            m_style.background == style.background;
}

int TextSegment::prependText(const QString &text)
{
    int growth = 0;
    if (text.size() > m_text.size()) {
        growth = text.size() - m_text.size();
    }

    if (growth) {
        m_text = text;
    } else {
        m_text = text + m_text.mid(text.size());
    }

    m_dirty = true;
    return growth;
}

int TextSegment::insertText(int index, const QString &text)
{
    int growth = 0;

    if (index == 0) {
        growth = prependText(text);
    } else {
        if (index + text.size() > m_text.size()) {
            growth = text.size() - m_text.size();
        }
        QString left = m_text.left(index);
        QString right = m_text.mid(index);
        if (right.size() <= text.size()) {
            right = text;
        } else {
            right = text + right.mid(text.size());
        }
        m_text = left + right;

        m_dirty = true;
    }
    return growth;
}

void TextSegment::appendText(const QString &text)
{
    m_text.append(text);
    m_dirty = true;
}

void TextSegment::removeCharAtPos(int index)
{
    m_text.remove(index,1);
    m_dirty = true;
}

void TextSegment::removeTextFromBeginning(int n_elements)
{
    m_text = m_text.mid(n_elements);
}

void TextSegment::truncate(int index)
{
    m_text.truncate(index);
    m_dirty = true;
}

void TextSegment::dispatchEvents()
{
    if (m_dirty) {
        m_dirty = false;
        emit textChanged();
    }
}

