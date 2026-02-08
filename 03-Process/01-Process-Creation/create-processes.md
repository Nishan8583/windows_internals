## Process Creation on Windows

![alt_text](./images/01.png)

### Classic (Desktop) Process Creation

- **CreateProcess**: Simplest API; creates a process using the caller’s access token.
    
- **CreateProcessAsUser**: Allows specifying a different user token (e.g., from `LogonUser`).
    
- **CreateProcessWithTokenW / CreateProcessWithLogonW**:
    
    - Require different privileges than `CreateProcessAsUser`.
        
    - Use the **Secondary Logon (SecLogon)** service via RPC.
        
    - Internally call `CreateProcessAsUser`.
        
    - Fail if the SecLogon service is disabled.
        
    - Used by the `runas` command.
        

---

### Modern / UWP (Immersive) Process Creation

- Modern apps (UWP / immersive apps) require more than `CreateProcess`.
    
- Requirements include:
    
    - Specific command-line arguments.
        
    - An undocumented process attribute:
        
        - `PROC_THREAD_ATTRIBUTE_PACKAGE_FULL_NAME`
            
- Alternative supported approach:
    
    - Use `IApplicationActivationManager` (COM API).
        
    - Call `ActivateApplication` with an **AppUserModelId**.
        
    - Obtain AppUserModelId via `GetPackageApplicationIds`.
        

---

### Non-Standard Process Types

- Some processes bypass Win32 APIs entirely:
    
    - **Native processes** (e.g., `Smss.exe`)
        
    - **Minimal processes** (e.g., System, Memory Compression)
        
    - **Pico processes** (e.g., WSL)
        
- Key system calls:
    
    - `NtCreateUserProcess`: User-mode process creation.
        
    - `NtCreateProcessEx`: Kernel-mode / minimal process creation.
        
- Helpers:
    
    - `RtlCreateUserProcess`: Ntdll wrapper for native processes.
        
    - `PspCreatePicoProcess`: Internal helper for Pico providers.
        
- All creation paths converge on internal routines:
    
    - `PspAllocateProcess`
        
    - `PspInsertProcess`
        

---

## Process Internals

### Core Data Structures

- **EPROCESS**: Primary kernel structure representing a process.
    
    - Lives in system address space.
        
    - Points to many related structures.
        
- **ETHREAD**: Represents threads within a process.
    
- **PEB (Process Environment Block)**:
    
    - Lives in user address space.
        
    - Accessible by user-mode code.
        
- Some memory-management structures are process-context specific.
    

---

### Subsystem-Specific Structures

- **CSR_PROCESS**: Maintained by `Csrss` for Win32 processes.
    
- **W32PROCESS**: Created by `Win32k.sys` when USER/GDI APIs are first used.
    
- **DXGPROCESS**: Maintained by `Dxgkrnl.sys` for DirectX/GPU resources.
    

---

### Object Manager & Handles

- Each process (except Idle) is wrapped as a **Process object**.
    
- Process objects are unnamed (not visible in WinObj).
    
- Handles expose controlled access to process internals via APIs.
    

---

### Extensibility & Security

- Drivers can register process-creation callbacks:
    
    - `PsSetCreateProcessNotifyRoutine(Ex)`
        
- Used for:
    
    - Tracking per-process data.
        
    - Blocking or filtering process creation (e.g., anti-malware).
        

---

### Kernel vs Executive Abstraction

- **EPROCESS** embeds a **KPROCESS (PCB)** as its first member.
    
- Kernel components (scheduler, dispatcher) use **KPROCESS**.
    
- Executive components use **EPROCESS**.
    
- This separation enforces clean layering and abstraction.



## CreateProcess* Function Arguments

- **Process Token / Credentials**
    
    - `CreateProcessAsUser`, `CreateProcessWithTokenW`: require a **token handle** for the new process.
        
    - `CreateProcessWithLogonW`: requires **username, domain, and password**.
        
- **Executable & Command Line**
    
    - Full path to the executable.
        
    - Command-line arguments passed to the new process.
        
- **Security Attributes**
    
    - Optional security descriptors for:
        
        - The **process object**
            
        - The **initial thread object**
            
- **Handle Inheritance**
    
    - Boolean flag controlling whether **inheritable handles** from the parent process are copied to the child.
        
- **Creation Flags**
    
    - Modify how the process is created.
        
    - Common examples:
        
        - `CREATE_SUSPENDED`: initial thread starts suspended (resume with `ResumeThread`).
            
        - `DEBUG_PROCESS`: parent acts as a debugger for the new process.
            
        - `EXTENDED_STARTUPINFO_PRESENT`: indicates use of `STARTUPINFOEX`.
            
- **Environment Block**
    
    - Optional custom environment variables.
        
    - If omitted, inherits the parent process environment.
        
- **Current Directory**
    
    - Optional working directory for the new process.
        
    - Used for relative path resolution (e.g., DLL loading).
        
    - Defaults to the parent’s current directory.
        
- **Startup Information**
    
    - `STARTUPINFO`: basic startup configuration.
        
    - `STARTUPINFOEX`: extended version supporting:
        
        - Process/thread attributes (key–value pairs).
            
        - Attributes set via `UpdateProcThreadAttributes`.
            
        - Includes undocumented attributes (e.g., for UWP/store apps).
            
- **Output: PROCESS_INFORMATION**
    
    - Returned on success.
        
    - Contains:
        
        - New **Process ID**
            
        - New **Thread ID**
            
        - Handle to the **process**
            
        - Handle to the **initial thread**
            
    - Handles allow post-creation manipulation (resume, debug, inspect, etc.).
        

---

### Notes

- A user-mode created process **always starts with exactly one thread**.
    
- That initial thread eventually executes the program’s `main` / entry point.