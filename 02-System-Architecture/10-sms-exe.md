# smss.exe

#  Simple Overview
## Short answer (TL;DR)

✅ **Yes** — **one SMSS instance exits**, while **another SMSS instance waits forever**.  
They are **different SMSS processes**, created at different times, for different purposes.

---

## The important distinction: _which_ Smss.exe?

There are **two kinds of SMSS instances**:

### 1️⃣ **The master (original) SMSS.exe**

- Started **very early during system boot**
    
- Runs in **session 0**
    
- **Does NOT exit**
    
- **Waits forever** on the session-0 `Csrss.exe` handle
    
- Marked **critical**
    
- If this SMSS dies → **BSOD**
    

👉 This is the one referred to by:

> “Smss.exe waits forever on the handle to the session 0 instance of Csrss.exe”

---

### 2️⃣ **Intermediate / per-session SMSS instances**

- Created **by the master SMSS**
    
- One for:
    
    - Session 0 initialization
        
    - Session 1 (interactive)
        
    - Possibly more (pre-created sessions)
        
- Their job is **session setup only**
    

They:

- Create:
    
    - `Csrss.exe` (Windows subsystem)
        
    - `Wininit.exe` (session 0) **or**
        
    - `Winlogon.exe` (interactive sessions)
        
- Then **exit intentionally**
    

👉 This is what the book means by:

> “this intermediate Smss.exe process exits”

---

## Timeline (simplified)

```
Boot
 └─ Kernel
     └─ smss.exe (MASTER, Session 0)
         ├─ smss.exe (Session 0 initializer) ──> exits
         │    └─ wininit.exe
         │    └─ csrss.exe
         │
         ├─ smss.exe (Session 1 initializer) ──> exits
         │    └─ winlogon.exe
         │    └─ csrss.exe
         │
         └─ waits forever on session-0 csrss.exe

```

---

## Why do Winlogon / Wininit appear parent-less?

Because:

- The **intermediate SMSS that created them exits**
    
- Windows does **not reparent** processes
    
- Result: they appear with **no parent PID**
    

This is:

- ✅ **Expected**
    
- ❌ **Not suspicious**
    
- Frequently misunderstood in threat hunting
    

---

## Why keep the master SMSS alive?

Security & stability reasons:

- SMSS is:
    
    - Critical
        
    - Protected
        
    - Minimal
        
- Waiting on `Csrss.exe` guarantees:
    
    - If the Windows subsystem dies → **system crash**
        
    - Prevents partial system corruption
        

This is a **deliberate fail-fast design**.

---

# Security Perspective

## smss.exe — Security-Critical Points

### 1. **Critical Process & Thread**

- `smss.exe` marks **itself and its initial thread as critical**.
    
- **Any termination ⇒ immediate system crash (BSOD)**.
    
- **Security implication**:
    
    - Protects system integrity very early in boot.
        
    - Attackers cannot safely tamper with or kill `smss.exe`.
        
    - Crashes involving `smss.exe` strongly suggest **kernel-mode bugs or rootkits**.
        

---

### 2. **Strict Error Handling + Mitigations**

- Treats:
    
    - Invalid handle use
        
    - Heap corruption  
        as **fatal errors**.
        
- Enables **Disable Dynamic Code Execution (D-DCE)**.
    
- **Security implication**:
    
    - Strong protection against:
        
        - Code injection
            
        - Runtime shellcode
            
        - Heap exploitation
            
    - `smss.exe` is **not a viable injection target**.
        

---

### 3. **Early Privilege & Priority**

- Base priority raised to **11** (above normal).
    
- **Security implication**:
    
    - Ensures SMSS work cannot be starved.
        
    - Prevents early user-mode DoS attacks.
        

---

### 4. **ALPC Control Surface (\SmApiPort)**

- Creates ALPC port `\SmApiPort`.
    
- Used for **session creation and system coordination**.
    
- **Security implication**:
    
    - Highly sensitive IPC endpoint.
        
    - Access is restricted by security descriptors.
        
    - Abuse attempts indicate **privilege escalation or kernel exploit activity**.
        

