#include "InjectorGUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
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

        hComboBoxProcesses = CreateWindowA("COMBOBOX", NULL, CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
            80, 40, 275, 200, hwnd, (HMENU)IDC_COMBOBOX_PROCESSES, GetModuleHandle(NULL), NULL);

        hButtonAction = CreateWindowA("BUTTON", "Inject", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 70, 345, 23, hwnd, (HMENU)IDC_BUTTON_ACTION, GetModuleHandle(NULL), NULL);

        LoadProcesses(hComboBoxProcesses);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_BROWSE)
        {
            BrowseForDLL(hwnd, hTextBoxPath);
        }
        else if (LOWORD(wParam) == IDC_BUTTON_ACTION)
        {
            OnInjectButtonClick(hwnd);
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
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            SendMessageA(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}

void OnInjectButtonClick(HWND hwnd)
{
    char dllPath[260];
    GetWindowTextA(GetDlgItem(hwnd, IDC_TEXTBOX_PATH), dllPath, 260);

    if (!IsDLLFile(dllPath))
    {
        MessageBoxA(hwnd, "The selected file is not a .dll.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    char processName[260];
    GetWindowTextA(GetDlgItem(hwnd, IDC_COMBOBOX_PROCESSES), processName, 260);

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

    // Here you can add the code to inject the DLL into the selected process
    MessageBoxA(hwnd, (std::string("Injecting ") + dllPath + " into " + processName).c_str(), "Info", MB_OK | MB_ICONINFORMATION);
}

void BrowseForDLL(HWND hwnd, HWND hTextBoxPath)
{
    OPENFILENAMEA ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE)
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
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return false;
    }

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