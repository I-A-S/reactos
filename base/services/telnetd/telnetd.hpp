/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#pragma once

#include <windef.h>

typedef int TELNETD_STATUS;

#define TELNETD_STATUS_OK 0
#define TELNETD_STATUS_ERR_SOCK_CREATE 1
#define TELNETD_STATUS_ERR_SOCK_BIND 2
#define TELNETD_STATUS_ERR_SOCK_LISTEN 3
#define TELNETD_STATUS_ERR_THRD_CREATE 4
#define TELNETD_STATUS_ERR_EVNT_CREATE 5
#define TELNETD_STATUS_ERR_UNKNOWN 6

#ifdef __cplusplus
extern "C"
#endif
TELNETD_STATUS kickoff_telnetd(HANDLE* out_worker_thread);

#ifdef __cplusplus
extern "C"
#endif
void shutdown_telnetd(void);

#ifdef __cplusplus

#define _INC_WINDOWS
#include <winsock2.h>

class Telnetd
{
    static DWORD WINAPI worker_thread_func(LPVOID self);

public:
    TELNETD_STATUS initialize();
    void terminate();

    HANDLE get_worker_thread() const { return m_worker_thread; } 

private:
    TELNETD_STATUS serve();

private:
    SOCKET m_socket {INVALID_SOCKET};
    HANDLE m_worker_thread {NULL};
    HANDLE m_stop_event {NULL};
};

#endif