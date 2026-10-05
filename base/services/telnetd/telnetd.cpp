/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "telnetd.hpp"

#include <winbase.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(telnetd);

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

    const auto wsa_status = WSAStartup(MAKEWORD(2,2), &m_wsa_data);
    if(wsa_status != 0) {
        ERR("winsock init failed with: %d\n", wsa_status);
        terminate();
        return TELNETD_STATUS_ERR_WINSCK_INIT;
    }

    m_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if(!m_stop_event) {
        terminate();
        return TELNETD_STATUS_ERR_EVNT_CREATE;
    }

    m_accept_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if(!m_accept_event) {
        terminate();
        return TELNETD_STATUS_ERR_EVNT_CREATE;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_socket == INVALID_SOCKET) {
        ERR("winsock socket create failed with: %d\n", WSAGetLastError());
        terminate();
        return TELNETD_STATUS_ERR_SOCK_CREATE;
    }

    sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(23);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(m_socket, (const sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        ERR("winsock socket bind failed with: %d\n", WSAGetLastError());
        terminate();
        return TELNETD_STATUS_ERR_SOCK_BIND;
    }

    if(listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
        ERR("winsock socket listen failed with: %d\n", WSAGetLastError());
        terminate();
        return TELNETD_STATUS_ERR_SOCK_LISTEN;
    }
    
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

    if(m_socket != INVALID_SOCKET) closesocket(m_socket);
    m_socket = INVALID_SOCKET;

    if(m_accept_event) CloseHandle(m_accept_event);
    if(m_stop_event) CloseHandle(m_stop_event);
    if(m_worker_thread) CloseHandle(m_worker_thread);
    m_worker_thread = NULL;
    m_stop_event = NULL;
    m_accept_event = NULL;

    if(m_wsa_data.wVersion) WSACleanup();
    m_wsa_data.wVersion = 0;
}

TELNETD_STATUS Telnetd::serve()
{
    OutputDebugStringA("telnetd: serve called\n");

    WSAEventSelect(m_socket, m_accept_event, FD_ACCEPT);

    HANDLE events[] = {m_stop_event, m_accept_event};
    const auto wait_result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (wait_result == WAIT_OBJECT_0)
        return TELNETD_STATUS_OK;
    else if (wait_result != (WAIT_OBJECT_0 + 1))
        return TELNETD_STATUS_ERR_UNKNOWN;

    return TELNETD_STATUS_OK;
}

DWORD WINAPI Telnetd::worker_thread_func(LPVOID self)
{
    return ((Telnetd*)self)->serve();
}