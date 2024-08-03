#ifndef INJECTORGUI_H
#define INJECTORGUI_H

#include <windows.h>
#include <commdlg.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

#include "Resource.h"

// Control IDs
#define IDC_TEXTBOX_PATH 101
#define IDC_BUTTON_BROWSE 102
#define IDC_COMBOBOX_PROCESSES 103
#define IDC_BUTTON_ACTION 104

// Function Declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnInjectButtonClick(HWND hwnd);
void LoadProcesses(HWND hwndComboBox);
void BrowseForDLL(HWND hwnd, HWND hTextBoxPath);
bool IsDLLFile(const char* filePath);
bool IsProcessRunning(const char* processName);
DWORD GetProcId(const char* procName);
bool InjectDLL(const char* dllPath, const char* procName);
bool IsSystemProcess(const std::string& processName);

// Extern Declarations
extern const std::vector<std::string> systemProcesses;

#endif // INJECTORGUI_H