---

### 5. **Security Descriptor Initialization (ProtectionMode)**

- Reads:
    
    `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\ProtectionMode`
    
- Creates **security descriptors** for system objects.
    
- **Security implication**:
    
    - Misconfiguration could weaken object protections.
        
    - Tampering requires **offline registry or kernel access**.
        

---

### 6. **Object Manager Namespace Creation**

- Creates protected directories:
    
    - `\`
        
    - `\Windows`
        
    - `\RPC Control`
        
- **Security implication**:
    
    - Prevents **object squatting attacks**.
        
    - Ensures attackers can’t pre-create fake system objects.
        

---

### 7. **BootExecute / SetupExecute – High-Value Persistence**

- Executes binaries from:
    
    `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager   - BootExecute  - BootExecuteNoPnpSync  - SetupExecute`
    
- Default: `Autochk.exe`
    
- **Security implication (VERY IMPORTANT)**:
    
    - Executes **before most security software**
        
    - Requires **admin or offline access**
        
    - Popular target for:
        
        - Bootkits
            
        - Rootkits
            
        - Stealth persistence
            
- **High-confidence malicious signal if modified**.
    

---

### 8. **Pending File Rename Operations**

- Processes:
    
    - `PendingFileRenameOperations`
        
    - `PendingFileRenameOperations2`
        
- Used for **replace-on-reboot** behavior.
    
- **Security implication**:
    
    - Legitimate use: patching protected files
        
    - Malicious use:
        
        - Replace system binaries on reboot
            
        - Bypass file-in-use protections
            
    - Strong persistence indicator.
        

---

### 9. **KnownDlls Mechanism**

- Creates:
    
    - `\KnownDlls`
        
    - `\KnownDlls32`
        
- Maps Known DLLs as **permanent sections**.
    
- Reads:
    
    - `KnownDlls`
        
    - `ExcludeFromKnownDllList`
        
- **Security implication**:
    
    - Prevents DLL search order hijacking for core DLLs.
        
    - Tampering requires **SYSTEM / kernel access**.
        
    - Changes are rare and suspicious.
        

---

### 10. **Device Namespace & DOS Devices**

- Creates symbolic links under:
    
    `\Global??`
    
- Based on:
    
    `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\DOS Devices`
    
- **Security implication**:
    
    - Controls device access mappings.
        
    - Abuse can redirect device access (advanced attacks).
        

---

### 11. **Protected Named Pipe & Mailslot Prefixes**

- Prevents **pre-creation spoofing**.
    
- **Security implication**:
    
    - Blocks a classic race attack where malware creates  
        service IPC endpoints before the real service starts.
        

---

### 12. **WPBT (Windows Platform Binary Table) Execution**

- Executes firmware-registered binary **unless disabled**.
    
- Binary:
    
    - Runs extremely early
        
    - Must be **native (Ntdll-only)**.
        
- **Security implication (EXTREMELY SENSITIVE)**:
    
    - Firmware-level persistence
        
    - Survives OS reinstalls
        
    - Used by:
        
        - Anti-theft software
            
        - Nation-state-grade malware
            
- Any unexpected WPBT execution is **high severity**.
    

---

### 13. **Session Creation with Explicit Session IDs**

- Uses `PROCESS_CREATE_NEW_SESSION`.
    
- Triggers kernel:
    
    - `MiSessionCreate`
        
- Sets up:
    
    - Session space
        
    - Win32k mappings
        
- **Security implication**:
    
    - Correct session isolation
        
    - Bugs here historically led to **LPE vulnerabilities**.
        

---

### 14. **Critical Dependency on Csrss.exe**

- Waits on **session 0 `Csrss.exe`** forever.
    
- `Csrss.exe` is:
    
    - Critical
        
    - Protected
        
- **Security implication**:
    
    - Killing or corrupting `Csrss.exe` ⇒ system crash
        
    - Malware cannot safely interfere here.
        

---

### 15. **Parent-less SYSTEM Processes**

- `Wininit` / `Winlogon` and subsystem processes:
    
    - Have **no parent**
        
- **Security implication**:
    
    - Normal behavior
        
    - Parent-less SYSTEM processes are **not suspicious** here
        
    - Helps avoid false positives in threat hunting
        

---

## 🔴 High-Value Threat-Hunting Signals

Focus on these for detections:

|Area|Why|
|---|---|
|BootExecute / SetupExecute|Early persistence|
|PendingFileRenameOperations|Replace system binaries|
|WPBT execution|Firmware persistence|
|KnownDlls changes|DLL hijack attempts|
|ALPC \SmApiPort abuse|Privilege escalation|
|Smss / Csrss crashes|Kernel compromise|

---

If you want next, I can:

- Build a **boot-time attack chain timeline**
    
- Map these to **Sysmon + ETW providers**
    
- Explain **how real malware abuses SMSS paths**
    
- Create a **1-page SMSS security cheat sheet**

---

## Key takeaway (exam / interview quality)

> **Smss.exe exists in multiple instances**:
> 
> - **One master instance** persists forever and enforces system integrity
>     
> - **Per-session instances** initialize sessions and then exit, leaving child  
>     processes parent-less
>

---

# Details
The master Smss.exe performs the following one-time initialization steps:
1. It marks the process and the initial thread as critical. If a process or thread marked critical exits for any reason, Windows crashes. See Chapter 3 for more information.

2. It causes the process to treat certain errors as critical, such as invalid handle usage and heap corruption, and enables the Disable Dynamic Code Execution process mitigation.

3. It increases the process base priority to 11.

4. If the system supports hot processor add, it enables automatic processor affinity updates. That way, if new processors are added, new sessions will take advantage of the new processors. For more information about dynamic processor additions, see Chapter 4.

5. It initializes a thread pool to handle ALPC commands and other work items.

6. It creates an ALPC port named \SmApiPort to receive commands.

7. It initializes a local copy of the NUMA topology of the system.

8. It creates a mutex named PendingRenameMutex to synchronize file-rename operations.

9. It creates the initial process environment block and updates the Safe Mode variable if needed.

10. Based on the ProtectionMode value in the HKLM\SYSTEM\CurrentControlSet\Control\Session Manager key, it creates the security descriptors that will be used for various system resources.

11. Based on the ObjectDirectories value in the HKLM\SYSTEM\CurrentControlSet\Control\Session Manager key, it creates the object manager directories that are described, such as \
RPC Control and \Windows. It also saves the programs listed under the values BootExecute, 
BootExecuteNoPnpSync, and SetupExecute.
12. It saves the program path listed in the S0InitialCommand value under the HKLM\SYSTEM\ 
CurrentControlSet\Control\Session Manager key
13. It reads the NumberOfInitialSessions value from the HKLM\SYSTEM\CurrentControlSet\
Control\Session Manager key, but ignores it if the system is in manufacturing mode.
14. It reads the file rename operations listed under the PendingFileRenameOperations and 
PendingFileRenameOperations2 values from the HKLM\SYSTEM\CurrentControlSet\Control\/
Session Manager key.
15. It reads the values of the AllowProtectedRenames, ClearTempFiles, TempFileDirectory, 
and DisableWpbtExecution values in the HKLM\SYSTEM\CurrentControlSet\Control\Session 
Manager key.
16. It reads the list of DLLs in the ExcludeFromKnownDllList value found under the HKLM\SYSTEM\ 
CurrentControlSet\Control\Session Manager key.
17. It reads the paging file information stored in the HKLM\SYSTEM\CurrentControlSet\Control\
Session Manager\Memory Management key, such as the PagingFiles and ExistingPageFiles 
list values and the PagefileOnOsVolume and WaitForPagingFiles configuration values.
18. It reads and saves the values stored in the HKLM\SYSTEM\CurrentControlSet\Control\Session 
Manager\ DOS Devices key.
19. It reads and saves the KnownDlls value list stored in the HKLM\SYSTEM\CurrentControlSet\
Control\Session Manager key.
20. It creates system-wide environment variables as defined in HKLM\SYSTEM\CurrentControlSet\
Control\Session Manager\Environment. 
21. It creates the \KnownDlls directory, as well as \KnownDlls32 on 64-bit systems with WoW64.
22. It creates symbolic links for devices defined in HKLM\SYSTEM\CurrentControlSet\Control\Ses
sion Manager\DOS Devices under the \Global?? directory in the object manager namespace.
23. It creates a root \Sessions directory in the object manager namespace.
24. It creates protected mailslot and named pipe prefixes to protect service applications from spoof
ing attacks that could occur if a malicious user-mode application executes before a service does.
25. It runs the programs part of the BootExecute and BootExecuteNoPnpSync lists parsed earlier. 
(The default is Autochk.exe, which performs a disk check.)
26. It initializes the rest of the registry (HKLM software, SAM, and security hives).
27. Unless disabled by the registry, it executes the Windows Platform Binary Table (WPBT) binary 
registered in the respective ACPI table. This is often used by anti-theft vendors to force the 
execution of a very early native Windows binary that can call home or set up other services for 
execution, even on a freshly installed system. These processes must link with Ntdll.dll only (that 
is, belong to the native subsystem).
28. It processes pending file renames as specified in the registry keys seen earlier unless this is a 
Windows Recovery Environment boot. 
93
CHAPTER 2 System architecture 
29. It initializes paging file(s) and dedicated dump file information based on the HKLM\System\
CurrentControlSet\Control\Session Manager\Memory Management and HKLM\System\Cur
rentControlSet\Control\CrashControl keys.
30. It checks the system’s compatibility with memory cooling technology, used on NUMA systems.
31. It saves the old paging file, creates the dedicated crash dump file, and creates new paging files 
as needed based on previous crash information.
32. It creates additional dynamic environment variables, such as PROCESSOR_ARCHITECTURE, 
PROCESSOR_LEVEL, PROCESSOR_IDENTIFIER, and PROCESSOR_REVISION, which are based on 
registry settings and system information queried from the kernel.
33. It runs the programs in HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\SetupEx
ecute. The rules for these executables are the same as for BootExecute in step 11.
34. It creates an unnamed section object that is shared by child processes (for example, Csrss.exe) 
for information exchanged with Smss.exe. The handle to this section is passed to child processes 
via handle inheritance. For more on handle inheritance, see Chapter 8 in Part 2.
35. It opens known DLLs and maps them as permanent sections (mapped files) except those listed 
as exclusions in the earlier registry checks (none listed by default).
36. It creates a thread to respond to session create requests.
37. It creates the Smss.exe instance to initialize session 0 (non-interactive session).
38. It creates the Smss.exe instance to initialize session 1 (interactive session) and, if configured 
in the registry, creates additional Smss.exe instances for extra interactive sessions to prepare 
itself in advance for potential future user logons. When Smss.exe creates these instances, it 
requests the explicit creation of a new session ID using the PROCESS_CREATE_NEW_SESSION flag 
in NtCreateUserProcess each time. This has the effect of calling the internal memory manager 
function MiSessionCreate, which creates the required kernel-mode session data structures 
(such as the Session object) and sets up the Session Space virtual address range that is used 
by the kernel-mode part of the Windows subsystem (Win32k.sys) and other session-space 
device drivers. 

After these steps have been completed, Smss.exe waits forever on the handle to the session 0 instance of `Csrss.exe`. Because `Csrss.exe` is marked as a critical process (and is also a protected process; see Chapter 3), if Csrss.exe exits, this wait will never complete because the system will crash.
A session startup instance of `Smss.exe` does the following:
- It creates the subsystem process(es) for the session (by default, the Windows subsystem `Csrss.exe`).
- It creates an instance of `Winlogon` (interactive sessions) or the Session 0 Initial Command, which is Wininit (for session 0) by default unless modified by the registry values seen in the preceding steps. See the upcoming paragraphs for more information on these two processes.
94 CHAPTER 2 System architecture

Finally, this intermediate Smss.exe process exits, leaving the subsystem processes and Winlogon or 
Wininit as parent-less processes.