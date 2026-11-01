/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#pragma once

#include "tcpserver.hpp"

#include <string>

class TelnetSessionHandler: public TCPServerSessionHandler
{
    static constexpr size_t MAX_TEXT_LINE_LENGTH = 8000;
    static constexpr size_t MAX_SUBNEGO_PAYLOAD_LENGTH = 255;

    enum class ECommand: uint8_t
    {
        xEOF = 236,
        SUSP = 237,
        ABORT = 238,
        EOR = 239,
        SE = 240,
        NOP = 241,
        DM = 242,
        BRK = 243,
        IP = 244,
        AO = 245,
        AYT = 246,
        EC = 247,
        EL = 248,
        GA = 249,
        SB = 250,
        WILL = 251,
        WONT = 252,
        DO = 253,
        DONT = 254,
        IAC = 255,
    };

    enum class EOption: uint8_t
    {
        BINARY = 0,
        ECHO = 1,
        SGA = 3,
        TM = 6,

        TTYPE = 24,
        NAWS = 31,
    };

    enum class EParserState: uint8_t
    {
        TEXT,
        AFTER_IAC,
        AFTER_VERB,
        AFTER_SB,
        SUBNEGO_PAYLOAD,
        SUBNEGO_AFTER_IAC,
        MALFORMED_SUBNEGO,
        MALFORMED_SUBNEGO_AFTER_IAC,
    };

    enum class EEchoState: uint8_t
    {
        INVALID,
        AGREED_TO,
        AGREED_NOT_TO,
        WE_OFFERED_TO,
    };

    enum class EState: uint8_t
    {
        INVALID,
        WAITING_FOR_USERNAME,
        WAITING_FOR_PASSWORD,
        TERMINAL,
    };

private:
    void handle_text(const uint8_t* data, int length);
    void handle_text_line(const std::string& line);
    void handle_command(ECommand cmd);
    void handle_option_command(ECommand verb, EOption option);
    void handle_subnegotiation(uint8_t option, const uint8_t* params, uint8_t length);

    void send_text(const uint8_t* data, int length);

    inline bool should_echo();
    inline void send_string(const char* str);
    inline void send_option_command(ECommand verb, EOption option);

public:
    TelnetSessionHandler();
    ~TelnetSessionHandler();

    void on_connect() override;

    void on_disconnect() override;

    void on_data(const uint8_t* data, int length) override;

private:
    uint8_t m_parser_state_data{};
    std::vector<uint8_t> m_sb_buffer{};
    EParserState m_parser_state{EParserState::TEXT};

    CRITICAL_SECTION m_out_text_cs{};
    bool m_last_out_was_cr{};
    std::vector<uint8_t> m_out_text_buffer{};

    EState m_state{EState::INVALID};
    EEchoState m_echo_state{EEchoState::INVALID};

    bool m_text_last_byte_was_cr{};
    std::string m_text_line_buffer{};
};

void TelnetSessionHandler::send_string(const char* str)
{
    send_text((const uint8_t*)str, strlen(str));
}

void TelnetSessionHandler::send_option_command(ECommand verb, EOption option)
{
    uint8_t data[3] = { (uint8_t)ECommand::IAC, (uint8_t)verb, (uint8_t)option, };
    send_data(data, sizeof(data));
}

bool TelnetSessionHandler::should_echo()
{
    return (m_echo_state == EEchoState::AGREED_TO) && (m_state != EState::WAITING_FOR_PASSWORD);
}