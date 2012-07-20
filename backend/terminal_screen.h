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

#ifndef TERMINALSCREEN_H
#define TERMINALSCREEN_H

#include <QObject>

#include "text_segment.h"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QVector>
#include <QtGui/QFont>

class TerminalScreen : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QSize geometry READ size WRITE resize NOTIFY geometryChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)

public:
    explicit TerminalScreen(QObject *parent = 0);
    
    QSize size() const;
    Q_INVOKABLE void resize(const QSize &size);
    int height() const;

    QFont font() const;
    void setFont(const QFont &font);

    QColor defaultForgroundColor();
    QColor defaultBackgroundColor();

    QPoint cursorPosition() const;

    void insertAtCursor(TextSegment *segment);

    void newLine();

    Q_INVOKABLE TextSegmentLine *at(int i) const;

signals:
    void geometryChanged();
    void heightChanged();
    void fontChanged();
    void scrollUp(int count);
    void scrollDown(int count);

private:
    QVector<TextSegmentLine *> m_screen_lines;
    QPoint m_cursor_pos;
    QFont m_font;
};

#endif // TERMINALSCREEN_H
