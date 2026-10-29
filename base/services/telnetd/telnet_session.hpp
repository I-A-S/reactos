/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#pragma once

#include "tcpserver.hpp"

class TelnetSessionHandler: public TCPServerSessionHandler
{
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

private:
    void handle_text(const uint8_t* data, int length);
    void handle_command(ECommand cmd, const uint8_t* params);
    void handle_subnegotiation(uint8_t option, const uint8_t* params, uint8_t length);

    void send_text(const uint8_t* data, int length);

    inline void send_iac_will(uint8_t param) { uint8_t data[3] = { (uint8_t)ECommand::IAC, (uint8_t)ECommand::WILL, param, }; send_data(data, sizeof(data)); }
    inline void send_iac_wont(uint8_t param) { uint8_t data[3] = { (uint8_t)ECommand::IAC, (uint8_t)ECommand::WONT, param, }; send_data(data, sizeof(data)); }
    inline void send_iac_do(uint8_t param)   { uint8_t data[3] = { (uint8_t)ECommand::IAC, (uint8_t)ECommand::DO, param, }; send_data(data, sizeof(data)); }
    inline void send_iac_dont(uint8_t param) { uint8_t data[3] = { (uint8_t)ECommand::IAC, (uint8_t)ECommand::DONT, param, }; send_data(data, sizeof(data)); }

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
};
