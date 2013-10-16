/*******************************************************************************
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
*******************************************************************************/

#ifndef BLOCK_H
#define BLOCK_H

#include "text_style.h"

class Text;
class Screen;

class Block
{
public:
    Block(Screen *screen);
    ~Block();

    Screen *screen() const;

    void clear();
    void clearToEnd(int from);
    void clearCharacters(int from, int to);
    void deleteCharacters(int from, int to);

    void replaceAtPos(int i, const QString &text, const TextStyle &style);
    void insertAtPos(int i, const QString &text, const TextStyle &style);

    void setLine(size_t line);
    void incLine(size_t inc = 1);
    void decIndec(size_t dec = 1);
    size_t line() const;

    const QString *textLine() const;
    int textSize() const;
    int lines() const;
    int width() const;
    void setWidth(int width);

    void setVisible(bool visible);
    bool visible() const;

    void dispatchEvents();
    void releaseTextObjects();

    QVector<TextStyleLine> style_list();

    void printStyleList() const;
    void printStyleList(QDebug &debug) const;
    void printRuler() const;
    void printRuler(QDebug &debug) const;

private:
    void mergeCompatibleStyles();
    Screen *m_screen;
    QString m_text_line;
    QVector<TextStyleLine> m_style_list;
    size_t m_line;
    size_t m_new_line;

    int m_width;

    bool m_visible;
    bool m_changed;

};

#endif // BLOCK_H
