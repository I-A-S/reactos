/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnetd.hpp"

#include <winbase.h>

extern "C" int kickoff_telnetd(DWORD argc, LPWSTR* argv)
{
    OutputDebugStringA("telnetd: hello\n");

    return TELNETD_STATUS_OK;
}

extern "C" void shutdown_telnetd(void)
{

}