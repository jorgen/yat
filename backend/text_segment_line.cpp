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

#include "text_segment_line.h"

#include "text_segment.h"
#include "terminal_screen.h"

#include <QtCore/QDebug>

TextSegmentLine::TextSegmentLine(TerminalScreen *terminalScreen)
    : QObject(terminalScreen)
    , m_screen(terminalScreen)
{
    connect(terminalScreen,&TerminalScreen::dispatchLineChanges,
            this, &TextSegmentLine::dispatchEvents);
}

TextSegmentLine::~TextSegmentLine()
{
}

void TextSegmentLine::clear()
{
    for (int i = 0; i < m_segments.size(); i++) {
        delete m_segments.at(i);
    }
    m_segments.clear();
    m_update_actions.clear();
    UpdateAction action(UpdateAction::Reset,0);
    m_update_actions.append(action);
}

int TextSegmentLine::size() const
{
    return m_segments.size();
}

TextSegment *TextSegmentLine::at(int i) const
{
    return m_segments.at(i);
}

QList<TextSegment *> TextSegmentLine::segments() const
{
    return m_segments;
}

void TextSegmentLine::append(TextSegment *segment)
{
    if (m_segments.size() && m_segments.last()->isCompatible(segment)) {
        m_segments.last()->appendTextSegment(segment);
        delete segment;
    } else {
        segment->setParent(this);
        m_segments.append(segment);
        if (m_update_actions.size() == 0 ||
                m_update_actions.last().action != UpdateAction::Reset) {
            UpdateAction action(UpdateAction::NewText, m_segments.size()-1);
            m_update_actions.append(action);
        }
    }
}

void TextSegmentLine::prepend(TextSegment *segment)
{
    if (m_segments.size() && m_segments.first()->isCompatible(segment)) {
        m_segments.first()->prependTextSegment(segment);
        delete segment;
    } else {
        segment->setParent(this);
        m_segments.prepend(segment);
        if (m_update_actions.size() == 0 ||
                m_update_actions.last().action != UpdateAction::Reset) {
            for (int i = 0; i < m_update_actions.size(); i++) {
                m_update_actions[i].index++;
            }
            m_update_actions << UpdateAction(UpdateAction::NewText,0);
        }
    }
}

void TextSegmentLine::insertAtPos(int pos, TextSegment *segment)
{
    if (m_segments.size() == 0 || pos == 0) {
        prepend(segment);
        return;
    }

    int char_count = 0;
    for (int i = 0; i < m_segments.size(); i++) {
        TextSegment *left = m_segments.at(i);
        if (pos < char_count + left->text().size()) {
            int split_position = pos - char_count;
            if (left->isCompatible(segment)) {
                left->insertTextSegment(split_position, segment);
                delete segment;
            } else {
                TextSegment *right = left->split(split_position);
                m_segments.insert(i+1,segment);
                m_segments.insert(i+1,right);
                if (m_update_actions.size() == 0 ||
                        m_update_actions.last().action != UpdateAction::Reset) {
                    for (int update_index = 0; update_index < m_update_actions.size(); update_index++) {
                        if (m_update_actions.at(update_index).index >= i)
                            m_update_actions[update_index].index++;
                    }
                    m_update_actions << UpdateAction(UpdateAction::NewText, i);
                }
            }
            return;
        }
        char_count += m_segments.at(i)->text().size();
    }
    append(segment);
}


void TextSegmentLine::dispatchEvents()
{
    UpdateAction action;
    foreach(action, m_update_actions) {
        switch(action.action) {
        case UpdateAction::Reset:
            qDebug() << "Emitting reset";
            emit reset();
            break;
        case UpdateAction::NewText:
            qDebug() << "New Text reset";
            emit newTextSegment(action.index);
            break;
        case UpdateAction::RemovedText:
            qDebug() << "Removed Text reset";
            emit textSegmentRemoved(action.index);
            break;
        default:
            qDebug() << "Unknown update action";
            break;
        }
    }
    m_update_actions.clear();
}
