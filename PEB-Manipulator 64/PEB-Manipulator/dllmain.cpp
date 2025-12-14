#include "pch.h"
#include "x64dbg/_plugins.h"
#include <Windows.h>
#include "resource.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)

#define PLUGIN_NAME    "Retr0"
#define PLUGIN_VERSION 2

#define MENU_PEB_MANIPULATOR 1
#define MENU_ABOUT           2
#define MENU_ARGUMENTS       3
#define MENU_ASLR_DEP        4
#define MENU_DIALOG          5

// ----------------------------------------------------------------------
// Native Structures
// ----------------------------------------------------------------------
typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

// ----------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------
static HINSTANCE g_hInstance = nullptr;
static int       g_PluginHandle = 0;
static int       g_MenuHandle = 0;

// ----------------------------------------------------------------------
// Forward Declarations
// ----------------------------------------------------------------------
void ShowCmdLine();
void ShowASLRandDEP();
bool ClearDebugFlags();
void ChangeArguments(LPCWSTR arguments);

// ----------------------------------------------------------------------
// Dialog Procedure
// ----------------------------------------------------------------------
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;

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
            MessageBox(hwndDlg, L"Telegram -> @AreYou_Lost", L"Retr0",
                MB_OK | MB_ICONINFORMATION);
            return TRUE;

        case IDC_BUTTON4:
        {
            WCHAR buffer[MAX_PATH]{};
            GetDlgItemText(hwndDlg, IDC_EDIT1, buffer, MAX_PATH);
            ChangeArguments(buffer);
            return TRUE;
        }

        case IDC_BUTTON5:
            ClearDebugFlags();
            return TRUE;

        case IDOK:
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------
// Change Command Line Arguments
// ----------------------------------------------------------------------
void ChangeArguments(LPCWSTR arguments)
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
    {
        _plugin_logputs("Failed to resolve PEB");
        return;
    }

    duint processParameters = 0;
    if (!DbgMemRead(peb + 0x20, &processParameters, sizeof(processParameters)))
    {
        _plugin_logputs("Failed to read ProcessParameters");
        return;
    }

    UNICODE_STRING cmd{};
    if (!DbgMemRead(processParameters + 0x70, &cmd, sizeof(cmd)))
    {
        _plugin_logputs("Failed to read CommandLine UNICODE_STRING");
        return;
    }

    SIZE_T newLen = (wcslen(arguments) + 1) * sizeof(WCHAR);
    if (newLen > cmd.MaximumLength)
    {
        _plugin_logputs("New CommandLine exceeds MaximumLength");
        return;
    }

    if (!DbgMemWrite((duint)cmd.Buffer, arguments, newLen))
    {
        _plugin_logputs("Failed to write new CommandLine buffer");
        return;
    }

    cmd.Length = static_cast<USHORT>(newLen - sizeof(WCHAR));
    DbgMemWrite(processParameters + 0x70, &cmd, sizeof(cmd));

    MessageBox(nullptr, L"CommandLine updated successfully",
        L"CmdLine", MB_OK | MB_ICONINFORMATION);
}

// ----------------------------------------------------------------------
// Plugin Init
// ----------------------------------------------------------------------
DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->sdkVersion = PLUG_SDKVERSION;
    initStruct->pluginVersion = PLUGIN_VERSION;

    g_PluginHandle = initStruct->pluginHandle;
    strcpy_s(initStruct->pluginName, PLUGIN_NAME);

    return true;
}

// ----------------------------------------------------------------------
// Show ASLR / DEP / CFG
// ----------------------------------------------------------------------
void ShowASLRandDEP()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
    {
        _plugin_logputs("Failed to resolve PEB");
        return;
    }

    duint imageBase = 0;
    if (!DbgMemRead(peb + 0x10, &imageBase, sizeof(imageBase)))
    {
        _plugin_logputs("Failed to read ImageBase");
        return;
    }

    IMAGE_DOS_HEADER dos{};
    if (!DbgMemRead(imageBase, &dos, sizeof(dos)))
    {
        _plugin_logputs("Failed to read DOS header");
        return;
    }

    IMAGE_NT_HEADERS64 nt{};
    if (!DbgMemRead(imageBase + dos.e_lfanew, &nt, sizeof(nt)) ||
        nt.Signature != IMAGE_NT_SIGNATURE)
    {
        _plugin_logputs("Invalid NT header");
        return;
    }

    WORD flags = nt.OptionalHeader.DllCharacteristics;

    WCHAR buffer[MAX_PATH]{};
    swprintf_s(
        buffer,
        L"DllCharacteristics: 0x%04X\n\n"
        L"ASLR: %s\nDEP: %s\nCFG: %s",
        flags,
        (flags & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) ? L"ENABLED" : L"DISABLED",
        (flags & IMAGE_DLLCHARACTERISTICS_NX_COMPAT) ? L"ENABLED" : L"DISABLED",
        (flags & IMAGE_DLLCHARACTERISTICS_GUARD_CF) ? L"ENABLED" : L"DISABLED");

    MessageBox(nullptr, buffer, L"PE Security Flags",
        MB_OK | MB_ICONINFORMATION);
}

