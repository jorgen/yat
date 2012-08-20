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
    if (i < m_segments.size())
        return m_segments.at(i);
    return 0;
}

QList<TextSegment *> TextSegmentLine::segments() const
{
    return m_segments;
}

void TextSegmentLine::append(const QString &text, const TextStyle &style)
{
    if (m_segments.size() && m_segments.last()->isCompatible(style)) {
        m_segments.last()->appendText(text);
    } else {
        TextSegment *textSegment = new TextSegment(text,style,m_screen);
        textSegment->setParent(this);
        m_segments.append(textSegment);
        addNewTextAction(m_segments.size()-1);
    }
}

void TextSegmentLine::prepend(const QString &text, const TextStyle &style)
{
    if (m_segments.size() && m_segments.first()->isCompatible(style)) {
        int chars_to_remove = m_segments.first()->prependText(text);
        if (m_segments.size() > 1)
            removeChars(1, chars_to_remove);
    } else {
        TextSegment *textSegment = new TextSegment(text,style,m_screen);
        textSegment->setParent(this);
        m_segments.prepend(textSegment);
        addNewTextAction(0);
        if (m_segments.size() > 1)
            removeChars(1, text.size());
    }
}

void TextSegmentLine::insertAtPos(int pos, const QString &text, const TextStyle &style)
{
    if (pos == 0) {
        prepend(text, style);
        return;
    }

    if (m_segments.size() == 0) {
        QByteArray spaces(pos -1, ' ');
        QString text_to_prepend = text;
        text_to_prepend.prepend(spaces);
        prepend(text_to_prepend,style);
        return;
    }

    int char_count;
    int index_for_segment;
    if (findSegmentIndexForChar(pos, &index_for_segment,&char_count) < 0) {
        QString text_to_append = text;
        if (char_count < pos) {
            Q_ASSERT(pos - char_count > 0);
            QByteArray spaces(pos - char_count, ' ');
            text_to_append.prepend(spaces);
        }
        append(text_to_append,style);
        return;
    }

    TextSegment *left = m_segments.at(index_for_segment);
    if (pos < char_count + left->text().size()) {
        int split_position = pos - char_count;
        if (left->isCompatible(style)) {
            int additional = left->insertText(split_position, text);
            removeChars(index_for_segment+1,additional);
        } else {
            TextSegment *right = left->split(split_position);
            TextSegment *segment = new TextSegment(text,style,m_screen);
            int insert_segment_index = index_for_segment +1;
            m_segments.insert(insert_segment_index,segment);
            addNewTextAction(insert_segment_index);
            insert_segment_index++;
            if (right) {
                m_segments.insert(insert_segment_index,right);
                addNewTextAction(insert_segment_index);
            }

            removeChars(insert_segment_index, text.size());


            addNewTextAction(index_for_segment);
        }
    }
//    int total = 0;
//    for (int i = 0; i < m_segments.size(); i++) {
//        qDebug() << "segmentsize:" << m_segments.at(i)->text().size();
//        total+= m_segments.at(i)->text().size();
//    }
//    qDebug() << "total" << total << ":" << m_screen->width();
//    if (total > m_screen->width())
//        qDebug() << "screenwidth to big";

}

void TextSegmentLine::removeCharAtPos(int pos)
{
    int char_count;
    int index_for_segment;
    if (findSegmentIndexForChar(pos, &index_for_segment,&char_count) < 0)
        return;

    TextSegment *text_segment = m_segments.at(index_for_segment);
    if (pos < char_count + text_segment->text().size()) {
        if (text_segment->text().size() == 1) {
            delete m_segments.takeAt(index_for_segment);
            addRemoveTextAction(index_for_segment);
        } else {
            int char_index_in_segment = pos - char_count;
            text_segment->removeCharAtPos(char_index_in_segment);
        }
    }
}

void TextSegmentLine::removeChars(int char_index)
{
    int char_count;
    int index_for_segment;
    if (findSegmentIndexForChar(char_index, &index_for_segment,&char_count) < 0)
        return;
    int index_in_segment = char_index - char_count;
    TextSegment *toTruncate = m_segments.at(index_for_segment);
    toTruncate->truncate(index_in_segment);
    if (m_segments.size() -1 > index_for_segment) {
        int segments_to_remove = m_segments.size() - (index_for_segment + 1);
        for (int i = 0; i < segments_to_remove; i++) {
            delete m_segments.takeLast();
            addRemoveTextAction(m_segments.size()-1);
        }
    }
}

void TextSegmentLine::removeChars(int from_index, int n_chars)
{
    while (n_chars > 0 && from_index < m_segments.size()) {
        TextSegment *segment = m_segments.at(from_index);
        if (segment->text().size() <= n_chars) {
            n_chars -= segment->text().size();
            m_segments.removeAt(from_index);
            delete segment;
            addRemoveTextAction(from_index);
        } else {
            segment->removeTextFromBeginning(n_chars);
            n_chars = 0;
        }
    }
}

void TextSegmentLine::dispatchEvents()
{
    for(int i = 0; i < m_update_actions.size(); i++) {
        const UpdateAction action = m_update_actions.at(i);
        switch(action.action) {
        case UpdateAction::Reset:
            emit reset();
            break;
        case UpdateAction::NewText:
            emit newTextSegment(action.index, action.data_index);
            break;
        case UpdateAction::RemovedText:
            emit textSegmentRemoved(action.index);
            break;
        default:
            break;
        }
    }
    m_update_actions.clear();
}

int TextSegmentLine::findSegmentIndexForChar(int pos, int *index, int *chars_before_index)
{
    int char_count = 0;

    for (int i = 0; i < m_segments.size(); i++) {
        if (pos < char_count + m_segments.at(i)->text().size()) {
            *index = i;
            *chars_before_index = char_count;
            return 0;
        }

        char_count += m_segments.at(i)->text().size();
    }
    *chars_before_index = char_count;
    return -1;
}

void TextSegmentLine::addNewTextAction(int index)
{
    if (m_update_actions.size() == 0 ||
            m_update_actions.last().action != UpdateAction::Reset) {
        for (int i = 0; i < m_update_actions.size(); i++) {
            if (m_update_actions.at(i).data_index >= index) {
                m_update_actions[i].data_index++;
            }
        }

        m_update_actions.append(UpdateAction(UpdateAction::NewText, index));
    }
}

void TextSegmentLine::addRemoveTextAction(int index)
{
    if (m_update_actions.size() == 0 ||
            m_update_actions.last().action != UpdateAction::Reset) {
        for (int i = 0; i < m_update_actions.size(); i++) {
            if (m_update_actions.at(i).data_index > index) {
                m_update_actions[i].data_index--;
            }
        }
        m_update_actions.append(UpdateAction(UpdateAction::RemovedText, index));
    }
}

void TextSegmentLine::addResetAction()
{
    m_update_actions.clear();
    m_update_actions << UpdateAction(UpdateAction::Reset,0);
}
