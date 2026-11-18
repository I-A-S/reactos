/*
 * PROJECT:     ReactOS Console Host
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Console Host.
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga
 */

#include <consrv.h>

#include <include/console.h>
#include <console.h>
#include <concfg/settings.h>

//#define NDEBUG
#include <debug.h>

HANDLE    ConSrvHeap;
HINSTANCE ConSrvDllInstance;

// HACK
NTSTATUS ConioProcessInputEvent(PCONSRV_CONSOLE C, PINPUT_RECORD e) { return STATUS_NOT_IMPLEMENTED; }
// HACK

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
    static WCHAR Title[] = L"ReactOS Console Host";
    static const WCHAR Text[] = L"conhost.exe is working!\r\n";

    NTSTATUS Status;
    HWND hWnd;
    HANDLE ConsoleHandle;
    PCONSRV_CONSOLE Console;
    
    CONSOLE_INIT_INFO ConsoleInitInfo;
    CONSOLE_START_INFO ConsoleStartInfo;

    CSR_PROCESS LeaderProcess; // [IATODO]: Change func sig
    WCHAR AppName[MAX_PATH];
    WCHAR CurDir[MAX_PATH];

    DPRINT("CONHOST: Starting with args \"%S\"..\n", lpCmdLine);

    ConSrvDllInstance = hInst;
    ConSrvHeap = RtlGetProcessHeap();

    ConSrvInitConsoleSupport();

    GetModuleFileNameW(NULL, AppName, ARRAYSIZE(AppName));
    GetCurrentDirectoryW(ARRAYSIZE(CurDir), CurDir);

    RtlZeroMemory(&ConsoleInitInfo, sizeof(ConsoleInitInfo));
    RtlZeroMemory(&ConsoleStartInfo, sizeof(ConsoleStartInfo));
    ConsoleInitInfo.ConsoleStartInfo = &ConsoleStartInfo;
    ConsoleInitInfo.IsWindowVisible  = TRUE;
    ConsoleInitInfo.ConsoleTitle     = Title;
    ConsoleInitInfo.TitleLength      = (ULONG)wcslen(Title);
    ConsoleInitInfo.AppName          = AppName;
    ConsoleInitInfo.AppNameLength    = (ULONG)wcslen(AppName);
    ConsoleInitInfo.CurDir           = CurDir;
    ConsoleInitInfo.CurDirLength     = (ULONG)wcslen(CurDir);

    RtlZeroMemory(&LeaderProcess, sizeof(LeaderProcess));
    LeaderProcess.ProcessHandle = NtCurrentProcess();

    Status = ConSrvInitConsole(&ConsoleHandle, &Console, &ConsoleInitInfo, &LeaderProcess);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CONHOST: ConSrvInitConsole failed, Status = 0x%08lx\n", Status);
        return 1;
    }

    DPRINT("CONHOST: Initialized successfully!\n");

    EnterCriticalSection(&Console->Lock);
    Status = ConDrvWriteConsole((PCONSOLE)Console,
                                (PTEXTMODE_SCREEN_BUFFER)Console->ActiveBuffer,
                                TRUE, (PVOID)Text, ARRAYSIZE(Text) - 1, NULL);
    LeaveCriticalSection(&Console->Lock);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CONHOST: ConDrvWriteConsole failed, Status = 0x%08lx\n", Status);
        return 1;
    }

    hWnd = Console->FrontEndIFace.Vtbl->GetConsoleWindowHandle(&Console->FrontEndIFace);
    while (1)
    {
        if (!hWnd || !IsWindow(hWnd)) break;
        Sleep(100);
    }

    DPRINT("CONHOST: Exiting normally..\n");

    return 0;
}