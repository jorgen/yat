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

#ifndef TERMINALSTATE_H
#define TERMINALSTATE_H

#include <QtCore/QObject>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtGui/QColor>

#include "terminal_screen.h"
#include "yat_pty.h"

class Tokenizer;

class TerminalState : public QObject
{
    Q_OBJECT


public:

    enum ControllSequence {
        NoControllSequence,
        UnknownControllSequence,
        NewLine,
        HorizontalTab,
        Backspace,
        QueryDeviceCode,
        ReportDeviceCode,
        QueryDeviceStatus,
        ReportDeviceOK,
        ReportDeviceFailure,
        QueryCursorPosition,
        ReportCursorPosistion,
        ResetDevice,
        EnableLineWrap,
        DisableLineWrap,
        FontSetG0,
        FontSetG1,
        CursorHome,
        CursorUp,
        CursorDown,
        CursorForward,
        CursorBackward,
        ForceCursorPosition,
        SaveCursor,
        UnsaveCursor,
        SaveCursorAndAttrs,
        RestoreCursorAndAttrs,
        ScrollScreen,
        ScrollDown,
        ScrollUp,
        SetTab,
        ClearTab,
        ClearAllTabs,
        EraseEndOfLine,
        EraseStartofLine,
        EraseOnLine,
        EraseDown,
        EraseUp,
        EraseScreen,
        PrintScreen,
        PrintLine,
        StopPrintLog,
        StartPrintLog,
        SetKeyDefinition,
        SetAttributeMode,
        ChangeWindowAndIconName,
        ChangeIconTitle,
        ChangeWindowTitle
    };

    TerminalState();
    ~TerminalState();

    void resetState();

    Q_INVOKABLE int width() const;
    Q_INVOKABLE void setWidth(int width);

    Q_INVOKABLE int height() const;
    Q_INVOKABLE void setHeight(int height);

    Q_INVOKABLE void resize(const QSize &size);
    Q_INVOKABLE QSize size() const;

    TerminalScreen *screen() const;

public slots:
    void write(const QString &data);

private:
    void readData();

    QColor m_forground_color;
    QColor m_background_color;
    YatPty *m_pty;
    TerminalScreen *m_screen;

    Tokenizer *m_parser;
};

#endif // TERMINALSTATE_H
