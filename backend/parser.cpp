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

#include "parser.h"

#include "controll_chars.h"
#include "terminal_screen.h"

#include <QtCore/QDebug>

Parser::Parser(TerminalScreen *screen)
    : m_decode_state(PlainText)
    , m_current_token_start(0)
    , m_currrent_position(0)
    , m_intermediate_char(QChar())
    , m_screen(screen)
{
}

void Parser::addData(const QByteArray &data)
{
    m_current_token_start = 0;
    m_current_data = data;
    for (m_currrent_position = 0; m_currrent_position < data.size(); m_currrent_position++) {
        uchar character = data.at(m_currrent_position);
        switch (m_decode_state) {
        case PlainText:
            if (character < C0::C0_END ||
                    (character >= C1_8bit::C1_8bit_Start &&
                     character <= C1_8bit::C1_8bit_Stop)) {
                if (m_currrent_position != m_current_token_start) {
                    m_screen->insertAtCursor(QString::fromUtf8(data.mid(m_current_token_start,
                                                                        m_currrent_position - m_current_token_start)));
                    tokenFinished();
                    m_current_token_start--;
                }
                m_decode_state = DecodeC0;
                decodeC0(data.at(m_currrent_position));
            }
            break;
        case DecodeC0:
            decodeC0(character);
            break;
        case DecodeC1_7bit:
            decodeC1_7bit(character);
            break;
        case DecodeCSI:
            decodeCSI(character);
            break;
        case DecodeOSC:
            decodeOSC(character);
            break;
       }
    }
    if (m_decode_state == PlainText) {
        m_screen->insertAtCursor(QString::fromUtf8(data.mid(m_current_token_start)));
        tokenFinished();
    }
    m_current_data = QByteArray();
}

void Parser::decodeC0(uchar character)
{
    switch (character) {
    case C0::NUL:
    case C0::SOH:
    case C0::STX:
    case C0::ETX:
    case C0::EOT:
    case C0::ENQ:
    case C0::ACK:
    case C0::BEL:
        qDebug() << "Unhandled Controll character" << character;
        tokenFinished();
        break;

    case C0::BS:
        m_screen->backspace();
        tokenFinished();
        break;
    case C0::HT: {
        int x = m_screen->cursorPosition().x();
        int spaces = 8 - (x % 8);
        m_screen->insertAtCursor(QString(spaces,' '));
    }
        tokenFinished();
        break;
    case C0::LF:
        m_screen->newLine();
        tokenFinished();
        break;
    case C0::VT:
    case C0::FF:
        qDebug() << "Unhandled Controll character" << character;
        break;
    case C0::CR:
        m_screen->moveCursorHome();
        tokenFinished();
        //next should be a linefeed;
        break;
    case C0::SOorLS1:
    case C0::SIorLS0:
    case C0::DLE:
    case C0::DC1:
    case C0::DC2:
    case C0::DC3:
    case C0::DC4:
    case C0::NAK:
    case C0::SYN:
    case C0::ETB:
    case C0::CAN:
    case C0::EM:
    case C0::SUB:
        qDebug() << "Unhandled Controll character" << character;
        break;
    case C0::ESC:
        m_decode_state = DecodeC1_7bit;
        break;
    case C0::IS4:
    case C0::IS3:
    case C0::IS2:
    case C0::IS1:
    default:
        qDebug() << "Unhandled Controll character" << character;
    }
}

void Parser::decodeC1_7bit(uchar character)
{
    switch(character) {
    case C1_7bit::CSI:
        m_decode_state = DecodeCSI;
        break;
    case C1_7bit::OSC:
        m_decode_state = DecodeOSC;
        break;
    default:
        qDebug() << "Unhandled C1_7bit character" << character;
    }
}

void Parser::decodeParameters(uchar character)
{
    switch (character) {
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
        m_parameter_string.append(character);
        break;
    case 0x3a:
        qDebug() << "Encountered special delimiter in parameterbyte";
        break;
    case 0x3b:
        m_parameters.append(m_parameter_string.toUShort());
        m_parameter_string.clear();
        break;
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    default:
        //this is undefined for now
        qDebug() << "Encountered undefined parameter byte";
        break;
    }
}

