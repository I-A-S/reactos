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
VOID LineInputKeyDown(PCONSRV_CONSOLE Console, PUNICODE_STRING ExeName, KEY_EVENT_RECORD *KeyEvent) {}
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
    static const WCHAR Title[] = L"ReactOS Console Host";

    NTSTATUS Status;
    HWND hWnd;
    TERMINAL Terminal;
    PCONSRV_CONSOLE Console;
    
    CONSOLE_INFO DrvConsoleInfo;
    CONSOLE_INIT_INFO ConsoleInitInfo;
    CONSOLE_START_INFO ConsoleStartInfo;

    BYTE ConsoleInfoBuffer[sizeof(CONSOLE_STATE_INFO) + MAX_PATH * sizeof(WCHAR)];
    PCONSOLE_STATE_INFO ConsoleInfo = (PCONSOLE_STATE_INFO)ConsoleInfoBuffer;

    DPRINT("CONHOST: Starting with args \"%S\"..\n", lpCmdLine);

    ConSrvDllInstance = hInst;
    ConSrvHeap = RtlGetProcessHeap();

    RtlZeroMemory(ConsoleInfoBuffer, sizeof(ConsoleInfoBuffer));
    ConsoleInfo->cbSize = sizeof(ConsoleInfoBuffer);
    wcscpy(ConsoleInfo->ConsoleTitle, Title);
    ConCfgGetDefaultSettings(ConsoleInfo);
    ConCfgReadUserSettings(ConsoleInfo, FALSE);
    ConsoleInfo->CodePage = GetOEMCP();

    RtlZeroMemory(&ConsoleInitInfo, sizeof(ConsoleInitInfo));
    RtlZeroMemory(&ConsoleStartInfo, sizeof(ConsoleStartInfo));
    ConsoleInitInfo.ConsoleStartInfo = &ConsoleStartInfo;
    ConsoleInitInfo.IsWindowVisible  = TRUE;
    ConsoleInitInfo.TitleLength      = sizeof(Title);
    ConsoleInitInfo.ConsoleTitle     = (PWCHAR)Title;

    Status = ConSrvInitTerminal(&Terminal, ConsoleInfo, &ConsoleInitInfo, NtCurrentProcess());
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CONHOST: ConSrvInitTerminal failed, Status = 0x%08lx\n", Status);
        return 1;
    }

    Console = ConsoleAllocHeap(HEAP_ZERO_MEMORY, sizeof(*Console));
    if (!Console)
    {
        DPRINT1("CONHOST: ConsoleAllocHeap failed\n");
        return 1;
    }

    RtlZeroMemory(&DrvConsoleInfo, sizeof(DrvConsoleInfo));
    DrvConsoleInfo.ScreenBufferSize = ConsoleInfo->ScreenBufferSize;
    DrvConsoleInfo.ConsoleSize      = ConsoleInfo->WindowSize;
    DrvConsoleInfo.CursorSize       = ConsoleInfo->CursorSize;
    DrvConsoleInfo.ScreenAttrib     = ConsoleInfo->ScreenAttributes;
    DrvConsoleInfo.PopupAttrib      = ConsoleInfo->PopupAttributes;
    DrvConsoleInfo.CodePage         = ConsoleInfo->CodePage;

    Status = ConDrvInitConsole((PCONSOLE)Console, &DrvConsoleInfo);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CONHOST: ConDrvInitConsole failed, Status = 0x%08lx\n", Status);
        return 1;
    }

    RtlCopyMemory(Console->Colors, ConsoleInfo->ColorTable, sizeof(ConsoleInfo->ColorTable));

    RtlInitUnicodeStringEx(&Console->OriginalTitle, ConsoleInfo->ConsoleTitle);
    RtlInitUnicodeStringEx(&Console->Title, ConsoleInfo->ConsoleTitle);

    InitializeListHead(&Console->ProcessList);
    InitializeListHead(&Console->ReadWaitQueue);
    InitializeListHead(&Console->WriteWaitQueue);
    InitializeListHead(&Console->HistoryBuffers);
    InitializeListHead(&Console->PopupWindows);

    Console->MaxNumberOfHistoryBuffers = ConsoleInfo->NumberOfHistoryBuffers;
    Console->HistoryBufferSize = ConsoleInfo->HistoryBufferSize;
    Console->HistoryNoDup      = ConsoleInfo->HistoryNoDup;
    Console->LineInsertToggle  = ConsoleInfo->InsertMode;
    Console->InsertMode        = ConsoleInfo->InsertMode;
    Console->QuickEdit         = ConsoleInfo->QuickEdit;

    Status = ConDrvAttachTerminal((PCONSOLE)Console, &Terminal);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("CONHOST: ConDrvAttachTerminal failed, Status = 0x%08lx\n", Status);
        return 1;
    }

    DPRINT("CONHOST: Initialized successfully!\n");

    hWnd = Console->FrontEndIFace.Vtbl->GetConsoleWindowHandle(&Console->FrontEndIFace);
    while (1)
    {
        if (!hWnd || !IsWindow(hWnd)) break;
        Sleep(100);
    }

    DPRINT("CONHOST: Exiting normally..\n");

    return 0;
}