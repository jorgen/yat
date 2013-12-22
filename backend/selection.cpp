/*******************************************************************************
* Copyright (c) 2013 Jørgen Lind
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
*******************************************************************************/

#include "selection.h"

#include "screen.h"
#include "screen_data.h"

#include <QtGui/QGuiApplication>

Selection::Selection(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_start_x(0)
    , m_start_y(0)
    , m_end_x(0)
    , m_end_y(0)
    , m_enable(false)
{

}

void Selection::setStartX(int x)
{
    if (x != m_start_x) {
        m_start_x = x;
        setValidity();
        emit startXChanged();
    }
}

int Selection::startX() const
{
    return m_start_x;
}

void Selection::setStartY(int y)
{
    if (y != m_start_y) {
        m_start_y = y;
        setValidity();
        emit startYChanged();
    }
}

int Selection::startY() const
{
    return m_start_y;
}

void Selection::setEndX(int x)
{
    if (m_end_x != x) {
        m_end_x = x;
        setValidity();
        emit endXChanged();
    }
}

int Selection::endX() const
{
    return m_end_x;
}

void Selection::setEndY(int y)
{
    if (m_end_y != y) {
        m_end_y = y;
        setValidity();
        emit endYChanged();
    }
}

int Selection::endY() const
{
    return m_end_y;
}

void Selection::setEnable(bool enable)
{
    if (m_enable != enable) {
        m_enable = enable;
        emit enableChanged();
    }
}

void Selection::setValidity()
{
    if (m_end_y > m_start_y ||
            (m_end_y == m_start_y &&
             m_end_x > m_start_x)) {
        setEnable(true);
    } else {
        setEnable(false);
    }
}

void Selection::sendToClipboard() const
{
    m_screen->currentScreenData()->sendSelectionToClipboard(start_point(), end_point(), QClipboard::Clipboard);
}

void Selection::sendToSelection() const
{
    m_screen->currentScreenData()->sendSelectionToClipboard(start_point(), end_point(), QClipboard::Selection);
}

void Selection::pasteFromSelection()
{
    m_screen->pty()->write(QGuiApplication::clipboard()->text(QClipboard::Selection).toUtf8());
}

void Selection::pasteFromClipboard()
{
    m_screen->pty()->write(QGuiApplication::clipboard()->text(QClipboard::Clipboard).toUtf8());
}

bool Selection::enable() const
{
    return m_enable;
}
