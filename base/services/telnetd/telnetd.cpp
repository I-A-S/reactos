/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnetd.hpp"

#include <winbase.h>

extern "C" int kickoff_telnetd(HANDLE stop_event)
{
    OutputDebugStringA("telnetd: kickoff called\n");
    
    DWORD thread_id{};
    HANDLE thread_handle =  CreateThread(NULL, 
        0, 
        [](LPVOID arg)-> DWORD {
            OutputDebugStringA("telnetd: inside thread\n");
            return 0;
        }, 
        NULL, 
        0, 
        &thread_id
    );

    WaitForSingleObject(thread_handle, INFINITE);

    if(!SetEvent(stop_event))
        return TELNETD_STATUS_ERR_UNKNOWN;

    return TELNETD_STATUS_OK;
}

extern "C" void shutdown_telnetd(void)
{
    OutputDebugStringA("telnetd: terminate called\n");

}

int Telnetd::launch()
{
    
    return TELNETD_STATUS_OK;
}

void Telnetd::shutdown()
{

}
