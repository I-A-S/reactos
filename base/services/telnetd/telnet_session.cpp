/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnet_session.hpp"

#include <cassert>

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

static constexpr uint8_t COMMAND_STRIDE_LUT[16] =
{
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 3, 3, 3, 3, 2,
};

void TelnetSessionHandler::on_connect()
{
    OutputDebugStringA("telnetd: client connected!\n");

    const uint8_t tmp[9] = {
        0xFF, 0xFB, 0x01,
        0xFF, 0xFB, 0x03,
        0xFF, 0xFD, 0x03,
    };
    send_data(tmp, sizeof(tmp));
}

void TelnetSessionHandler::on_disconnect()
{
    OutputDebugStringA("telnetd: client disconnected.\n");

}

void TelnetSessionHandler::on_data(const uint8_t* data, int length)
{
    const uint8_t* data_cursor = data;

    while(length > 0) {
        
        if(*data == 0xFF) {

            if(data_cursor != data) {
                handle_data(data_cursor, data - data_cursor);
                data_cursor = data;
            }

            const auto stride = handle_command(data);
            data += stride;
            length -= stride;
            data_cursor += stride;

            continue;
        }

        data++;
        length--;
    }
    if(data_cursor != data)
        handle_data(data_cursor, data - data_cursor);
}


void TelnetSessionHandler::handle_data(const uint8_t* data, int length)
{
    OutputDebugStringA("telnetd: got data:");

    CHAR buf[256];
    const auto buf_len = min((size_t)255, length);
    RtlCopyMemory(buf, data, buf_len);
    buf[buf_len] = '\0';
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
    
    send_data(data, length);
}

int TelnetSessionHandler::handle_command(const uint8_t* cmd)
{
    OutputDebugStringA("telnetd: got command:");

    assert(*cmd == 0xFF);    
    cmd++;
    
    int stride = COMMAND_STRIDE_LUT[*cmd - 240];

    if(*cmd == (uint8_t)ECommand::SB)
    {
        while(*cmd != (uint8_t)ECommand::SE) {
            cmd++;
        }
    }
    
    return stride;
}
