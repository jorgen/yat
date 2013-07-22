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

#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>

#include <QtCore/QDebug>

Line::Line(Screen *screen)
    : QObject(screen)
    , m_screen(screen)
    , m_index(0)
    , m_old_index(-1)
    , m_changed(true)
    , m_item(screen->createLineItem())
{
    m_text_line.resize(screen->width());
    m_style_list.reserve(25);
    m_unused_segments.reserve(25);

    clear();

    m_item->setProperty("parent", QVariant::fromValue(m_screen->parent()));
    m_item->setProperty("height", m_screen->lineHeight());
    m_item->setProperty("textLine", QVariant::fromValue(this));
}

Line::~Line()
{
    releaseTextObjects();

    if (m_item)
        m_screen->destroyLineItem(m_item);
}

Screen *Line::screen() const
{
    return m_screen;
}

void Line::releaseTextObjects()
{
    m_changed = true;
    for (int i = 0; i < m_style_list.size(); i++) {
        if (m_style_list.at(i).text_segment) {
            m_style_list.at(i).text_segment->setVisible(false);
            screen()->releaseText(m_style_list.at(i).text_segment);
            m_style_list[i].text_segment = 0;
            m_style_list[i].changed = true;
        }
    }
}

void Line::clear()
{
    m_text_line.fill(QChar(' '));

    for (int i = 0; i < m_style_list.size(); i++) {
        if (m_style_list.at(i).text_segment)
            releaseTextSegment(m_style_list.at(i).text_segment);
    }

    m_style_list.clear();
    m_style_list.append(TextStyleLine(m_screen->defaultTextStyle(),0,m_text_line.size() -1));

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
            if (current_style.text_segment)
                releaseTextSegment(current_style.text_segment);
            m_style_list.remove(i);
            i--;
        } else {
            if (index <= current_style.end_index) {
                found = true;
                if (current_style.start_index == index) {
                    if (current_style.text_segment)
                        releaseTextSegment(current_style.text_segment);
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

void Line::clearCharacters(int from, int to)
{
    QString empty(to-from, QChar(' '));
    const TextStyle &defaultTextStyle = m_screen->defaultTextStyle();
    insertAtPos(from, empty, defaultTextStyle);
}

void Line::deleteCharacters(int from, int to)
{
    int removed = 0;
    const int size = to - from;
    bool found = false;

    for (int i = 0; i < m_style_list.size(); i++) {
        TextStyleLine &current_style = m_style_list[i];
        if (found) {
            current_style.start_index -= removed;
            current_style.end_index -= removed;
            if (removed != size) {
                int current_style_size = current_style.end_index - current_style.start_index;
                if (current_style_size < size - removed) {
                    removed += current_style.end_index - current_style.start_index;
                    if (current_style.text_segment)
                        releaseTextSegment(current_style.text_segment);
                    m_style_list.remove(i);
                    i--;
                } else {
                    current_style.end_index -= size - removed;
                    removed = size;
                }
            }
        } else {
            if (current_style.start_index >= from) {
                found = true;
                if (current_style.end_index <= to) {
                    removed += current_style.end_index - current_style.start_index;
                    if (current_style.text_segment)
                        releaseTextSegment(current_style.text_segment);
                    m_style_list.remove(i);
                    i--;
                } else {
                    current_style.end_index = to;
                    removed = size;
                }
            }
        }
    }

    m_text_line.remove(from, to-from);
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

void Line::insertAtPos(int pos, const QString &text, const TextStyle &style)
{
    Q_ASSERT(pos + text.size() <= m_text_line.size());

    m_changed = true;

    m_text_line.replace(pos,text.size(),text);

    bool found = false;
    for (int i = 0; i < m_style_list.size(); i++) {
        const TextStyleLine &current_style = m_style_list.at(i);
        if (found) {
            if (current_style.end_index <= pos + text.size()) {
                if (current_style.text_segment)
                    releaseTextSegment(current_style.text_segment);
                m_style_list.remove(i);
                i--;
            } else if (current_style.start_index <= pos + text.size()) {
                m_style_list[i].start_index = pos + text.size();
                m_style_list[i].changed = true;
            } else {
                break;
            }
        } else if (pos >= current_style.start_index && pos <= current_style.end_index) {
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

QString *Line::textLine()
{
    return &m_text_line;
}

QObject *Line::item() const
{
    return m_item;
}

void Line::setVisible(bool visible)
{
    m_item->setProperty("visible", visible);
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

    for (int i = 0; i < m_style_list.size(); i++) {
        const TextStyleLine current_style = m_style_list.at(i);
        if (!current_style.changed)
            continue;


        if (current_style.changed) {
            if (current_style.text_segment == 0) {
                m_style_list[i].text_segment = createTextSegment(current_style);
            }
            m_style_list[i].text_segment->setStringSegment(current_style.start_index, current_style.end_index);
            m_style_list[i].text_segment->setTextStyle(current_style);
            m_style_list[i].changed = false;

            m_style_list.at(i).text_segment->dispatchEvents();
        }
    }

    m_changed = false;
}

void Line::printStyleElements() const
{
    for (int i = 0; i < m_style_list.size(); i++) {
        if (i != 0)
            fprintf(stderr, ",");
        fprintf(stderr, "[%d : %d]", m_style_list.at(i).start_index, m_style_list.at(i).end_index);
    }
}

QVector<TextStyleLine> Line::style_list()
{
    return m_style_list;
}
Text *Line::createTextSegment(const TextStyleLine &style_line)
{
    Text *to_return = screen()->createText();
    to_return->setLine(this);
    to_return->setStringSegment(style_line.start_index, style_line.end_index);
    to_return->setTextStyle(style_line);
    to_return->setVisible(true);

    return to_return;
}

void Line::releaseTextSegment(Text *text)
{
    screen()->releaseText(text);
    text->setVisible(false);
}

