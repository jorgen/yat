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

#ifndef TEXT_SEGMENT_LINE_H
#define TEXT_SEGMENT_LINE_H

#include <QtCore/QObject>

#include "text_segment.h"

class TerminalScreen;

class TextStyleLine : public TextStyle {
public:
    TextStyleLine(const TextStyle &style, int start_index, int end_index)
        : TextStyle(style.style,style.forground)
        , start_index(start_index)
        , end_index(end_index)
        , old_index(-1)
        , text_segment(0)
        , changed(true)
    {
        background = style.background;
    }

    int start_index;
    int end_index;

    int old_index;
    TextSegment *text_segment;
    bool changed;

    void setStyle(const TextStyle &style) {
        forground = style.forground;
        background = style.background;
        this->style = style.style;
    }
};

class TextSegmentLine : public QObject
{
    Q_OBJECT
public:
    TextSegmentLine(TerminalScreen *terminalScreen);
    ~TextSegmentLine();

    void clear();
    void clearToEndOfLine(int index);
    void setWidth(int width);

    Q_INVOKABLE int size() const;
    Q_INVOKABLE TextSegment *at(int i);

    void insertAtPos(int i, const QString &text, const TextStyle &style);

signals:
    void newTextSegment(int index, int data_index);
    void textSegmentRemoved(int index);

    void reset();

private:
    void dispatchEvents();

    TextSegment *createTextSegment(const TextStyleLine &style_line);

    TerminalScreen *m_screen;
    QString m_text_line;
    QList<TextStyleLine> m_style_list;
    QList<int> m_indexes_to_remove;

    QList<TextSegment *> m_unused_segments;
    bool m_changed;
    bool m_reset;
};

#endif // TEXT_SEGMENT_LINE_H