void Parser::decodeCSI(uchar character)
{
        if (character >= 0x30 && character <= 0x3f) {
            decodeParameters(character);
        } else {
            if (character >= 0x20 && character <= 0x2f) {
                if (m_intermediate_char.unicode())
                    qDebug() << "Warning!: double intermediate bytes found in CSI";
                m_intermediate_char = character;
            } else if (character >= 0x40 && character <= 0x7d) {
                if (m_intermediate_char.unicode()) {
                    switch (character) {
                    case FinalBytesSingleIntermediate::SL:
                    case FinalBytesSingleIntermediate::SR:
                    case FinalBytesSingleIntermediate::GSM:
                    case FinalBytesSingleIntermediate::GSS:
                    case FinalBytesSingleIntermediate::FNT:
                    case FinalBytesSingleIntermediate::TSS:
                    case FinalBytesSingleIntermediate::JFY:
                    case FinalBytesSingleIntermediate::SPI:
                    case FinalBytesSingleIntermediate::QUAD:
                    case FinalBytesSingleIntermediate::SSU:
                    case FinalBytesSingleIntermediate::PFS:
                    case FinalBytesSingleIntermediate::SHS:
                    case FinalBytesSingleIntermediate::SVS:
                    case FinalBytesSingleIntermediate::IGS:
                    case FinalBytesSingleIntermediate::IDCS:
                    case FinalBytesSingleIntermediate::PPA:
                    case FinalBytesSingleIntermediate::PPR:
                    case FinalBytesSingleIntermediate::PPB:
                    case FinalBytesSingleIntermediate::SPD:
                    case FinalBytesSingleIntermediate::DTA:
                    case FinalBytesSingleIntermediate::SHL:
                    case FinalBytesSingleIntermediate::SLL:
                    case FinalBytesSingleIntermediate::FNK:
                    case FinalBytesSingleIntermediate::SPQR:
                    case FinalBytesSingleIntermediate::SEF:
                    case FinalBytesSingleIntermediate::PEC:
                    case FinalBytesSingleIntermediate::SSW:
                    case FinalBytesSingleIntermediate::SACS:
                    case FinalBytesSingleIntermediate::SAPV:
                    case FinalBytesSingleIntermediate::STAB:
                    case FinalBytesSingleIntermediate::GCC:
                    case FinalBytesSingleIntermediate::TATE:
                    case FinalBytesSingleIntermediate::TALE:
                    case FinalBytesSingleIntermediate::TAC:
                    case FinalBytesSingleIntermediate::TCC:
                    case FinalBytesSingleIntermediate::TSR:
                    case FinalBytesSingleIntermediate::SCO:
                    case FinalBytesSingleIntermediate::SRCS:
                    case FinalBytesSingleIntermediate::SCS:
                    case FinalBytesSingleIntermediate::SLS:
                    case FinalBytesSingleIntermediate::SCP:
                    default:
                        qDebug() << "unhandled CSI sequence";
                        tokenFinished();
                        break;
                    }
                } else {
                    switch (character) {
                    case FinalBytesNoIntermediate::ICH:
                    case FinalBytesNoIntermediate::CUU:
                    case FinalBytesNoIntermediate::CUD:
                    case FinalBytesNoIntermediate::CUF:
                    case FinalBytesNoIntermediate::CUB:
                    case FinalBytesNoIntermediate::CNL:
                    case FinalBytesNoIntermediate::CPL:
                    case FinalBytesNoIntermediate::CHA:
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::CUP:
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());
                        qDebug() << "Setting cursor position";
                        if (!m_parameters.size()) {
                            m_screen->moveCursorHome();
                            m_screen->moveCursorTop();
                        } else if (m_parameters.size() == 2){
                            m_screen->moveCursor(m_parameters.at(0), m_parameters.at(1));
                        }
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::CHT:
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::ED:
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());
                        if (!m_parameters.size()) {
                            m_screen->eraseFromCurrentLineToEndOfScreen();
                        } else {
                            switch (m_parameters.at(0)) {
                            case 1:
                                m_screen->eraseFromCurrentLineToBeginningOfScreen();
                                break;
                            case 2:
                                m_screen->eraseScreen();
                                break;
                            default:
                                qDebug() << "Invalid parameter value for FinalBytesNoIntermediate::ED";
                            }
                        }

                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::EL:
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());
                        if (!m_parameters.size() || m_parameters.at(0) == 0) {
                            m_screen->eraseFromCursorPositionToEndOfLine();
                         } else {
                            m_screen->eraseLine();
                         }
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::IL:
                    case FinalBytesNoIntermediate::DL:
                    case FinalBytesNoIntermediate::EF:
                    case FinalBytesNoIntermediate::EA:
                    case FinalBytesNoIntermediate::DCH:
                    case FinalBytesNoIntermediate::SSE:
                    case FinalBytesNoIntermediate::CPR:
                    case FinalBytesNoIntermediate::SU:
                    case FinalBytesNoIntermediate::SD:
                    case FinalBytesNoIntermediate::NP:
                    case FinalBytesNoIntermediate::PP:
                    case FinalBytesNoIntermediate::CTC:
                    case FinalBytesNoIntermediate::ECH:
                    case FinalBytesNoIntermediate::CVT:
                    case FinalBytesNoIntermediate::CBT:
                    case FinalBytesNoIntermediate::SRS:
                    case FinalBytesNoIntermediate::PTX:
                    case FinalBytesNoIntermediate::SDS:
                    case FinalBytesNoIntermediate::SIMD:
                    case FinalBytesNoIntermediate::HPA:
                    case FinalBytesNoIntermediate::HPR:
                    case FinalBytesNoIntermediate::REP:
                    case FinalBytesNoIntermediate::DA:
                    case FinalBytesNoIntermediate::VPA:
                    case FinalBytesNoIntermediate::VPR:
                    case FinalBytesNoIntermediate::HVP:
                    case FinalBytesNoIntermediate::TBC:
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::SM:
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());
                        qDebug() << "SET MODE!!!!";
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::MC:
                    case FinalBytesNoIntermediate::HPB:
                    case FinalBytesNoIntermediate::VPB:
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::RM:
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());

                        qDebug() << "Resetting mode";
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::SGR: {
                        if (!m_parameter_string.isEmpty())
                            m_parameters.append(m_parameter_string.toUShort());
                        if (m_parameters.size()) {
                            switch(m_parameters.at(0)) {
                            case 0:
                                m_screen->resetStyle();
                                break;
                            case 1:
                                if (m_parameters.size() > 1)
                                    m_screen->setColor(true, m_parameters.at(1));
                                break;
                            case 2:
                                if (m_parameters.size() > 1)
                                    m_screen->setColor(false, m_parameters.at(1));
                            }
                        } else {
                            m_screen->resetStyle();
                        }
                        tokenFinished();
                    }
                        break;
                    case FinalBytesNoIntermediate::DSR:
                    case FinalBytesNoIntermediate::DAQ:
                    default:
                        qDebug() << "Unhandeled CSI squence\n";
                        tokenFinished();
                        break;
                    }
                }
            }
        }
}

