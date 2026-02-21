## Ntdll.dll — Role and Purpose

**Ntdll.dll** is a special, low-level user-mode system library used primarily by:

- Subsystem DLLs (Kernel32, User32, etc.)
    
- Native images (not tied to any subsystem)
    

It is the **lowest common user-mode layer** above the kernel.

---

## Two Main Categories of Functions in Ntdll.dll

### 1. System Service Dispatch Stubs

- Entry points for **Windows executive system services**
    
- Examples: `NtCreateFile`, `NtReadFile`, `NtWriteFile`, `NtSetEvent`
    
- Over **450 system services**
    
- Mostly undocumented for direct use
    
- Typically wrapped by Win32 APIs
    

**How they work:**

1. User-mode code calls `NtXxx` function in `ntdll.dll`
    
2. Function loads a **system service number** into `EAX`
    
3. Executes a **privileged transition**:
    
    - `syscall` (modern path)
        
    - or `int 2Eh` (legacy / VBS-optimized path)
        
4. CPU switches to kernel mode
    
5. Kernel dispatcher routes to real implementation in `Ntoskrnl.exe`
    

---

### 2. Internal Support Functions

Used by the OS and subsystem infrastructure, not applications directly.

Key groups:

- **Image loader** (`Ldr*`)
    
- **Heap manager**
    
- **Runtime library** (`Rtl*`)
    
- **Subsystem communication** (`Csr*`)
    
- **Debugging support** (`DbgUi*`)
    
- **ETW tracing** (`Etw*`)
    
- **APC dispatcher**
    
- **Exception dispatcher**
    
- Minimal **CRT routines** (`memcpy`, `sprintf`, etc.)
    

These allow native applications to function without Win32.

---

## Example: System Call Stub (NtCreateFile)

Typical x64 stub behavior:

- Load service index into `EAX`
    
- Move parameters into expected registers
    
- Execute `syscall`
    
- Return to user mode
    

Special detail:

- Checks a flag in **SharedUserData**
    
- If **Credential Guard (VBS)** is enabled:
    
    - Uses `int 2Eh` instead of `syscall`
        
    - Hypervisor handles it more efficiently
        

---

## Secure System Calls (VTL 1)

When **Virtualization-Based Security (VBS)** is enabled:

- VTL 1 user-mode code uses **IumDll.dll**
    
- Equivalent to `ntdll.dll` for **Isolated User Mode (IUM)**
    

Key differences:

- System call numbers have the **high bit set**
    
- Always use `syscall`
    
- No `SharedUserData` checks
    
- Calls are routed to the **secure kernel**
    
- Used by Trustlets and Credential Guard
    

---

## Native Images (Executables)

### What is a Native Image?

- Not tied to **any subsystem**
    
- Does **not** link against:
    
    - Kernel32.dll
        
    - User32.dll
        
    - Other Win32 DLLs
        
- Links **only to Ntdll.dll**
    
- Uses the **native NT API**
    

### Characteristics:

- Subsystem type in PE header: **Native**
    
- Mostly undocumented APIs
    
- Almost exclusively built by Microsoft
    

---

## Why Native Images Exist

Some processes must run **before subsystems exist**.

Examples:

- **Smss.exe (Session Manager)**
    
    - First user-mode process
        
    - Created directly by the kernel
        
    - Responsible for starting `Csrss.exe`
        
- **Autochk.exe**
    
    - Runs early during boot
        
    - Cannot rely on Win32 or GUI subsystems
        

These processes **must not depend** on the Windows subsystem.

---

## Key Takeaways

- **Ntdll.dll is the gateway** between user mode and the kernel
    
- All Win32 APIs eventually funnel down to **NtXxx system calls**
    
- Native images exist to bootstrap and manage the OS itself
    
- VBS introduces a **parallel system-call path** via `IumDll.dll`
    
- Without Ntdll.dll, **no user-mode process can function**
    

---

### Mental Model

> **Kernel32 / User32 = convenience APIs**  
> **Ntdll = raw system interface**  
> **Native images = OS bootstrap tools**

If you want next, I can:

- Trace a **CreateFile call end-to-end**
    
- Explain **Nt vs Zw differences (user vs kernel)**
    
- Tie **Ntdll hooks** to malware & EDR detection techniques