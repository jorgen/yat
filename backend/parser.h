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

#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QLinkedList>

#include "text_segment.h"

class Token
{
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
    Token();
    void appendText(const QByteArray &string);
    bool isEmpty() const
    {
        return m_text.isEmpty() &&
                m_controll_sequense == Token::NoControllSequence &&
                m_parameters.size() == 0;
    }
    bool isText() const { return !m_text.isEmpty(); }
    QString &textR() { return m_text; }
    QString text() const { return m_text; }

    void addParameter(ushort parameter);
    QVector<ushort> parameters() const { return m_parameters; }

    Token::ControllSequence controllSequence() const { return m_controll_sequense; }
    void setControllSequence(Token::ControllSequence controll_sequence);
private:
    QString m_text;
    Token::ControllSequence m_controll_sequense;
    QVector<ushort> m_parameters;
};

class Parser
{
public:
    Parser(TerminalScreen *screen);

    void addData(const QByteArray &data);

    QLinkedList<Token> tokens() const { return m_tokens; }
    void clearTokensList() { m_tokens.clear(); }
private:

    enum DecodeState {
        PlainText,
        DecodeC0,
        DecodeC1_7bit,
        DecodeCSI,
        DecodeOSC
    };

    void decodeC0(uchar character);
    void decodeC1_7bit(uchar character);
    void decodeParameters(uchar character);
    void decodeCSI(uchar character);
    void decodeOSC(uchar character);
    void tokenFinished();

    DecodeState m_decode_state;
    Token m_current_token;
    int m_current_token_start;
    int m_currrent_position;
    QChar m_intermediate_char;
    bool m_has_osc_parameter;
    QByteArray m_current_data;

    QString m_parameter_string;
    QLinkedList<Token> m_tokens;

    TerminalScreen *m_screen;
};

#endif // PARSER_H
