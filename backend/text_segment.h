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

class TerminalScreen;

class TextSegment : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
public:
    TextSegment(const QString &text, const QColor &forground, const QColor &background, TerminalScreen *terminalScreen);
    TextSegment(TerminalScreen *terminalScreen);
    ~TextSegment();

    QString text() const;

    Q_INVOKABLE QColor forgroundColor() const;
    void  setForgroundColor(const QColor &color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    TextSegment *split(int i);
    bool isCompatible(TextSegment *other);
    void prependTextSegment(TextSegment *other);
    void insertTextSegment(int index, TextSegment *other);
    void appendTextSegment(TextSegment *other);

    void removeCharAtPos(int index);
    void truncate(int index);

signals:
    void textChanged();

private:
    void dispatchEvents();

    QString m_text;
    QColor m_forground_color;
    QColor m_background_color;
    bool m_dirty;
    TerminalScreen *m_screen;

};

#endif // TEXT_SEGMENT_H
