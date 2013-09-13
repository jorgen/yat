/******************************************************************************
* Copyright (c) 2012 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
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

#include "line.h"

#include "text.h"
#include "screen.h"

#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>

#include <QtCore/QDebug>

#include <algorithm>

Line::Line(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_index(0)
    , m_new_index(-1)
    , m_visible(true)
    , m_changed(true)
{
    m_text_line.resize(screen->width());
    m_style_list.reserve(25);

    clear();
}

Line::~Line()
{
    for (int i = 0; i < m_style_list.size(); i++) {
        m_screen->releaseTextSegment(m_style_list.at(i).text_segment);
    }
}

Screen *Line::screen() const
{
    return m_screen;
}

void Line::clear()
{
    m_text_line.fill(QChar(' '));

    for (int i = 0; i < m_style_list.size(); i++) {
        m_screen->releaseTextSegment(m_style_list.at(i).text_segment);
    }

    m_style_list.clear();
    m_style_list.append(TextStyleLine(m_screen->defaultTextStyle(),0,m_text_line.size() -1));

    m_changed = true;
}

void Line::clearCharacters(int from, int to)
{
    QString empty(to-from, QChar(' '));
    const TextStyle &defaultTextStyle = m_screen->defaultTextStyle();
    replaceAtPos(from, empty, defaultTextStyle);
}

void Line::deleteCharacters(int from, int to, int margin)
{
    m_changed = true;

    if (margin < 0)
        margin = m_text_line.size() -1;

    int removed = 0;
    const int size = (std::min(to,margin) + 1) - from;
    bool found = false;

    int last_index = -1;

    for (int i = 0; i < m_style_list.size(); i++) {
        TextStyleLine &current_style = m_style_list[i];
        if (current_style.start_index > margin)
            break;
        last_index = i;
        if (found) {
            current_style.start_index -= removed;
            current_style.end_index -= removed;
            current_style.index_dirty = true;
            if (removed != size) {
                int current_style_size = current_style.end_index + 1  - current_style.start_index;
                if (current_style_size <= size - removed) {
                    removed += current_style.end_index + 1 - current_style.start_index;
                    if (current_style.text_segment)
                        m_screen->releaseTextSegment(current_style.text_segment);
                    m_style_list.remove(i);
                    i--;
                } else {
                    current_style.end_index -= size - removed;
                    removed = size;
                }
            }
        } else {
            if (current_style.start_index <= from && current_style.end_index >= from) {
                found = true;
                int left_in_style = (current_style.end_index + 1) - from;
                int subtract = std::min(left_in_style, size);
                current_style.end_index -= subtract;
                current_style.text_dirty = true;
                removed = subtract;
                if (current_style.end_index < current_style.start_index) {
                    if (current_style.text_segment)
                        m_screen->releaseTextSegment(current_style.text_segment);
                    m_style_list.remove(i);
                    i--;
                }
            }
        }
    }

    if (last_index >= 0) {
        TextStyleLine &last_modified = m_style_list[last_index];
        TextStyle defaultStyle = m_screen->defaultTextStyle();
        if (last_modified.isCompatible(defaultStyle)) {
            last_modified.end_index += size;
            last_modified.text_dirty = true;
        } else {
            m_style_list.insert(last_index + 1, TextStyleLine(defaultStyle,
                        last_modified.end_index + 1, last_modified.end_index + size));
        }
    }

    m_text_line.remove(from, size);
    QString empty(size,' ');
    m_text_line.insert(margin + 1 - empty.size(),empty);
}

void Line::setWidth(int width)
{
    int old_size = m_text_line.size();
    bool emit_changed = old_size != width;
    if (old_size > width) {
        m_text_line.chop(old_size - width);
        for (int i = 0; i < m_style_list.size(); i++) {
            TextStyleLine &style_line = m_style_list[i];
            if (style_line.end_index >= width) {
                if (style_line.start_index > width) {
                    m_screen->releaseTextSegment(style_line.text_segment);
                    m_style_list.remove(i);
                    i--;
                } else {
                    style_line.end_index = width -1;
                    style_line.text_dirty = true;
                }
            }
        }
    } else if (m_text_line.size() < width) {
        m_text_line.append(QString(width - m_text_line.size(), QChar(' ')));
        TextStyleLine &style_line = m_style_list.last();
        if (style_line.isCompatible(m_screen->defaultTextStyle())) {
            style_line.end_index = width - 1;
        } else {
            TextStyleLine new_style_line(m_screen->defaultTextStyle(), style_line.end_index + 1, width - 1);
            m_style_list.append(new_style_line);
        }
    }

    if (emit_changed) {
        m_changed = true;
        emit widthChanged();
    }
}

int Line::width() const
{
    return m_text_line.size();
}

void Line::replaceAtPos(int pos, const QString &text, const TextStyle &style)
{
    Q_ASSERT(pos + text.size() <= m_text_line.size());

    m_changed = true;

    m_text_line.replace(pos,text.size(),text);
    bool found = false;
    for (int i = 0; i < m_style_list.size(); i++) {
        TextStyleLine &current_style = m_style_list[i];
        if (found) {
            if (current_style.end_index <= pos + text.size()) {
                if (current_style.text_segment)
                    m_screen->releaseTextSegment(current_style.text_segment);
                m_style_list.remove(i);
                i--;
            } else if (current_style.start_index <= pos + text.size()) {
                current_style.start_index = pos + text.size();
                current_style.style_dirty = true;
                current_style.text_dirty = true;
                current_style.index_dirty = true;
            } else {
                break;
            }
        } else if (pos >= current_style.start_index && pos <= current_style.end_index) {
            found = true;
            if (pos + text.size() -1 <= current_style.end_index) {
                if (current_style.isCompatible(style)) {
                    current_style.text_dirty = true;
                } else {
                    if (current_style.start_index == pos && current_style.end_index == pos + text.size() - 1) {
                        current_style.setStyle(style);
                        current_style.text_dirty = true;
                        current_style.style_dirty = true;
                    } else if (current_style.start_index == pos) {
                        current_style.start_index = pos + text.size();
                        current_style.text_dirty = true;
                        m_style_list.insert(i, TextStyleLine(style,pos, pos+text.size() -1));
                    } else if (current_style.end_index == pos + text.size()) {
                        current_style.end_index = pos - 1;
                        current_style.text_dirty = true;
                        m_style_list.insert(i+1, TextStyleLine(style,pos, pos+text.size()));
                    } else {
                        int old_end = current_style.end_index;
                        current_style.end_index = pos - 1;
                        current_style.text_dirty = true;
                        m_style_list.insert(i+1, TextStyleLine(style,pos, pos + text.size() - 1));
                        if (pos + text.size() < m_text_line.size()) {
                            m_style_list.insert(i+2, TextStyleLine(current_style,pos + text.size(), old_end));
                        }
                    }
                }
                break;
            } else {
                if (current_style.isCompatible(style)) {
                    current_style.end_index = pos + text.size() - 1;
                    current_style.text_dirty = true;
                } else {
                    if (current_style.start_index == pos) {
                        current_style.end_index = pos + text.size() - 1;
                        current_style.style = style.style;
                        current_style.forground = style.forground;
                        current_style.background = style.background;
                        current_style.text_dirty = true;
                        current_style.style_dirty = true;
                    } else {
                        current_style.end_index = pos - 1;
                        current_style.text_dirty = true;
                        m_style_list.insert(i+1, TextStyleLine(style, pos, pos + text.size() -1));
                        i++;
                    }
                }
            }
        }
    }
}

void Line::insertAtPos(int pos, const QString &text, const TextStyle &style)
{
    m_changed = true;

    m_text_line.insert(pos,text);
    m_text_line.chop(text.size());
    bool found = false;

    for (int i = 0; i < m_style_list.size(); i++) {
        TextStyleLine &current_style = m_style_list[i];
        if (found) {
            current_style.start_index += text.size();
            current_style.end_index += text.size();
            current_style.index_dirty = true;
            if (current_style.start_index >= m_text_line.size()) {
                m_screen->releaseTextSegment(current_style.text_segment);
                m_style_list.remove(i);
                i--;
            } else if (current_style.end_index >= m_text_line.size()) {
                current_style.end_index = m_text_line.size()-1;
            }
        } else if (pos >= current_style.start_index && pos <= current_style.end_index) {
            found = true;
            if (current_style.start_index == pos) {
                current_style.start_index += text.size();
                current_style.end_index += text.size();
                current_style.index_dirty = true;
                m_style_list.insert(i, TextStyleLine(style, pos, pos+ text.size() - 1));
                i++;
            } else if (current_style.end_index == pos) {
                current_style.end_index--;
                current_style.text_dirty = true;
                m_style_list.insert(i+1, TextStyleLine(style, pos, pos+ text.size() - 1));
                i++;
            } else {
                int old_end = current_style.end_index;
                current_style.end_index = pos -1;
                current_style.text_dirty = true;
                m_style_list.insert(i+1, TextStyleLine(style, pos, pos + text.size() - 1));
                if (pos + text.size() < m_text_line.size()) {
                    int segment_end = std::min(m_text_line.size() -1, old_end + text.size());
                    m_style_list.insert(i+2, TextStyleLine(current_style, pos + text.size(), segment_end));
                    i+=2;
                } else {
                    i++;
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
    m_new_index = index;
    for (int i = 0; i < m_style_list.size(); i++) {
        if (m_style_list.at(i).text_segment) {
            m_style_list.at(i).text_segment->setLine(index);
        }
    }
}

QString *Line::textLine()
{
    return &m_text_line;
}

void Line::setVisible(bool visible)
{
    if (visible != m_visible) {
        m_visible = visible;
        emit visibleChanged();
        for (int i = 0; i < m_style_list.size(); i++) {
            if (m_style_list.at(i).text_segment) {
                m_style_list.at(i).text_segment->setVisible(visible);
            }
        }
    }
}

bool Line::visible() const
{
    return m_visible;
}

void Line::dispatchEvents()
{
    if (m_index != m_new_index) {
        m_index = m_new_index;
        emit indexChanged();
    }

    if (!m_changed) {
        return;
    }

    for (int i = 0; i < m_style_list.size(); i++) {
        TextStyleLine &current_style = m_style_list[i];

        if (current_style.text_segment == 0) {
            current_style.text_segment = m_screen->createTextSegment(current_style);
            current_style.text_segment->setLine(m_index);
        }

        if (current_style.style_dirty) {
            current_style.text_segment->setTextStyle(current_style);
            current_style.style_dirty = false;
        }

        if (current_style.index_dirty || current_style.text_dirty) {
            current_style.text_segment->setStringSegment(current_style.start_index, current_style.end_index, current_style.text_dirty);
            current_style.index_dirty = false;
            current_style.text_dirty = false;
        }

        m_style_list.at(i).text_segment->dispatchEvents();
    }

    m_changed = false;

    for (int i = 0; i< m_to_delete.size(); i++) {
        delete m_to_delete[i];
    }
    m_to_delete.clear();
}

QVector<TextStyleLine> Line::style_list()
{
    return m_style_list;
}

void Line::printStyleList() const
{
    QString text_line = m_text_line;
    text_line.remove(QRegExp("\\s+$"));
    qDebug() << "Line: " << this << text_line;
    QDebug debug = qDebug();
    debug << "\t";
    for (int i= 0; i < m_style_list.size(); i++) {
        debug << m_style_list.at(i);
    }
}