// ----------------------------------------------------------------------
// Clear Debug Flags
// ----------------------------------------------------------------------
bool ClearDebugFlags()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
    {
        _plugin_logputs("Failed to resolve PEB");
        return false;
    }

    BYTE  beingDebugged = 0;
    DWORD ntGlobalFlag = 0;

    DbgMemWrite(peb + 0x02, &beingDebugged, sizeof(beingDebugged));
    DbgMemWrite(peb + 0xBC, &ntGlobalFlag, sizeof(ntGlobalFlag));

    _plugin_logputs("PEB debug flags cleared");
    return true;
}

// ----------------------------------------------------------------------
// Show Command Line
// ----------------------------------------------------------------------
void ShowCmdLine()
{
    duint peb = DbgValFromString("peb()");
    if (!peb)
        return;

    duint processParameters = 0;
    if (!DbgMemRead(peb + 0x20, &processParameters, sizeof(processParameters)))
        return;

    UNICODE_STRING cmd{};
    if (!DbgMemRead(processParameters + 0x70, &cmd, sizeof(cmd)))
        return;

    if (!cmd.Length || !cmd.Buffer)
        return;

    PWCHAR buffer = (PWCHAR)malloc(cmd.Length + sizeof(WCHAR));
    if (!buffer)
        return;

    ZeroMemory(buffer, cmd.Length + sizeof(WCHAR));
    DbgMemRead((duint)cmd.Buffer, buffer, cmd.Length);

    MessageBox(nullptr, buffer, L"CommandLine",
        MB_OK | MB_ICONINFORMATION);

    free(buffer);
}

// ----------------------------------------------------------------------
// Menu Callback
// ----------------------------------------------------------------------
void cbMenuEntry(CBTYPE, PVOID info)
{
    auto menu = reinterpret_cast<PLUG_CB_MENUENTRY*>(info);

    switch (menu->hEntry)
    {
    case MENU_PEB_MANIPULATOR:
        if (ClearDebugFlags())
            MessageBox(nullptr, L"PEB Manipulated!", L"Success",
                MB_OK | MB_ICONINFORMATION);
        break;

    case MENU_ARGUMENTS:
        ShowCmdLine();
        break;

    case MENU_ASLR_DEP:
        ShowASLRandDEP();
        break;

    case MENU_DIALOG:
        DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1),
            GuiGetWindowHandle(), DialogProc);
        break;

    case MENU_ABOUT:
        MessageBox(nullptr, L"Telegram -> @AreYou_Lost",
            L"Retr0", MB_OK | MB_ICONINFORMATION);
        break;
    }
}

// ----------------------------------------------------------------------
// Plugin Setup
// ----------------------------------------------------------------------
DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    g_MenuHandle = _plugin_menuadd(setupStruct->hMenu, "PE/PE+ Manipulator");

    _plugin_menuaddentry(g_MenuHandle, MENU_PEB_MANIPULATOR, "Hide Debugger");
    _plugin_menuaddseparator(g_MenuHandle);
    _plugin_menuaddentry(g_MenuHandle, MENU_ARGUMENTS, "Show CommandLine");
    _plugin_menuaddseparator(g_MenuHandle);
    _plugin_menuaddentry(g_MenuHandle, MENU_ASLR_DEP, "ASLR / DEP");
    _plugin_menuaddseparator(g_MenuHandle);
    _plugin_menuaddentry(g_MenuHandle, MENU_DIALOG, "Dialog");
    _plugin_menuaddseparator(g_MenuHandle);
    _plugin_menuaddentry(g_MenuHandle, MENU_ABOUT, "About");

    _plugin_registercallback(g_PluginHandle, CB_MENUENTRY, cbMenuEntry);
}

// ----------------------------------------------------------------------
DLL_EXPORT void plugstop()
{
    _plugin_menuclear(g_PluginHandle);
}

// ----------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        g_hInstance = hInst;

    return TRUE;
}
