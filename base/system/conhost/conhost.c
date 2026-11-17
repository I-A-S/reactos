/*
 * PROJECT:     ReactOS ConHost
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Console Host.
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga
 */

#include <consrv.h>

#include <include/console.h>
#include <console.h>
#include <concfg/settings.h>

#define NDEBUG
#include <debug.h>

HANDLE    ConSrvHeap;
HINSTANCE ConSrvDllInstance;

VOID
ConSrvInitObject(PCONSOLE_IO_OBJECT Object,
    CONSOLE_IO_OBJECT_TYPE Type,
    PCONSOLE Console)
{
    Object->Type = Type;
    Object->Console = Console;
    Object->AccessRead = 0;
    Object->AccessWrite = 0;
    Object->ExclusiveRead = 0;
    Object->ExclusiveWrite = 0;
    Object->ReferenceCount = 0;
}

// HACK
VOID HistoryReshapeAllBuffers(PCONSRV_CONSOLE C, ULONG a, ULONG b, BOOLEAN c) {}
VOID ConioPause(PCONSRV_CONSOLE Console, UCHAR Flags) {}
VOID ConioUnpause(PCONSRV_CONSOLE Console, UCHAR Flags) {}
PCONSOLE_PROCESS_DATA NTAPI ConSrvGetConsoleLeaderProcess(PCONSRV_CONSOLE Console) { return NULL; }
NTSTATUS NTAPI ConSrvConsoleProcessCtrlEvent(PCONSRV_CONSOLE C, ULONG g, ULONG e) { return STATUS_NOT_IMPLEMENTED; }
NTSTATUS NTAPI ConSrvSetConsoleProcessFocus(PCONSRV_CONSOLE C, BOOLEAN f) { return STATUS_NOT_IMPLEMENTED; }
NTSTATUS ConioProcessInputEvent(PCONSRV_CONSOLE C, PINPUT_RECORD e) { return STATUS_NOT_IMPLEMENTED; }
// HACK

NTSTATUS NTAPI
ConSrvInitTerminal(IN OUT PTERMINAL Terminal,
                   IN OUT PCONSOLE_STATE_INFO ConsoleInfo,
                   IN OUT PCONSOLE_INIT_INFO ConsoleInitInfo,
                   IN HANDLE ConsoleLeaderProcessHandle);

NTSTATUS NTAPI
ConDrvWriteConsole(IN PCONSOLE Console,
                   IN PTEXTMODE_SCREEN_BUFFER ScreenBuffer,
                   IN BOOLEAN Unicode,
                   IN PVOID StringBuffer,
                   IN ULONG NumCharsToWrite,
                   OUT PULONG NumCharsWritten OPTIONAL);

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