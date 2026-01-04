### **System Processes Overview**

- Some processes on Windows 10 are **not full processes** (minimal processes) as they don’t run a user-mode executable:
    
    - **Idle Process** – accounts for CPU idle time; one thread per logical CPU; no real executable.
        
    - **System Process** – hosts kernel-mode system threads.
        
    - **Secure System Process** – contains VTL 1 secure kernel address space if VBS is active.
        
    - **Memory Compression Process** – stores compressed memory pages for user-mode processes; runs system threads for memory management.
        
- **Other key system processes:**
    
    - `Smss.exe` – Session Manager, first user-mode process; creates sessions and child processes.
        
    - `Csrss.exe` – Windows Subsystem.
        
    - `Wininit.exe` – Session 0 initialization.
        
    - `Winlogon.exe` – Logon process for interactive sessions.
        
    - `Services.exe` – Service Control Manager (SCM), manages service processes (`Svchost.exe` children).
        
    - `Lsass.exe` – Local Security Authentication Service; `Lsaiso.exe` if Credential Guard is active.
        

---

### **Process Tree & Relationships**

- Parent/child relationships help understand process origin.
    
- Use **Process Monitor** (boot logging + process tree view) to see initial processes and those that have exited.
    

---

### **Idle Process**

- Process ID: 0.
    
- Names vary by utility: Task Manager (“System Idle Process”), Process Explorer (“System Idle Process”), Tlist (“System Process”).
    
- Does not execute code; accounts for CPU idle cycles.
    

---

### **System Process**

- Process ID: 4.
    
- Hosts **kernel-mode system threads**:
    
    - Threads run in kernel only; no user-mode address space.
        
    - Use OS memory pools for dynamic storage.
        
    - Created by `PsCreateSystemThread` or `IoCreateSystemThread`.
        
- Examples:
    
    - Memory manager threads (dirty page writes, swapping).
        
    - Cache manager (read-ahead, write-behind).
        
    - File server threads (network I/O).
        
    - Device driver threads (e.g., floppy polling).
        
- System threads can belong to other processes (e.g., `Win32k.sys` in `Csrss.exe`).
    

---

### **Secure System Process**

- Hosts VTL 1 secure kernel address space.
    
- Provides visual indication that **Virtualization-Based Security (VBS)** is active.
    
- No actual threads or handles since VTL 0 kernel manages scheduling/memory.
    

---

### **Memory Compression Process**

- Stores compressed pages evicted from working sets.
    
- Runs system threads (`SmKmStoreHelperWorker`, `SmStReadThread`).
    
- Uses user-mode address space → visible memory usage.
    
- Helps save memory by reducing paging.
    

---

### **Session Manager (`Smss.exe`)**

- First user-mode process; created by kernel thread.
    
- Creates multiple instances for session initialization (multi-session support).
- When Smss.exe starts, it checks whether it is the first instance (the master Smss.exe) or an instance of itself that the master Smss.exe launched to create a session. If command-line arguments are present, it is the latter.
- By creating multiple instances of itself during boot-up and Terminal Services session creation, Smss.exe can create multiple sessions at the same time—as many as four concurrent sessions, plus one more for each extra CPU beyond one
    
- Performs **one-time system initialization**:
    
    - Marks itself and main thread critical.
        
    - Handles process mitigations (heap corruption, invalid handles).
        
    - Creates ALPC ports, thread pools, mutexes, environment variables.
        
    - Initializes registry values (KnownDLLs, DOS Devices, BootExecute, SetupExecute, paging files).
        
    - Creates session-specific processes (`Csrss.exe`, `Wininit.exe`, `Winlogon.exe`) and shared section objects.
        
- Waits on session 0 `Csrss.exe`; if it terminates, system crashes.
    

---

### **Windows Initialization Process (`Wininit.exe`)**

- Performs further system initialization:
    
    - Critical process and thread marking.
        
    - Creates events (FirstLogonCheck, WinlogonLogoff).
        
    - Increases priority (process 13, thread 15).
        
    - Sets environment variables (COMPUTERNAME, USERPROFILE, etc.).
        
    - Initializes temp directory, font loading, Desktop Window Manager (DWM).
        
    - Initializes LSA machine key.
        
    - Starts `Services.exe` (SCM) and `Lsass.exe`/`Lsaiso.exe`.
        
    - Launches setup if pending.
        
    - Waits for shutdown or monitored processes to terminate.
        

---

### **Service Control Manager (`Services.exe`)**

- User-mode process that starts/stops/manages Windows services.
    
- Services:
    
    - Can run automatically at boot or manually.
        
    - Run in special accounts (SYSTEM, LOCAL SERVICE) or user accounts.
        
    - Defined in registry under `HKLM\SYSTEM\CurrentControlSet\Services`.
        
- Commands:
    
    - `tasklist /svc` or `tlist /s` to map service processes to services.
        
- Services can share a process (not always one-to-one).
    

---

### **Winlogon, LogonUI, Userinit**

- `Winlogon.exe` handles interactive logons/logoffs and SAS (Ctrl+Alt+Del).
    
- **LogonUI.exe** – launched by Winlogon; runs credential providers (password, smartcard, biometrics like Windows Hello).
    
- Credentials sent to `Lsass.exe` for authentication.
    
    - If Credential Guard enabled, interacts with `Lsaiso.exe`.
        
- `Userinit.exe` – initializes user environment, runs login scripts, restores network connections.
    
- Default shell (`Explorer.exe`) launched by `Userinit.exe` → parentless process.