/*
 * PROJECT:     ReactOS ConHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Console Host.
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga
 */

#include <consrv.h>

#define NDEBUG
#include <debug.h>

NTSTATUS NTAPI
GuiLoadFrontEnd(IN OUT PFRONTEND FrontEnd,
                IN OUT PCONSOLE_STATE_INFO ConsoleInfo,
                IN OUT PCONSOLE_INIT_INFO ConsoleInitInfo,
                IN HANDLE ConsoleLeaderProcessHandle);
NTSTATUS NTAPI
GuiUnloadFrontEnd(IN OUT PFRONTEND FrontEnd);

INT
WINAPI
wWinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPWSTR lpCmdLine,
    INT nCmdShow)
{
    DPRINT1("conhost: %S\n", lpCmdLine);

    GuiUnloadFrontEnd(NULL);

    return 0;
}