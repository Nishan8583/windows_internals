#define INITGUID  // Seems like we need to do this to get the GUIDs defined here (global variable configuration stuff)

#include <windows.h>
#include <stdio.h>
#include <conio.h>  // for windows specific
#include <strsafe.h>  // StringCbCopyW
#include <wmistr.h>  // This is WMI + ETW infrastructure.  WNODE_HEADER, EVENT_TRACE_HEADER
#include <evntrace.h>  // EVENT_TRACE_PROPERTIES, StartTraceW, 

/* eventrace.h
#ifdef INITGUID
EXTERN_C const GUID SystemTraceControlGuid = { ... };
#else
EXTERN_C const GUID SystemTraceControlGuid;
#endif
*/

#define LOGFILE_PATH L"etwlogfile.etl" // This is unicode wide string,


// turns out wmain is the entry point for console application
void wmain(void) {

    ULONG status = ERROR_SUCCESS;  // unsigned long
    TRACEHANDLE SessionHandle = 0;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
    ULONG BufferSize = 0;

    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.
    
    // This buffer will not be big enough to close
    //BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(LOGFILE_PATH) + sizeof(KERNEL_LOGGER_NAME);
    BufferSize = sizeof(EVENT_TRACE_PROPERTIES)
           + 2 * MAX_PATH * sizeof(WCHAR);

    pSessionProperties = (EVENT_TRACE_PROPERTIES*) malloc(BufferSize);
    if (pSessionProperties == NULL) {
        wprintf(L"Unable to allocate %lu bytes for event trace properties\n",BufferSize);
        goto cleanup;
    }

    ZeroMemory(pSessionProperties,BufferSize);  // We need to zero this out because we allocated in heap later on

      pSessionProperties->Wnode.BufferSize = BufferSize;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    pSessionProperties->Wnode.Guid = SystemTraceControlGuid; 
    //pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;   // Enable TCP/IP events

    // For process activity monitoring
    pSessionProperties->EnableFlags =
        EVENT_TRACE_FLAG_PROCESS      // process start/exit
        | EVENT_TRACE_FLAG_THREAD       // thread start/exit
        | EVENT_TRACE_FLAG_IMAGE_LOAD;  // DLL / EXE loads

    pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR;
    pSessionProperties->MaximumFileSize = 5;  // 5 MB
    pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    pSessionProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME); 
    StringCbCopyW((LPWSTR)((BYTE*)pSessionProperties + pSessionProperties->LogFileNameOffset), sizeof(LOGFILE_PATH), LOGFILE_PATH);


    // Create the trace session
    status = StartTrace((PTRACEHANDLE)&SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties);
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_ALREADY_EXISTS) {
            wprintf(L"Trace session %s already exists.\n", KERNEL_LOGGER_NAME);
        } else {
            wprintf(L"StartTraceW failed with %lu\n", status);
        }
        goto cleanup;
    }


    wprintf(L"ETW Trace session started. Press any key to stop it.\n");
    _getch();  // wait for user input

cleanup:
    if (SessionHandle) {
        status = ControlTrace(SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_STOP);
        if (status != ERROR_SUCCESS) {
            wprintf(L"ControlTraceW to stop failed with %lu\n", status);
        }
    }

    if (pSessionProperties) {
        free(pSessionProperties);
    }

}