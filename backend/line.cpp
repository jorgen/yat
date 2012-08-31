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

#include "line.h"

#include "text.h"
#include "screen.h"

#include <QtCore/QDebug>

Line::Line(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_index(0)
    , m_old_index(-1)
    , m_changed(true)
    , m_reset(true)
{
    m_text_line.resize(screen->width());
    m_style_list.reserve(25);
    m_unused_segments.reserve(25);

    clear();

    connect(screen,&Screen::dispatchLineChanges,
            this, &Line::dispatchEvents);
}

Line::~Line()
{
}

Screen *Line::screen() const
{
    return m_screen;
}

void Line::clear()
{
    m_text_line.fill(QChar(' '));

    m_indexes_to_remove.clear();

    for (int i = 0; i < m_style_list.size(); i++) {
        if (m_style_list.at(i).text_segment)
            m_unused_segments.append(m_style_list.at(i).text_segment);
    }

    m_style_list.clear();
    m_style_list.append(TextStyleLine(m_screen->defaultTextStyle(),0,m_text_line.size() -1));

    m_reset = true;
    m_changed = true;
}

void Line::clearToEndOfLine(int index)
{
    m_changed = true;

    QString empty(m_text_line.size() - index, QChar(' '));
    m_text_line.replace(index, m_text_line.size()-index,empty);
    bool found = false;
    for (int i = 0; i < m_style_list.size(); i++) {
        const TextStyleLine current_style = m_style_list.at(i);
        if (found) {
            if (current_style.old_index >= 0)
                m_indexes_to_remove.append(current_style.old_index);
            m_style_list.remove(i);
            i--;
        } else {
            if (index <= current_style.end_index) {
                found = true;
                if (current_style.start_index == index) {
                    if (current_style.old_index >= 0)
                        m_indexes_to_remove.append(current_style.old_index);
                    m_style_list.remove(i);
                    i--;
                } else {
                    m_style_list[i].changed = true;
                }
            }
        }
    }

    if (m_style_list.size() && m_style_list.last().isCompatible(m_screen->defaultTextStyle())) {
        m_style_list.last().end_index = m_text_line.size() -1;
    } else {
        m_style_list.append(TextStyleLine(m_screen->defaultTextStyle(),index, m_text_line.size() -1));
    }
}

void Line::setWidth(int width)
{
    if (m_text_line.size() > width) {
        m_text_line.chop(m_text_line.size() - width);
    } else if (m_text_line.size() < width) {
        m_text_line.append(QString(width - m_text_line.size(), QChar(' ')));
    }
}

int Line::size() const
{
    return m_style_list.size();
}

Text *Line::at(int i)
{
    if (i < 0 || i >= m_style_list.size())
        return 0;
    if (!m_style_list.at(i).text_segment) {
        m_style_list[i].text_segment = createTextSegment(m_style_list.at(i));
    }

    m_style_list[i].old_index = i;

    return m_style_list.at(i).text_segment;
}

void Line::insertAtPos(int pos, const QString &text, const TextStyle &style)
{
    Q_ASSERT(pos + text.size() <= m_text_line.size());

    m_changed = true;

    m_text_line.replace(pos,text.size(),text);

    bool found = false;
    for (int i = 0;i < m_style_list.size(); i++) {
        const TextStyleLine current_style = m_style_list.at(i);
        if (found) {
            if (current_style.end_index <= pos + text.size()) {
                if (current_style.text_segment)
                    m_unused_segments.append(current_style.text_segment);
                if (current_style.old_index >= 0)
                    m_indexes_to_remove.append(current_style.old_index);
                m_style_list.remove(i);
                i--;
            } else if (current_style.start_index <= pos + text.size()) {
                m_style_list[i].start_index = pos + text.size();
                m_style_list[i].changed = true;
            } else {
                break;
            }
        } else if (pos <= current_style.end_index) {
            found = true;
            if (pos + text.size() <= current_style.end_index) {
                if (current_style.isCompatible(style)) {
                    m_style_list[i].changed = true;
                } else {
                    if (current_style.start_index == pos && current_style.end_index == pos + text.size()) {
                        m_style_list[i].setStyle(style);
                    } else if (current_style.start_index == pos) {
                        m_style_list[i].start_index = pos + text.size();
                        m_style_list.insert(i, TextStyleLine(style,pos, pos+text.size()-1));
                    } else if (current_style.end_index == pos + text.size()) {
                        m_style_list[i].end_index = pos -1;
                        m_style_list.insert(i+1, TextStyleLine(style,pos, pos+text.size() - 1));
                    } else {
                        m_style_list[i].end_index = pos -1;
                        m_style_list.insert(i+1, TextStyleLine(style,pos, pos+text.size() -1 ));
                        m_style_list.insert(i+2, TextStyleLine(current_style,pos + text.size(), current_style.end_index));
                    }
                }
                break;
            } else {
                if (current_style.isCompatible(style)) {
                    m_style_list[i].end_index = pos + text.size() - 1;
                    m_style_list[i].changed = true;
                } else {
                    if (current_style.start_index == pos) {
                        m_style_list[i].end_index = pos + text.size() - 1;
                        m_style_list[i].changed = true;
                        m_style_list[i].style = style.style;
                        m_style_list[i].foreground = style.foreground;
                        m_style_list[i].background = style.background;
                    } else {
                        m_style_list[i].end_index = pos - 1;
                        m_style_list[i].changed = true;
                        m_style_list.insert(i+1, TextStyleLine(style, pos, pos + text.size() -1));
                        i++;
                    }
                }
            }
        }
    }
}

int Line::index() const
{
    return m_index;
}

void Line::setIndex(int index)
{
    m_index = index;
}

static bool lessThanInverse(int x1, int x2)
{
    return x2 < x1;
}

void Line::dispatchEvents()
{
    if (m_index != m_old_index) {
        m_old_index = m_index;
        emit indexChanged();
    }

    if (!m_changed) {
        return;
    }

    if (m_reset) {
        m_reset = false;
        m_changed = false;
        m_indexes_to_remove.clear();
        emit reset();
        return;
    }

    qSort(m_indexes_to_remove.begin(), m_indexes_to_remove.end(), lessThanInverse);
    for (int i = 0; i < m_indexes_to_remove.size(); i++) {
        emit textSegmentRemoved(m_indexes_to_remove.at(i));
    }
    m_indexes_to_remove.clear();

    for (int i = 0; i < m_style_list.size(); i++) {
        const TextStyleLine current_style = m_style_list.at(i);
        if (!current_style.changed)
            continue;

        if (current_style.old_index == -1) {
            emit newTextSegment(i);
        } else if (current_style.changed) {
            m_style_list[i].text_segment->setStringSegment(current_style.start_index, current_style.end_index);
            m_style_list[i].text_segment->setTextStyle(current_style);
        }
    }

    m_changed = false;

    for (int i = 0; i < m_unused_segments.size(); i++) {
        delete m_unused_segments.at(i);
    }
    m_unused_segments.clear();
}

Text *Line::createTextSegment(const TextStyleLine &style_line)
{
    Text *to_return = 0;
    if (m_unused_segments.size()) {
        to_return = m_unused_segments.at(m_unused_segments.size()-1);
        m_unused_segments.remove(m_unused_segments.size() -1);
    } else {
        to_return = new Text(&m_text_line, m_screen);
    }

    to_return->setStringSegment(style_line.start_index, style_line.end_index);
    to_return->setTextStyle(style_line);

    return to_return;
}

