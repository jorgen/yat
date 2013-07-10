/**************************************************************************************************
 * Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
***************************************************************************************************/

#include "terminal_item.h"

TerminalItem::TerminalItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_screen(0)
{
}

void TerminalItem::createScreen(QQuickTextDocument *primary, QQuickTextDocument *secondary)
{
    delete m_screen;
    m_screen = new Screen(primary->textDocument(), secondary->textDocument(),  this);
    connect(m_screen, SIGNAL(flash()), this, SIGNAL(flash()));
    connect(m_screen, SIGNAL(cursorPositionChanged(int, int)), this, SIGNAL(cursorPositionChanged(int,int)));
    connect(this, SIGNAL(heightChanged()), SLOT(handleHeightChanged()));
    connect(this, SIGNAL(widthChanged()), SLOT(handleWidthChanged()));
}

void TerminalItem::sendKey(const QString &text, Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    if (!m_screen)
        return;
    m_screen->sendKey(text, key, modifiers);
}

void TerminalItem::handleHeightChanged()
{
    if (!m_screen)
        return;
    m_screen->setHeight(height());
}

void TerminalItem::handleWidthChanged()
{
    if (!m_screen)
        return;
    m_screen->setWidth(width());
}

