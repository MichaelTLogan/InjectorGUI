#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const char* procName) {
    DWORD procId = 0;                                               //initialize variable
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //Open the Handle and create snapshot of processes
    if (hSnap != INVALID_HANDLE_VALUE) {                            //error checking - make sure we have a good snapshot
        PROCESSENTRY32 procEntry;                                   //create process entry
        procEntry.dwSize = sizeof(procEntry);                       
        if (Process32First(hSnap, &procEntry)) {                    //procEntry receives first value from Process32First
            do {                                                    //loop start
                if (!_stricmp(procEntry.szExeFile, procName)) {     //_stricmp string insensitive compare - if you put wrong capitalization you can still find your process.
                    procId = procEntry.th32ProcessID;               //if we've found our process then set procId to its ID and break
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));             //loop will continue as long as there's processes in the snapshot still.
        }
    }
    CloseHandle(hSnap);                                             //close the handle to avoid memory leaks and other bad stuffs
    return procId;                                                  //return our value;
}

int main()
{
    const char* dllPath = "D:\\micha\\Documents\\GH\\First DLL Hack\\Debug\\First DLL Hack.dll";      //self explanatory
    std::cout << dllPath << std::endl;
    const char* procName = "ac_client.exe";
    DWORD procId = 0;

    while (!procId) {
        procId = GetProcId(procName);
        Sleep(30);
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);                                  //Calling OpenProcess with PROCESS_ALL_ACCESS but we only need READWRITE.

    if (hProc && hProc != INVALID_HANDLE_VALUE) {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);           //Allocates memory in an external process via the handle.
        if (loc)
            WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);                                //Write the DLL Path in memory at loc

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);  //Create the remotethread which calls LoadLibraryA and loads the DLL
        if (hThread) {
            CloseHandle(hThread);
        }
        if (hProc) {
            CloseHandle(hProc);
        }
    }
    return 0;
}