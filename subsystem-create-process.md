# CreateProcess: tracing across subsystem boundaries

We’ll trace what happens when a **Win32 app** calls:

`CreateProcessW(...)`

---

## High-level picture (keep this in mind)

`Win32 App (user mode)   ↓ Win32 Subsystem DLLs   ↓ NT Native API (ntdll)   ↓   ← user → kernel transition NT Kernel (Executive)   ↓ Win32 Subsystem Server (csrss.exe)   ↓ Process starts running`

---

## Step 1: Win32 application (User mode)

Your program calls:

`CreateProcessW(...)`

This is **pure Win32 API**.

📍 You are fully inside the **Win32 subsystem** at this point.

---

## Step 2: kernel32.dll (Win32 subsystem client)

Flow:

`CreateProcessW   → CreateProcessInternalW`

What happens here:

- Parameter validation
    
- Command line parsing
    
- Environment block setup
    
- Path resolution
    
- Deciding flags (console / GUI / suspended)
    

📌 **Still user-mode**  
📌 **Still Win32 subsystem**

No kernel interaction yet.

---

## Step 3: Transition to NT Native API (ntdll.dll)

Eventually:

`kernel32 → ntdll`

Key calls:

- `NtCreateUserProcess`
    
- `NtCreateFile`
    
- `NtQueryInformationProcess`
    

📌 **This is the subsystem boundary**

- Win32 semantics → NT semantics
    
- Portable Win32 API → stable internal NT API
    

`ntdll.dll`:

- Is **shared by all subsystems**
    
- Is the **last user-mode layer**
    

---

## Step 4: User → Kernel transition

Inside `ntdll`:

`NtCreateUserProcess   → syscall / sysenter`

CPU switches:

- Ring 3 → Ring 0
    
- User stack → Kernel stack
    

📌 You have **left the Win32 subsystem**  
📌 You are now in the **NT kernel**

---

## Step 5: NT Kernel (Executive)

Kernel components involved:

### Executive managers:

- **Object Manager** – creates process & thread objects
    
- **Memory Manager** – address space, PEB, VADs
    
- **Security Reference Monitor** – access token
    
- **I/O Manager** – image file handling
    
- **Scheduler** – thread ready state
    

Important internal steps:

- Create **EPROCESS**
    
- Create **ETHREAD**
    
- Map executable image
    
- Create initial thread (but not started yet)
    

📌 Kernel does **not care** that this is Win32  
📌 Kernel sees “create process” — nothing more

---

## Step 6: Call into Win32 subsystem support (csrss / win32k)

Now something _subsystem-specific_ happens.

The kernel:

- Notifies the **Win32 subsystem** that a new process exists
    

This happens via:

- Internal callbacks
    
- Historically **ALPC** to `csrss.exe`
    

Win32-specific setup:

- Console association
    
- GUI initialization
    
- Thread message queues
    
- CSR structures
    

📌 This is where **Win32 behavior** is added  
📌 Kernel itself stays generic

---

## Step 7: csrss.exe (Win32 subsystem server)

`csrss.exe` runs in **user mode**, but is **critical**.

It handles:

- Console creation
    
- Process/thread bookkeeping
    
- DLL initialization notifications
    

📌 If `csrss.exe` dies → system crash  
📌 It is **not a service**

This is **subsystem server logic**, not kernel logic.

---

## Step 8: Final kernel work & thread start

Back in kernel:

- Initial thread context is finalized
    
- Thread is marked runnable
    
- Scheduler picks it
    

Entry point:

`ntdll!LdrInitializeThunk`

Which:

- Initializes loader
    
- Loads DLLs
    
- Calls CRT startup
    
- Eventually reaches `main()` or `WinMain()`
    

---

## Step 9: New process begins execution

You are now inside:

`main()`

🎉 Process creation complete.

---

# Where exactly are the subsystem boundaries?

Let’s mark them explicitly.

### Boundary 1: Win32 → NT API

`kernel32.dll → ntdll.dll`

### Boundary 2: User → Kernel

`ntdll.dll → NtCreateUserProcess (syscall)`

### Boundary 3: Kernel → Win32 subsystem server

`Kernel → csrss.exe / win32k.sys`

---

## Key takeaway (this is the exam answer)

>