void Parser::decodeOSC(uchar character)
{
        if (!m_parameters.size() &&
                character >= 0x30 && character <= 0x3f) {
            decodeParameters(character);
        } else {
            if (m_decode_osc_state ==  None) {
                if (m_parameter_string.size()) {
                    m_parameters.append(m_parameter_string.toUShort());
                    m_parameter_string.clear();
                }
                Q_ASSERT(m_parameters.size() == 1);
                switch (m_parameters.at(0)) {
                case 0:
                    m_decode_osc_state = ChangeWindowAndIconName;
                    break;
                case 1:
                    m_decode_osc_state = ChangeIconTitle;
                    break;
                case 2:
                    m_decode_osc_state = ChangeWindowTitle;
                    break;
                }
            } else {
                if (character == 0x07) {
                    if (m_decode_osc_state == ChangeWindowAndIconName ||
                        m_decode_osc_state == ChangeWindowTitle) {
                        QString title = QString::fromUtf8(m_current_data.mid(m_current_token_start+4,
                                                                             m_currrent_position - m_current_token_start -1));
                        m_screen->setTitle(title);
                    }
                    tokenFinished();
                }
            }
        }
}

void Parser::tokenFinished()
{
    m_decode_state = PlainText;
    m_decode_osc_state = None;

    m_parameters.clear();
    m_parameter_string.clear();

    m_current_token_start = m_currrent_position + 1;
    m_intermediate_char = 0;
}

