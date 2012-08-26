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

#ifndef SCREENDATA_H
#define SCREENDATA_H

#include <QtCore/QVector>
#include <QtCore/QPoint>

class TextSegmentLine;
class TerminalScreen;

class ScreenData
{
public:
    ScreenData(TerminalScreen *screen);

    int width() const;
    void setWidth(int width);
    int height() const;
    void setHeight(int height);

    TextSegmentLine *at(int index) const;
    QPoint cursorPosition() const;

    void clearToEndOfLine(int row, int from_char);
    void clearToEndOfScreen(int row);
    void clearToBeginningOfScreen(int row);
    void clearLine(int index);
    void clear();

    QPoint insertText(const QPoint &cursor, const QString &text);

    void scrollOneLineUp(int from_row);

    void printScreen() const;
private:
    TerminalScreen *m_screen;
    int m_width;
    QVector<TextSegmentLine *> m_screen_lines;
    QPoint m_cursor_pos;
};

#endif // SCREENDATA_H
