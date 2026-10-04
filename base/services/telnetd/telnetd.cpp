/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnetd.hpp"

#include <winbase.h>

static Telnetd* g_telnetd{};

static DWORD WINAPI worker_thread(LPVOID)
{
    g_telnetd->serve();

    return 0;
}

extern "C" TELENTD_STATUS kickoff_telnetd(HANDLE* out_worker_thread)
{
    TELENTD_STATUS status {TELNETD_STATUS_OK};

    g_telnetd = new Telnetd();

    status = g_telnetd->initialize();
    
    if(status != TELNETD_STATUS_OK) {
        delete g_telnetd;
        return status;
    }
    
    DWORD worker_thread_id{};
    HANDLE worker_thread_handle =  CreateThread(NULL, 
        0, 
        worker_thread, 
        nullptr, 
        0, 
        &worker_thread_id
    );
    if(!worker_thread_handle) {
        delete g_telnetd;
        return TELNETD_STATUS_ERR_THRD_CREATE;
    }
    *out_worker_thread = worker_thread_handle;

    return status;
}

extern "C" void shutdown_telnetd(void)
{
    if(g_telnetd)
        g_telnetd->terminate();

    delete g_telnetd;
}

TELENTD_STATUS Telnetd::initialize()
{
    OutputDebugStringA("telnetd: initialize called\n");
    
    return TELNETD_STATUS_OK;
}

void Telnetd::terminate()
{
    OutputDebugStringA("telnetd: terminate called\n");
}

TELENTD_STATUS Telnetd::serve()
{
    OutputDebugStringA("telnetd: serve called\n");

    while(true) {}

    return TELNETD_STATUS_OK;
}
