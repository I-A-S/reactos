/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#pragma once

#include <windef.h>

#define TELNETD_STATUS_OK 0
#define TELNETD_STATUS_ERR_SOCK_CREATE 1
#define TELNETD_STATUS_ERR_SOCK_BIND 2

#ifdef __cplusplus
extern "C"
#endif
int kickoff_telnetd(DWORD argc, LPWSTR* argv);

#ifdef __cplusplus
extern "C"
#endif
void shutdown_telnetd(void);

#ifdef __cplusplus

class Telnetd
{
public:

private:

};

#endif