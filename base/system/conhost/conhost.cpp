/*
 * PROJECT:     ReactOS ConHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Console Host.
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga
 */

#include <windows.h>

#define NDEBUG
#include <debug.h>

INT
WINAPI
wWinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPWSTR lpCmdLine,
    INT nCmdShow)
{
    DPRINT1("conhost: %S\n", lpCmdLine);

    return 0;
}
