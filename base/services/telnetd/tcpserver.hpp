/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

// [IANOTE]: For this telnet PR, I'll keep the tcp server implementation
// here in telnetd source tree. When we get to SSH, it will be better
// to make this a static helper lib in "sdk/lib" instead.

#pragma once

#define _INC_WINDOWS
#include <winsock2.h>

#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C
#endif



#undef EXTERN_C
