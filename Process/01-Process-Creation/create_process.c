#include <windows.h>
#include <stdio.h>

int main() {

    PROCESS_INFORMATION processInfo;
    STARTUPINFO startupInfo;
    
    ZeroMemory(&processInfo, sizeof(processInfo));
    ZeroMemory(&startupInfo, sizeof(startupInfo));

    int result = CreateProcess(
        NULL, // module name use commandline so can be set to NULL,
        "notepad.exe", // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        0, // No creation flags
        NULL, // Use parent's environment block
        NULL, // Use parent's starting directory
        &startupInfo, // Pointer to STARTUPINFO structure
        &processInfo // Pointer to PROCESS_INFORMATION structure
    );

    if (result == 0) {
        printf("Failed to create process. Error: %d\n", GetLastError());
        // Handle error
        return -1;
    }
    return 0;
}