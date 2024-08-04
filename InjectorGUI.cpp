#include "InjectorGUI.h"

// List of known system processes to exclude
const std::vector<std::string> systemProcesses = {
    "Registry", "System", "smss.exe", "csrss.exe", "wininit.exe", "services.exe",
    "lsass.exe", "svchost.exe", "winlogon.exe", "explorer.exe", "spoolsv.exe",
    "taskhost.exe", "dwm.exe", "audiodg.exe", "taskeng.exe", "sppsvc.exe",
    "wlanext.exe", "conhost.exe", "SearchIndexer.exe", "dllhost.exe", "[System Process]",
    "Secure System", "fontdrvhost.exe", "System Idle Process", "Idle", "System Interrupts",
    "Interrupts", "RuntimeBroker.exe", "sihost.exe", "taskhostw.exe", "ctfmon.exe",
    "ShellExperienceHost.exe", "SearchUI.exe", "StartMenuExperienceHost.exe",
    "SystemSettings.exe", "TextInputHost.exe", "vmms.exe", "LsaIso.exe", "Memory Compression"
};

bool IsSystemProcess(const std::string& processName)
{
    return std::find(systemProcesses.begin(), systemProcesses.end(), processName) != systemProcesses.end();
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASSA wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Injector",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // Non-resizable window style
        CW_USEDEFAULT, CW_USEDEFAULT, 380, 140,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hLabelPath, hTextBoxPath, hButtonBrowse, hLabelProcesses, hComboBoxProcesses, hButtonAction;

    switch (uMsg)
    {
    case WM_CREATE:
        hLabelPath = CreateWindowA("STATIC", "DLL", WS_CHILD | WS_VISIBLE,
            10, 10, 60, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        hTextBoxPath = CreateWindowExA(0, "EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
            80, 10, 190, 20, hwnd, (HMENU)IDC_TEXTBOX_PATH, GetModuleHandle(NULL), NULL);

        hButtonBrowse = CreateWindowA("BUTTON", "Browse", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            280, 10, 75, 23, hwnd, (HMENU)IDC_BUTTON_BROWSE, GetModuleHandle(NULL), NULL);

        hLabelProcesses = CreateWindowA("STATIC", "Process", WS_CHILD | WS_VISIBLE,
            10, 40, 60, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        hComboBoxProcesses = CreateWindowA("COMBOBOX", NULL, CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
            80, 40, 275, 200, hwnd, (HMENU)IDC_COMBOBOX_PROCESSES, GetModuleHandle(NULL), NULL);

        hButtonAction = CreateWindowA("BUTTON", "Inject", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 70, 345, 23, hwnd, (HMENU)IDC_BUTTON_ACTION, GetModuleHandle(NULL), NULL);

        LoadProcesses(hComboBoxProcesses);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_BROWSE:
            BrowseForDLL(hwnd, hTextBoxPath);
            break;
        case IDC_BUTTON_ACTION:
            OnInjectButtonClick(hwnd);
            break;
        case IDC_COMBOBOX_PROCESSES:
            if (HIWORD(wParam) == CBN_DROPDOWN)
            {
                SendMessage(hComboBoxProcesses, CB_RESETCONTENT, 0, 0);
                LoadProcesses(hComboBoxProcesses);
            }
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void LoadProcesses(HWND hwndComboBox)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            std::string processName = pe32.szExeFile;
            if (!IsSystemProcess(processName))
            {
                SendMessageA(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)processName.c_str());
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}

void OnInjectButtonClick(HWND hwnd)
{
    char dllPath[MAX_PATH];
    GetWindowTextA(GetDlgItem(hwnd, IDC_TEXTBOX_PATH), dllPath, MAX_PATH);

    if (!IsDLLFile(dllPath))
    {
        MessageBoxA(hwnd, "The selected file is not a .dll.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char processName[MAX_PATH];
    GetWindowTextA(GetDlgItem(hwnd, IDC_COMBOBOX_PROCESSES), processName, MAX_PATH);

    if (strlen(processName) == 0)
    {
        MessageBoxA(hwnd, "No process selected or typed.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (!IsProcessRunning(processName))
    {
        MessageBoxA(hwnd, "The selected process is not running.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (InjectDLL(dllPath, processName))
    {
        MessageBoxA(hwnd, "DLL successfully injected.", "Info", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBoxA(hwnd, "DLL injection failed.", "Error", MB_OK | MB_ICONERROR);
    }
}

void BrowseForDLL(HWND hwnd, HWND hTextBoxPath)
{
    OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA) };
    char szFile[MAX_PATH] = { 0 };

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "DLL Files\0*.dll\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        SetWindowTextA(hTextBoxPath, ofn.lpstrFile);
    }
}

bool IsDLLFile(const char* filePath)
{
    const char* extension = strrchr(filePath, '.');
    return extension && _stricmp(extension, ".dll") == 0;
}

bool IsProcessRunning(const char* processName)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    bool processFound = false;
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processFound = true;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return processFound;
}

DWORD GetProcId(const char* procName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32 procEntry = { sizeof(PROCESSENTRY32) };
    DWORD procId = 0;
    if (Process32First(hSnap, &procEntry))
    {
        do
        {
            if (_stricmp(procEntry.szExeFile, procName) == 0)
            {
                procId = procEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &procEntry));
    }

    CloseHandle(hSnap);
    return procId;
}

bool InjectDLL(const char* dllPath, const char* procName)
{
    DWORD procId = GetProcId(procName);
    if (!procId)
    {
        return false;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProc)
    {
        return false;
    }

    void* loc = VirtualAllocEx(hProc, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!loc)
    {
        CloseHandle(hProc);
        return false;
    }

    if (!WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, NULL))
    {
        VirtualFreeEx(hProc, loc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, NULL);
    if (!hThread)
    {
        VirtualFreeEx(hProc, loc, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    CloseHandle(hThread);
    CloseHandle(hProc);
    return true;
}
