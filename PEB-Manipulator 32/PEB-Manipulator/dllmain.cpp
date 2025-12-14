#include "pch.h"
#include "x64dbg/_plugins.h"
#include <Windows.h>
#include "resource.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)
#define PLUGIN_NAME "Retr0-x86"
#define VERSION 1

#define MENU_HIDE_DEBUGGER  1
#define MENU_SHOW_CMDLINE   2
#define MENU_ASLR_DEP       3
#define MENU_DIALOG         4
#define MENU_ABOUT          5

// ================================
// x86 OFFSETS
// ================================
#define PEB_BEINGDEBUGGED_OFFSET     0x02
#define PEB_IMAGEBASE_OFFSET         0x08
#define PEB_PROCESSPARAMS_OFFSET     0x10
#define PEB_NTGLOBALFLAG_OFFSET      0x68
#define RTL_CMDLINE_OFFSET           0x40

// ================================
// UNICODE_STRING (x86)
// ================================
typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

// ================================
static int pluginHandle = 0;
static int hMenu = 0;
HINSTANCE hInstance = nullptr;

// ================================
void ShowCmdLine();
void ShowASLRandDEP();
bool ClearDebugFlags();
void ChangeArguments(LPCWSTR);

// ================================
// Dialog
// ================================
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON1:
            ShowCmdLine();
            return TRUE;

        case IDC_BUTTON2:
            ShowASLRandDEP();
            return TRUE;

        case IDC_BUTTON3:
            ClearDebugFlags();
            return TRUE;

        case IDC_BUTTON4:
        {
            WCHAR buffer[MAX_PATH]{};
            GetDlgItemText(hwndDlg, IDC_EDIT1, buffer, MAX_PATH);
            ChangeArguments(buffer);
            return TRUE;
        }

        case IDOK:
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// ================================
// Change CommandLine (x86)
// ================================
void ChangeArguments(LPCWSTR arguments)
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
        return;

    duint procParams = 0;
    if (!DbgMemRead(peb + PEB_PROCESSPARAMS_OFFSET,
        &procParams, sizeof(procParams)))
        return;

    UNICODE_STRING cmd{};
    if (!DbgMemRead(procParams + RTL_CMDLINE_OFFSET,
        &cmd, sizeof(cmd)))
        return;

    SIZE_T newLen = (wcslen(arguments) + 1) * sizeof(WCHAR);
    if (newLen > cmd.MaximumLength)
        return;

    DbgMemWrite((duint)cmd.Buffer, arguments, newLen);

    cmd.Length = (USHORT)(newLen - sizeof(WCHAR));
    DbgMemWrite(procParams + RTL_CMDLINE_OFFSET,
        &cmd, sizeof(cmd));

    MessageBoxW(nullptr, L"CommandLine changed (x86)",
        L"Retr0", MB_OK | MB_ICONINFORMATION);
}

// ================================
// Clear PEB debug flags (x86)
// ================================
bool ClearDebugFlags()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
        return false;

    BYTE beingDebugged = 0;
    DWORD ntGlobalFlag = 0;

    DbgMemWrite(peb + PEB_BEINGDEBUGGED_OFFSET,
        &beingDebugged, sizeof(beingDebugged));

    DbgMemWrite(peb + PEB_NTGLOBALFLAG_OFFSET,
        &ntGlobalFlag, sizeof(ntGlobalFlag));

    MessageBoxW(nullptr, L"PEB flags cleared (x86)",
        L"Retr0", MB_OK | MB_ICONINFORMATION);
    return true;
}

// ================================
// Show CommandLine (x86)
// ================================
void ShowCmdLine()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
        return;

    duint procParams = 0;
    if (!DbgMemRead(peb + PEB_PROCESSPARAMS_OFFSET,
        &procParams, sizeof(procParams)))
        return;

    UNICODE_STRING cmd{};
    if (!DbgMemRead(procParams + RTL_CMDLINE_OFFSET,
        &cmd, sizeof(cmd)))
        return;

    if (!cmd.Length || !cmd.Buffer)
        return;

    PWSTR buffer = (PWSTR)malloc(cmd.Length + sizeof(WCHAR));
    memset(buffer, 0, cmd.Length + sizeof(WCHAR));

    DbgMemRead((duint)cmd.Buffer, buffer, cmd.Length);
    MessageBoxW(nullptr, buffer,
        L"CommandLine (x86)", MB_OK);

    free(buffer);
}

// ================================
// Show ASLR / DEP (x86)
// ================================
void ShowASLRandDEP()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
        return;

    duint imageBase = 0;
    DbgMemRead(peb + PEB_IMAGEBASE_OFFSET,
        &imageBase, sizeof(imageBase));

    IMAGE_DOS_HEADER dos{};
    DbgMemRead(imageBase, &dos, sizeof(dos));

    duint ntAddr = imageBase + dos.e_lfanew;

    IMAGE_NT_HEADERS32 nt{};
    if (!DbgMemRead(ntAddr, &nt, sizeof(nt)))
        return;

    WORD flags = nt.OptionalHeader.DllCharacteristics;

    WCHAR text[256];
    swprintf(text, 256,
        L"DllCharacteristics: 0x%04X\nASLR: %s\nDEP: %s",
        flags,
        (flags & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) ? L"ON" : L"OFF",
        (flags & IMAGE_DLLCHARACTERISTICS_NX_COMPAT) ? L"ON" : L"OFF");

    MessageBoxW(nullptr, text,
        L"ASLR / DEP (x86)", MB_OK);
}

// ================================
// Plugin
// ================================
DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->sdkVersion = PLUG_SDKVERSION;
    initStruct->pluginVersion = VERSION;
    pluginHandle = initStruct->pluginHandle;
    strcpy_s(initStruct->pluginName, PLUGIN_NAME);
    return true;
}

void cbMenuEntry(CBTYPE, void* info)
{
    auto menu = (PLUG_CB_MENUENTRY*)info;

    switch (menu->hEntry)
    {
    case MENU_HIDE_DEBUGGER:
        ClearDebugFlags();
        break;

    case MENU_SHOW_CMDLINE:
        ShowCmdLine();
        break;

    case MENU_ASLR_DEP:
        ShowASLRandDEP();
        break;

    case MENU_DIALOG:
        DialogBox(hInstance,
            MAKEINTRESOURCE(IDD_DIALOG1),
            GuiGetWindowHandle(),
            DialogProc);
        break;

    case MENU_ABOUT:
        MessageBoxW(nullptr, L"Retr0 x86 Plugin",
            L"About", MB_OK);
        break;
    }
}

DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setup)
{
    hMenu = _plugin_menuadd(setup->hMenu, "Retr0 x86");

    _plugin_menuaddentry(hMenu, MENU_HIDE_DEBUGGER, "Hide Debugger");
    _plugin_menuaddentry(hMenu, MENU_SHOW_CMDLINE, "Show CommandLine");
    _plugin_menuaddentry(hMenu, MENU_ASLR_DEP, "ASLR / DEP");
    _plugin_menuaddentry(hMenu, MENU_DIALOG, "Dialog");
    _plugin_menuaddentry(hMenu, MENU_ABOUT, "About");

    _plugin_registercallback(pluginHandle,
        CB_MENUENTRY, cbMenuEntry);
}

DLL_EXPORT void plugstop()
{
    _plugin_menuclear(pluginHandle);
}

BOOL APIENTRY DllMain(HINSTANCE hInst,
    DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        hInstance = hInst;
    return TRUE;
}
