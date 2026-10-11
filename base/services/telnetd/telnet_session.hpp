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
    enum class ECommand: uint8_t
    {
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

    void handle_data(const uint8_t* data, int length);
    void handle_command(ECommand cmd, const uint8_t* params);
    void handle_subnegotiation(uint8_t option, const uint8_t* params, uint8_t length);

public:
    void on_connect() override;

    void on_disconnect() override;

    void on_data(const uint8_t* data, int length) override;

private:
    uint8_t m_command_buffer[128];
    uint8_t m_command_buffer_cursor{};
};
