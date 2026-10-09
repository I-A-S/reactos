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
public:
    void on_connect() override;

    void on_disconnect() override;

    void on_data(const uint8_t* data, size_t length) override;
};
