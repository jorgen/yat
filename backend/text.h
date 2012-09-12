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

#ifndef TEXT_SEGMENT_H
#define TEXT_SEGMENT_H

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtCore/QObject>
#include <QtCore/QSize>

#include "text_style.h"

class Screen;
class Line;
class QQuickItem;

class Text : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int index READ index NOTIFY indexChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor NOTIFY forgroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(Screen *screen READ screen CONSTANT)
    Q_PROPERTY(QObject *item READ item CONSTANT)
public:
    Text(QString *text_line, Line *line);
    ~Text();

    int index() const;

    QString text() const;

    QColor foregroundColor() const;

    QColor backgroundColor() const;

    void setStringSegment(int start_index, int end_index);
    void setTextStyle(const TextStyle &style);

    Screen *screen() const;

    QObject *item() const;

public slots:
    void dispatchEvents();

signals:
    void aboutToDestroy();
    void textChanged();
    void styleChanged();
    void indexChanged();
    void forgroundColorChanged();
    void backgroundColorChanged();

private:

    QString m_text;
    QString *m_text_line;
    int m_start_index;
    int m_old_start_index;
    int m_end_index;
    TextStyle m_style;

    bool m_style_dirty;
    bool m_text_dirty;
    bool m_initial;
    Line *m_line;

    QObject *m_item;
};

#endif // TEXT_SEGMENT_H
