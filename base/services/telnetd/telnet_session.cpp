/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnet_session.hpp"

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

void TelnetSessionHandler::on_data(const uint8_t* data, size_t length)
{
    if(data[0] == 0xFF) return;

    OutputDebugStringA("telnetd: got data:");

    CHAR buf[256];
    const auto buf_len = min((size_t)255, length);
    RtlCopyMemory(buf, data, buf_len);
    buf[buf_len] = '\0';
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
    
    send_data(data, length);
}
