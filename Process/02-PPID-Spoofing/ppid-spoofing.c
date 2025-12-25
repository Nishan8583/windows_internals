#include <windows.h>
#include <stdio.h>

int main() {

    // 1. First the get the handle that is going to be the parent 
    DWORD target_ppid = 0; // DWORD is unsigned long
    printf("[*] Enter Target Parent Process ID: ");
    scanf("%lu", &target_ppid);
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_PROCESS, // desired access level
        FALSE, // child can not inherit the handle
        target_ppid // Target Process ID
    );


    if (hProcess == NULL) {
        printf("[-] OpenProcess failed. GetLastError: %lu\n", GetLastError());
        return -1;
    }

    printf("[+] Handle to Process with PID %lu obtained: 0x%p\n", target_ppid, hProcess);

    // 2. Setup the attribute list for the parent process
    STARTUPINFOEXA siex  = {0};  // structure used for process creation with attributes
    PROCESS_INFORMATION pi = {0};
    SIZE_T size = 0;

    siex.StartupInfo.cb = sizeof(STARTUPINFOEXA);  // windows uses this to know the size of the structure
    InitializeProcThreadAttributeList(NULL, 1, 0, &size);  // Telling we are setting 1 attribute, not actually initializing memory yet

    // STARTUPINFOEX
    // ├── StartupInfo (base info)
    // └── lpAttributeList
    //      └── PROC_THREAD_ATTRIBUTE_PARENT_PROCESS
    //           └── HANDLE → EPROCESS
    // The below code is creating the container
    siex.lpAttributeList= (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(),
        0,
        size
    );

    // This sets up internal headers
    InitializeProcThreadAttributeList(
        siex.lpAttributeList,
        1,
        0,
        &size
    );

    // This actually sets the parent process attribute
    // PROC_THREAD_ATTRIBUTE_PARENT_PROCESS
    UpdateProcThreadAttribute(
        siex.lpAttributeList,
        0,
        PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, // DWORD_PTR Attribute
        &hProcess,  // PVOID lpValue,
        sizeof(HANDLE),
        NULL,
        NULL
    );

    
     // 4. Create process with spoofed PPID by setting the attribute
    BOOL success = CreateProcessA(
        "C:\\Windows\\System32\\notepad.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &siex.StartupInfo,
        &pi
    );
}


/*
Pattern
InitializeSecurityDescriptor

GetTokenInformation

NtQueryInformationProcess

RegQueryValueEx

CryptDecodeObject

CreateProcess (user)
  ↓
kernel32.dll
  ↓
ntdll!NtCreateUserProcess
  ↓
kernel creates EPROCESS
  ↓
EPROCESS.InheritedFromUniqueProcessId = CurrentProcessId

*/