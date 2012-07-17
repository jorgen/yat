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

#include "tokenizer.h"

#include "controll_chars.h"

#include <QtCore/QDebug>

Token::Token()
    : m_controll_sequense(TerminalState::NoControllSequence)
    , m_parameters(0)
{
}

void Token::appendText(const QByteArray &string)
{
    m_text.append(QString::fromUtf8(string));
}

void Token::addParameter(ushort parameter)
{
    m_parameters.append(parameter);
}

void Token::setControllSequence(TerminalState::ControllSequence controll_sequence)
{
    m_controll_sequense = controll_sequence;
}

Tokenizer::Tokenizer()
    : m_decode_state(PlainText)
    , m_current_token_start(0)
    , m_currrent_position(0)
    , m_intermediate_char(QChar())
{
}

void Tokenizer::addData(const QByteArray &data)
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
                    m_current_token.appendText(data.mid(m_current_token_start,
                                                     m_currrent_position - m_current_token_start));
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
        m_current_token.appendText(data.mid(m_current_token_start));
        tokenFinished();
    }
    m_current_data = QByteArray();
}

void Tokenizer::decodeC0(uchar character)
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
    case C0::BS:
    case C0::HT:
        qDebug() << "Unhandled Controll character" << character;
        break;
    case C0::LF:
        m_current_token.setControllSequence(TerminalState::NewLine);
        tokenFinished();
        break;
    case C0::VT:
    case C0::FF:
        qDebug() << "Unhandled Controll character" << character;
        break;
    case C0::CR:
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

void Tokenizer::decodeC1_7bit(uchar character)
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

void Tokenizer::decodeParameters(uchar character)
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
        m_current_token.addParameter(m_parameter_string.toInt());
        m_parameter_string = QString();
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

void Tokenizer::decodeCSI(uchar character)
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
                        m_current_token.setControllSequence(TerminalState::UnknownControllSequence);
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
                    case FinalBytesNoIntermediate::CUP:
                    case FinalBytesNoIntermediate::CHT:
                    case FinalBytesNoIntermediate::ED:
                    case FinalBytesNoIntermediate::EL:
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
                        m_current_token.setControllSequence(TerminalState::UnknownControllSequence);
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::SM:
                        m_current_token.setControllSequence(TerminalState::UnknownControllSequence);
                        qDebug() << "SET MODE!!!!";
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::MC:
                    case FinalBytesNoIntermediate::HPB:
                    case FinalBytesNoIntermediate::VPB:
                    case FinalBytesNoIntermediate::RM:
                        m_current_token.setControllSequence(TerminalState::UnknownControllSequence);
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::SGR:
                        m_current_token.setControllSequence(TerminalState::SetAttributeMode);
                        if (!m_parameter_string.isEmpty())
                            m_current_token.addParameter(m_parameter_string.toUShort());
                        tokenFinished();
                        break;
                    case FinalBytesNoIntermediate::DSR:
                    case FinalBytesNoIntermediate::DAQ:
                    default:
                        qDebug() << "Unhandeled CSI squence\n";
                        m_current_token.setControllSequence(TerminalState::UnknownControllSequence);
                        tokenFinished();
                        break;
                    }
                }
            }
        }
}

void Tokenizer::decodeOSC(uchar character)
{
        if (!m_current_token.parameters().size() &&
                character >= 0x30 && character <= 0x3f) {
            decodeParameters(character);
        } else {
            if (m_current_token.controllSequence() ==  TerminalState::NoControllSequence) {
                if (m_parameter_string.size()) {
                    m_current_token.addParameter(m_parameter_string.toUShort());
                    m_parameter_string = QString();
                }
                Q_ASSERT(m_current_token.parameters().size() == 1);
                TerminalState::ControllSequence windowAttribute;
                switch (m_current_token.parameters().at(0)) {
                case 0:
                    windowAttribute = TerminalState::ChangeWindowAndIconName;
                    break;
                case 1:
                    windowAttribute = TerminalState::ChangeIconTitle;
                    break;
                case 2:
                    windowAttribute = TerminalState::ChangeWindowTitle;
                    break;
                default:
                    windowAttribute = TerminalState::UnknownControllSequence;
                    break;
                }
                m_current_token.setControllSequence(windowAttribute);
            } else {
                if (character == 0x07) {
                    m_current_token.appendText(m_current_data.mid(m_current_token_start+4,
                                                                  m_currrent_position - m_current_token_start -1));
                    tokenFinished();
                }
            }
        }
}

void Tokenizer::tokenFinished()
{
    if (!m_current_token.isEmpty()) {
        m_tokens << m_current_token;
        m_current_token = Token();
    }
    m_current_token_start = m_currrent_position + 1;
    m_intermediate_char = 0;
    if (m_parameter_string.size())
        m_parameter_string = QString();
    m_decode_state = PlainText;
}

