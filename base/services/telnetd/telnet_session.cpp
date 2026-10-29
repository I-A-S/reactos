/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnet_session.hpp"

#include <cassert>
#include <cstdint>

using cursor_type = const uint8_t*;

TelnetSessionHandler::TelnetSessionHandler()
{
    m_sb_buffer.reserve(MAX_SUBNEGO_PAYLOAD_LENGTH);
    InitializeCriticalSection(&m_out_text_cs);
}

TelnetSessionHandler::~TelnetSessionHandler()
{
    DeleteCriticalSection(&m_out_text_cs);
}

void TelnetSessionHandler::on_connect()
{
    OutputDebugStringA("telnetd: client connected!\n");

    send_iac_will(0x01);
    send_iac_will(0x03);
    send_iac_do(0x03);

    send_text((const uint8_t*)"Hello from ReactOS!\n", 20);
}

void TelnetSessionHandler::on_disconnect()
{
    OutputDebugStringA("telnetd: client disconnected.\n");

}

void TelnetSessionHandler::on_data(const uint8_t* data, int length)
{
    const cursor_type end_cur = data + length;
    cursor_type scan_cur{data}, text_cur{data};

    while(scan_cur < end_cur)
    {
        switch(m_parser_state)
        {
            case EParserState::TEXT:
                if (*scan_cur++ == (uint8_t)ECommand::IAC) {
                    handle_text(text_cur, scan_cur - text_cur - 1);
                    text_cur = scan_cur;
                    m_parser_state = EParserState::AFTER_IAC;
                }
                break;

            case EParserState::AFTER_IAC:
            {
                const auto c = (ECommand)*scan_cur++;
                if(c == ECommand::SB) {
                    m_parser_state = EParserState::AFTER_SB;
                } else if(
                    (c >= ECommand::WILL) &&
                    (c <= ECommand::DONT)
                ) {
                    m_parser_state_data = (uint8_t)c;
                    m_parser_state = EParserState::AFTER_VERB;
                } else {
                    if (c == ECommand::IAC)
                        handle_text((const uint8_t*)&c, 1);
                    else
                        handle_command(c, nullptr);
                    text_cur = scan_cur;
                    m_parser_state = EParserState::TEXT;
                }
            }
            break;

            case EParserState::AFTER_VERB:
                handle_command((ECommand)m_parser_state_data, scan_cur++);
                text_cur = scan_cur;
                m_parser_state = EParserState::TEXT;
                break;

            case EParserState::AFTER_SB:
                m_sb_buffer.clear();
                m_parser_state_data = *scan_cur++;
                m_parser_state = EParserState::SUBNEGO_PAYLOAD;
                break;

            case EParserState::SUBNEGO_PAYLOAD:
            {
                const auto c = *scan_cur++;

                if (c == (uint8_t)ECommand::IAC) {
                    m_parser_state = EParserState::SUBNEGO_AFTER_IAC;
                } else {
                    if (m_sb_buffer.size() >= MAX_SUBNEGO_PAYLOAD_LENGTH) {
                        m_parser_state = EParserState::MALFORMED_SUBNEGO;
                        break;
                    }

                    m_sb_buffer.push_back(c);
                }
            }
            break;

            case EParserState::SUBNEGO_AFTER_IAC:
            {
                const auto c = (ECommand)*scan_cur++;

                if (c == ECommand::IAC) {
                    if (m_sb_buffer.size() >= MAX_SUBNEGO_PAYLOAD_LENGTH) {
                        m_parser_state = EParserState::MALFORMED_SUBNEGO;
                        break;
                    }

                    m_sb_buffer.push_back(0xFF);
                    m_parser_state = EParserState::SUBNEGO_PAYLOAD;
                } else if (c == ECommand::SE) {
                    assert(m_sb_buffer.size() <= MAX_SUBNEGO_PAYLOAD_LENGTH);
                    handle_subnegotiation(m_parser_state_data, m_sb_buffer.data(), (uint8_t)m_sb_buffer.size());
                    text_cur = scan_cur;
                    m_parser_state = EParserState::TEXT;
                } else {
                    scan_cur--;
                    m_parser_state = EParserState::AFTER_IAC;
                }
            }
            break;

            case EParserState::MALFORMED_SUBNEGO:
                if (*scan_cur++ == (uint8_t)ECommand::IAC)
                    m_parser_state = EParserState::MALFORMED_SUBNEGO_AFTER_IAC;
            break;

            case EParserState::MALFORMED_SUBNEGO_AFTER_IAC:
            {
                const auto c = (ECommand)*scan_cur++;

                if (c == ECommand::SE) {
                    text_cur = scan_cur;
                    m_parser_state = EParserState::TEXT;
                } else if (c == ECommand::IAC)
                    m_parser_state = EParserState::MALFORMED_SUBNEGO;
                else {
                    scan_cur--;
                    m_parser_state = EParserState::AFTER_IAC;
                }
            }
            break;
        }
    }

    if (m_parser_state == EParserState::TEXT)
        handle_text(text_cur, scan_cur - text_cur);
}

void TelnetSessionHandler::handle_text(const uint8_t* data, int length)
{
    if (length <= 0)
        return;

    OutputDebugStringA("telnetd: got data:");

    CHAR buf[256];
    const auto buf_len = std::min<int>(255, length);
    RtlCopyMemory(buf, data, buf_len);
    buf[buf_len] = '\0';
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
    
    send_text(data, length);
}

void TelnetSessionHandler::handle_command(ECommand cmd, const uint8_t* params)
{
    OutputDebugStringA("telnetd: got command:");
    
}

void TelnetSessionHandler::handle_subnegotiation(uint8_t option, const uint8_t* params, uint8_t length)
{
    OutputDebugStringA("telnetd: started subnegotiation:");

}

void TelnetSessionHandler::send_text(const uint8_t* data, int length)
{
    if (length <= 0)
        return;

    const cursor_type end_cur = data + length;
    cursor_type write_cur{data}, text_cur{data};

    EnterCriticalSection(&m_out_text_cs);

    // 1.25 * length to account for potential escaped IACs, CRs, LFs
    m_out_text_buffer.reserve(length + (length >> 2)); 
    m_out_text_buffer.clear();

    while(write_cur < end_cur)
    {        
        const auto c = *write_cur;

        if (m_last_out_was_cr && c != '\n' && c != '\0') {
            m_out_text_buffer.insert(m_out_text_buffer.end(), text_cur, write_cur);
            m_out_text_buffer.push_back(0);
            text_cur = write_cur;
            m_last_out_was_cr = false;
        }

        if (c == 0xFF) {
            m_out_text_buffer.insert(m_out_text_buffer.end(), text_cur, write_cur);
            m_out_text_buffer.push_back(0xFF);
            m_out_text_buffer.push_back(0xFF);
            text_cur = write_cur + 1;
        } else if (c == '\n') {
            m_out_text_buffer.insert(m_out_text_buffer.end(), text_cur, write_cur);
            if(!m_last_out_was_cr)
                m_out_text_buffer.push_back('\r');
            m_out_text_buffer.push_back('\n');
            m_last_out_was_cr = false;
            text_cur = write_cur + 1;
        } else
            m_last_out_was_cr = (c == '\r');

        write_cur++;
    }
    m_out_text_buffer.insert(m_out_text_buffer.end(), text_cur, write_cur);

    send_data(m_out_text_buffer.data(), (int)m_out_text_buffer.size());

    LeaveCriticalSection(&m_out_text_cs);
}
