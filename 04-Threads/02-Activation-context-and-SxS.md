# 1️⃣ Is `PspCreateThread` ultimately called?

**Yes.**

Flow simplified:

```
CreateRemoteThreadEx (Kernel32 - user mode)
        ↓
NtCreateThreadEx (Ntdll - user mode)
        ↓
NtCreateThreadEx (Kernel - executive)
        ↓
PspCreateThread (Kernel internal)
```

Inside kernel mode:

- `NtCreateThreadEx` does setup and validation
    
- Then calls **`PspCreateThread`**
    
- `PspCreateThread`:
    
    - Allocates the executive thread object
        
    - Initializes:
        
        - ETHREAD
            
        - KTHREAD
            
    - Links thread to process
        
    - Sets initial state (usually suspended)
        

So yes — **`PspCreateThread` is the internal worker that actually creates the executive thread object.**

Think of it like:

> `NtCreateThreadEx` = public syscall entry  
> `PspCreateThread` = internal implementation

(`Psp` prefix = Process Support)

---

# 2️⃣ What is an "Activation Context"?

This is a **user-mode structure used by the Windows loader to manage DLL versioning and configuration.**

An activation context contains:

- Which DLL versions to load
    
- COM registration redirection
    
- Side-by-side assembly metadata
    
- Application manifest info
    
- Dependency resolution rules
    

It tells Windows:

> “When this thread loads DLL X, use this specific version and configuration.”

Each thread has:

- An **activation context stack**
    
- Pointer stored in the **TEB**
    

---

## Why per-thread?

Because activation contexts can be:

- Activated
    
- Deactivated
    
- Temporarily overridden
    

Different threads may operate under different assembly bindings.

---

# 3️⃣ What is Side-by-Side (SxS)?

SxS = **Side-by-Side Assemblies**

It was introduced to fix **DLL Hell**.

---

## 🔥 The Problem Before SxS (DLL Hell)

Suppose:

- App A needs `msvcrt.dll v1`
    
- App B needs `msvcrt.dll v2`
    

Without SxS:

- Only one version could live in `System32`
    
- Installing one app could break another
    

Classic Windows XP pain.

---

## ✅ SxS Solution

Windows allows:

- Multiple versions of the same DLL
    
- Stored in:
    
    `C:\Windows\WinSxS\`
    

Apps include a **manifest** specifying:

- Exact version
    
- Public key token
    
- Architecture
    
- Assembly identity
    

The loader uses activation contexts to resolve:

`Load "comctl32.dll"         ↓ Check activation context         ↓ Load correct version from WinSxS`

---

## 🔹 Example

Visual Studio apps might request:

- `Microsoft.VC90.CRT`
    
- Version 9.0.21022.8
    

That resolution is handled via:

- SxS
    
- Activation context
    
- Loader
    

---

# 🧠 Why Does Thread Creation Touch Activation Context?

When a thread starts:

- It may load DLLs
    
- It may create COM objects
    
- It may use manifests
    

So Windows:

1. Allocates activation context
    
2. Attaches it to thread (in TEB)
    
3. Ensures correct assembly binding from the start
    

---

# 🔎 From a Threat Hunting Perspective

Activation contexts matter because:

- DLL hijacking sometimes abuses manifest resolution
    
- Malware may:
    
    - Manipulate activation contexts
        
    - Use SxS redirection tricks
        
- Unusual WinSxS loads can be suspicious
    

---

# 🔥 Summary

### ✔ Yes — `PspCreateThread` is ultimately called.

### ✔ Activation Context:

A thread-specific structure that controls:

- DLL version binding
    
- COM redirection
    
- Manifest resolution
    

### ✔ SxS (Side-by-Side):

A Windows mechanism that allows:

- Multiple DLL versions
    
- Stored in WinSxS
    
- Resolved via activation contexts