
✅ Fix #1 (recommended): Explicitly build a console Unicode app

Compile like this:
 gcc main.c -municode -mconsole -o etw.exe

# What this program does (big picture)

This program:

1. **Creates a kernel ETW session** (the NT Kernel Logger)
    
2. Enables **TCP/IP network events**
    
3. Writes events into an **ETL file** (circular buffer, max 5 MB)
    
4. Waits for a keypress
    
5. **Stops the trace session** and cleans up
    

This is **producer-side ETW** (starting/stopping logging), _not_ consuming events.

---

# Why this is special

This is not a normal ETW session:

- It uses **`SystemTraceControlGuid`**
    
- That means:
    
    - You’re controlling the **kernel logger**
        
    - Requires **Administrator privileges**
        
    - Only one kernel logger can exist at a time
        

---

# Walkthrough (in order)

---

## 1. `#define INITGUID`

`#define INITGUID`

### Why this exists

Some GUIDs in Windows headers are declared like this:

`EXTERN_C const GUID SystemTraceControlGuid;`

If **INITGUID is defined** _before including the header_, the header actually **defines the GUID**, not just declares it.

Without this:

- You’ll get a **linker error**
    
- Because the symbol is declared but never defined
    

Think of it as:

> “I want the actual GUID object, not just a reference.”

---

## 2. Headers

`#include <windows.h> #include <stdio.h> #include <conio.h> #include <strsafe.h> #include <wmistr.h> #include <evntrace.h>`

Key ones:

- `evntrace.h` → ETW APIs (`StartTrace`, `ControlTrace`)
    
- `wmistr.h` → WMI / kernel tracing structures
    
- `strsafe.h` → safe string APIs (`StringCbCopyW`)
    

---

## 3. Log file path

`#define LOGFILE_PATH L"<FULLPATHTOTHELOGFILE.etl>"`

- Wide string (`WCHAR[]`)
    
- ETW expects **UTF-16**
    
- This becomes part of the trace session metadata
    

---

## 4. Entry point

`void wmain(void)`

- Unicode console entry point
    
- Equivalent to `int main()` but with wide strings
    
- Used by `wmainCRTStartup`
    

---

## 5. Local variables

`ULONG status = ERROR_SUCCESS; TRACEHANDLE SessionHandle = 0; EVENT_TRACE_PROPERTIES* pSessionProperties = NULL; ULONG BufferSize = 0;`

### What these are

|Variable|Meaning|
|---|---|
|`status`|Windows error codes|
|`SessionHandle`|Handle to ETW session|
|`pSessionProperties`|Configuration block|
|`BufferSize`|Size of allocated config block|

---

## 6. Why manual memory allocation is required

`BufferSize = sizeof(EVENT_TRACE_PROPERTIES)            + sizeof(LOGFILE_PATH)            + sizeof(KERNEL_LOGGER_NAME);`

### This is **very important**

`EVENT_TRACE_PROPERTIES` is a **variable-length structure**.

Memory layout:

`[ EVENT_TRACE_PROPERTIES ] [ Logger Name (WCHAR[]) ] [ Log File Name (WCHAR[]) ]`

Microsoft does **not** allocate this for you.  
You must provide **one contiguous block**.

---

## 7. Allocate memory

`pSessionProperties = malloc(BufferSize);`

- Raw heap allocation
    
- ETW APIs expect **caller-managed memory**
    
- Kernel will read this buffer during `StartTrace`
    

---

## 8. Initialize the structure

`ZeroMemory(pSessionProperties, BufferSize);`

ETW APIs:

- **Require zero-initialized memory**
    
- Uninitialized fields can cause `ERROR_INVALID_PARAMETER`
    

---

## 9. WNODE header (this is ETW plumbing)

`pSessionProperties->Wnode.BufferSize = BufferSize; pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; pSessionProperties->Wnode.ClientContext = 1; pSessionProperties->Wnode.Guid = SystemTraceControlGuid;`

### What this means

|Field|Meaning|
|---|---|
|`BufferSize`|Size of entire buffer|
|`Flags`|This is an ETW trace|
|`ClientContext = 1`|Timestamp = QPC|
|`Guid`|**Kernel logger control GUID**|

This is how ETW knows:

> “Oh — you want the NT kernel logger.”

---

## 10. Enable kernel events

`pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;`

This enables:

- TCP send/receive
    
- Connection events
    
- Network stack activity
    

Kernel flags are **bitmasks** — you can OR more later.

---

## 11. Log file behavior

`pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR; pSessionProperties->MaximumFileSize = 5;`

Meaning:

- Circular buffer ETL file
    
- Max 5 MB
    
- Old data overwritten when full
    

---

## 12. Offsets (critical part)

`pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES); pSessionProperties->LogFileNameOffset =     sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);`

These offsets tell ETW:

> “The strings are not pointers — they live **inside the same buffer** at these byte offsets.”

This avoids kernel/user pointer issues.

---

## 13. Copy the log file path

`StringCbCopyW(     (LPWSTR)((BYTE*)pSessionProperties + pSessionProperties->LogFileNameOffset),     sizeof(LOGFILE_PATH),     LOGFILE_PATH );`

### What’s happening

1. Cast to `BYTE*` → byte-accurate pointer math
    
2. Add offset
    
3. Cast to `LPWSTR`
    
4. Copy UTF-16 string safely
    

This writes the log file path **inside the allocated buffer**.

---

## 14. Start the trace session

`status = StartTrace(     &SessionHandle,     KERNEL_LOGGER_NAME,     pSessionProperties );`

This call:

- Talks to the kernel ETW subsystem
    
- Creates or attaches to the NT kernel logger
    
- Returns a session handle
    

If another kernel logger exists → `ERROR_ALREADY_EXISTS`

---

## 15. Wait for user input

`_getch();`

Keeps tracing alive until you press a key.

---

## 16. Stop the trace session

`ControlTrace(     SessionHandle,     KERNEL_LOGGER_NAME,     pSessionProperties,     EVENT_TRACE_CONTROL_STOP );`

This:

- Flushes buffers
    
- Closes ETL file
    
- Stops kernel logging
    

---

## 17. Cleanup

`free(pSessionProperties);`

Caller owns the memory → caller frees it.