/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnetd.hpp"

#include <winbase.h>

static Telnetd g_telnetd{};

extern "C" TELNETD_STATUS kickoff_telnetd(HANDLE* out_worker_thread)
{
    TELNETD_STATUS status = g_telnetd.initialize();
    if(status != TELNETD_STATUS_OK)
        return status;
    *out_worker_thread = g_telnetd.get_worker_thread();
    return status;
}

extern "C" void shutdown_telnetd(void)
{
    g_telnetd.terminate();
}

TELNETD_STATUS Telnetd::initialize()
{
    OutputDebugStringA("telnetd: initialize called\n");

    m_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if(!m_stop_event) {
        return TELNETD_STATUS_ERR_EVNT_CREATE;
    }

    // IATODO: Create socket and bind
    
    DWORD thread_id{};
    m_worker_thread =  CreateThread(NULL, 
        0, 
        Telnetd::worker_thread_func, 
        this, 
        0, 
        &thread_id
    );
    if(!m_worker_thread) {
        terminate();
        return TELNETD_STATUS_ERR_THRD_CREATE;
    }

    return TELNETD_STATUS_OK;
}

void Telnetd::terminate()
{
    OutputDebugStringA("telnetd: terminate called\n");

    if(m_stop_event) SetEvent(m_stop_event);
    if(m_worker_thread) WaitForSingleObject(m_worker_thread, INFINITE);

    // IATODO: Cleanup socket

    if(m_stop_event) CloseHandle(m_stop_event);
    if(m_worker_thread) CloseHandle(m_worker_thread);
    m_worker_thread = NULL;
    m_stop_event = NULL;
}

TELNETD_STATUS Telnetd::serve()
{
    OutputDebugStringA("telnetd: serve called\n");

    WaitForSingleObject(m_stop_event, INFINITE);

    return TELNETD_STATUS_OK;
}

DWORD WINAPI Telnetd::worker_thread_func(LPVOID self)
{
    return ((Telnetd*)self)->serve();
}