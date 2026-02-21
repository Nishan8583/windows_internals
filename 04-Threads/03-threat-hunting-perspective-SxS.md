# 1️⃣ How Activation Context Looks Inside the TEB

## 📍 First: What is the TEB?

TEB = **Thread Environment Block**

- User-mode structure
    
- One per thread
    
- Pointer stored in:
    
    - x64 → `GS:[0x30]`
        
    - x86 → `FS:[0x18]`
        
- Contains:
    
    - Thread ID
        
    - Stack limits
        
    - TLS data
        
    - SEH chain
        
    - Activation context pointer
        

---

## 🔎 Where Activation Context Lives in the TEB

Inside the TEB, there is:

```
TEB
 ├── NtTib
 ├── ClientId
 ├── ProcessEnvironmentBlock (PEB)
 ├── ThreadLocalStoragePointer
 ├── ...
 ├── ActivationContextStackPointer  ← important

```
### The important field:

`TEB->ActivationContextStackPointer`

This points to:

`_ACTIVATION_CONTEXT_STACK`

---

## 🧱 Structure Conceptually

```
TEB
  ↓
ActivationContextStackPointer
  ↓
_ACTIVATION_CONTEXT_STACK
    ├── ActiveFrame
    ├── FrameListCache
    └── Flags

```

Each frame represents an active activation context:

```
_ACTIVATION_CONTEXT_STACK_FRAME
    ├── Previous
    ├── ActivationContext
    └── Flags

```

So think of it like:

```
Thread
   has
Activation Context Stack
   which contains
Frames
   which reference
Activation Context objects
```

---

## 🔹 What is an Activation Context Object?

Kernel object managed by:

- `RtlCreateActivationContext`
    
- `RtlActivateActivationContext`
    

It contains:

- Parsed manifest
    
- Assembly binding rules
    
- Redirection metadata
    
- COM registration redirection
    
- DLL version mapping
    

---

## 🧠 Why a Stack?

Because activation contexts can be:

- Activated temporarily
    
- Nested
    
- Deactivated later
    

Example:

```
Activate A
   Activate B
Deactivate B
Deactivate A
```

Very similar to structured exception handling stack behavior.

---

## 🔎 Debugging View

In WinDbg:

`!teb`

You’ll see the activation context pointer.

You can inspect:

```
dt _TEB
dt _ACTIVATION_CONTEXT_STACK
```

---

# 2️⃣ How Attackers Abuse SxS

This is where it gets interesting for you as a threat hunter.

SxS is abused mainly in **DLL hijacking and persistence scenarios**.

---

# 🔥 Attack Pattern 1 — Local DLL Redirection (Classic)

If an application has a manifest requesting:

`comctl32.dll version 6.0.0.0`

Windows will look in:

1. Application directory
    
2. WinSxS
    
3. System locations
    

If the app directory contains a malicious DLL that matches binding rules:

👉 It may get loaded.

This is called:

- **Side-by-side DLL hijacking**
    

---

# 🔥 Attack Pattern 2 — Fake Manifest Injection

An attacker can:

- Drop a malicious EXE
    
- Include a crafted manifest
    
- Redirect assembly resolution
    

Example abuse:

- Redirect a DLL to a malicious version
    
- Redirect COM class to malicious implementation
    

---

# 🔥 Attack Pattern 3 — WinSxS Directory Abuse

WinSxS is trusted.

Malware may:

- Hide payloads in WinSxS-like folder structures
    
- Mimic assembly naming format:
    
    `amd64_microsoft.windows.common-controls_6595b64144ccf1df_6.0.19041.1_none_xxxxx`
    

To evade naive detection rules.

---

# 🔥 Attack Pattern 4 — COM Redirection Abuse

SxS can redirect COM objects.

Instead of registry:

`HKCR\CLSID\...`

Manifest can say:

> Use this local DLL instead

So malware can:

- Override COM resolution
    
- Without touching registry
    

This is stealthier.

---

# 🔥 Attack Pattern 5 — UAC Bypass via SxS

Some UAC bypasses use:

- Auto-elevated binaries
    
- SxS DLL hijacking
    
- Malicious DLL placed next to system binary
    

Windows loads attacker DLL during elevated execution.

Very powerful technique.

---

# 🔎 Detection Ideas (Threat Hunting Angle)

You’d hunt for:

- Suspicious manifests
    
- Unusual DLL loads from:
    
    - Application directory instead of System32
        
- WinSxS folder anomalies
    
- `Load Image` events (Sysmon Event ID 7)
    
- DLL loaded where:
    
    - Path doesn’t match expected SxS resolution
        

---

# 🧠 Big Picture

Activation context:

Controls **how DLLs and COM objects are resolved**.

Attackers abuse:

The fact that Windows trusts:

- Manifests
    
- Local redirection
    
- SxS resolution logic
    

---

# 🔥 Mental Model

Without SxS:

`LoadLibrary("foo.dll") → System32`

With SxS:

```
LoadLibrary("foo.dll")
        ↓
Check Activation Context
        ↓
Check Manifest Rules
        ↓
Maybe Load Local Malicious DLL
```

That indirection layer is what attackers exploit.