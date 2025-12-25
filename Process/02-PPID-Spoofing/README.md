This is a **very good question** — and this is exactly where _Windows Internals_ finally clicks with _real attacker behavior_.

I’ll explain this **precisely**, without hand-waving, and map **each concept → kernel structure → your PPID spoofing code**.

---

# First: what PPID actually is (ground truth)

On Windows, **PPID is not a user-mode concept**.

- PPID is stored in **kernel memory**
    
- Specifically in the **EPROCESS structure**
    
- Field (conceptually):
    

`EPROCESS.InheritedFromUniqueProcessId`

⚠️ **The PEB is NOT involved in PPID spoofing**

Let’s kill that misconception right now.

---

# Structures involved (very important)

|Structure|Mode|Role in PPID|
|---|---|---|
|**EPROCESS**|Kernel|Stores the _official_ parent PID|
|**PEB**|User|Has nothing to do with PPID|
|**ETHREAD**|Kernel|Thread context, not PPID|
|**CSR_PROCESS**|User/Subsystem|Mirrors process info for Win32|
|**Sysmon / ETW**|User|Reads PPID reported by kernel|

👉 **Only EPROCESS matters**

---

# Normal process creation flow (no spoofing)

Let’s start with **baseline behavior**.

### You call:

`CreateProcess(...)`

### Internally:

`CreateProcess (user)   ↓ kernel32.dll   ↓ ntdll!NtCreateUserProcess   ↓ kernel creates EPROCESS   ↓ EPROCESS.InheritedFromUniqueProcessId = CurrentProcessId`

So by default:

`Parent = calling process`

---

# PPID spoofing flow (what you are doing)

Now the **spoofed case**, step by step.

---

## Step 1: You open a handle to a _different_ process

### Code:

`hParent = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, targetPid);`

### What this does:

- You obtain a **kernel handle** to an existing process
    
- That handle internally points to an **EPROCESS**
    

🔑 This is critical:

> You are not passing a PID — you are passing a **handle to an EPROCESS object**

---

## Step 2: You tell Windows “use THIS process as parent”

### Code:

`UpdateProcThreadAttribute(     siex.lpAttributeList,     0,     PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,     &hParent,     sizeof(HANDLE),     NULL,     NULL );`

### What this actually means:

- You populate a **process creation attribute**
    
- This attribute is consumed by `NtCreateUserProcess`
    
- It overrides the default parent selection logic
    

Internally:

`ParentProcess = EPROCESS referenced by hParent`

---

## Step 3: Process creation enters the kernel

### Code:

`CreateProcessA(... EXTENDED_STARTUPINFO_PRESENT ...)`

### Internally:

`kernel32 → ntdll → NtCreateUserProcess`

Now the kernel does:

`new_eprocess = PsCreateProcess(...) new_eprocess->InheritedFromUniqueProcessId =     ParentEPROCESS->UniqueProcessId`

🔥 **This is the moment PPID is set**

No patching.  
No post-creation modification.  
No cheating.

It’s set **correctly**, just with a different parent.

---

# What is NOT touched (important)

❌ **PEB**

- Not modified
    
- No PPID field
    
- Only loader / heap / module info
    

❌ **User-mode memory**

- No memory injection
    
- No structure patching
    

❌ **EPROCESS after creation**

- Nothing is edited later
    
- No DKOM
    

This is why:

- Sysmon trusts it
    
- EDRs struggle
    
- It’s not inherently malicious
    

---

# Why Sysmon shows the spoofed PPID

Sysmon does:

`PsSetCreateProcessNotifyRoutine   ↓ Reads EPROCESS fields   ↓ Logs ParentProcessId`

Sysmon **cannot know intent** — it logs what the kernel reports.

So you get:

`ParentImage: explorer.exe`

Even though:

`Creator ≠ explorer.exe`

---

# Mapping: flow → code → structure

|Step|Code|Kernel object|
|---|---|---|
|Select parent|`OpenProcess`|Existing EPROCESS|
|Override parent|`PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`|Attribute list|
|Create process|`CreateProcess`|NtCreateUserProcess|
|PPID set|(kernel)|`EPROCESS.InheritedFromUniqueProcessId`|
|Logged|Sysmon|Reads EPROCESS|

---

# Why Windows Internals doesn’t scream “PPID spoofing”

Because from Windows’ perspective:

- This is **by design**
    
- Used by debuggers
    
- Used by job managers
    
- Used by sandboxing frameworks
    

It’s not an exploit — it’s **policy abuse**

---

# Detection insight (this is key for you)

Since **EPROCESS is truthful**, detection must use:

- Timing anomalies
    
- Parent-child behavior mismatch
    
- Token / integrity mismatch
    
- Handle ancestry
    
- ETW stack correlation
    

❌ Not kernel structure inspection  
❌ Not WinDbg labs

You were **100% right** to shift focus.

---

# One sentence takeaway (remember this)

> **PPID spoofing doesn’t modify Windows — it politely asks Windows to lie for it.**

# Can we update EPROCEESS structure ourselves ?
Let’s be **very precise**, because this distinction matters a lot in Windows security.

---

## Short answer (clear and strict)

> **You cannot legally or directly modify an existing `EPROCESS` structure from user mode.**

What you _can_ do is:

> **Influence how a _new_ `EPROCESS` is initialized at creation time.**

PPID spoofing is the second case — **not** the first.

---

## Why direct modification is “not allowed”

### `EPROCESS` facts

- Lives in **kernel memory**
    
- Accessible only in **kernel mode**
    
- Protected by:
    
    - Kernel privilege boundary
        
    - PatchGuard
        
    - Object manager rules
        

From **user mode**:

- ❌ You cannot write to `EPROCESS`
    
- ❌ You cannot change PPID after creation
    
- ❌ You cannot “edit” kernel objects
    

Doing so would require:

- A kernel driver
    
- A vulnerability exploit
    
- DKOM (rootkit territory)
    

---

## So how does PPID spoofing work then?

This is the key insight:

> **PPID spoofing does NOT modify an existing `EPROCESS`.**

Instead, it **controls the inputs** to process creation.

---

## The legal path (what you are doing)

### Timeline view

`(no process yet)     ↓ User provides attributes     ↓ Kernel creates EPROCESS     ↓ Fields initialized ONCE`

The kernel sets:

`EPROCESS.InheritedFromUniqueProcessId`

based on:

- Default: calling process
    
- Override: `PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`
    

You’re saying:

> “When you create the new process, please treat _this_ process as the parent.”

Windows says:

> “Okay.”

No rules are broken.

---

## Analogy (this makes it click)

Think of `EPROCESS` like a birth certificate.

- ❌ You cannot alter someone’s birth certificate after birth
    
- ✅ You can choose who is listed as the parent _before it’s issued_
    

PPID spoofing happens **before the certificate exists**.

---

## Why PatchGuard doesn’t care

PatchGuard protects:

- Kernel code
    
- Kernel structures **after initialization**
    

It does **not** block:

- Legitimate process creation parameters
    
- Documented APIs
    
- Normal object initialization
    

This is why:

- Malware can do it
    
- Debuggers can do it
    
- Windows itself does it
    

---

## What _would_ be illegal modification

For contrast, these are **not allowed** without kernel exploitation:

- Changing PPID after creation
    
- Editing `EPROCESS->Token`
    
- Hiding process by unlinking lists
    
- DKOM manipulation
    

That’s **rootkit behavior**.

---

## Why this distinction matters for threat hunting

|Technique|Detection surface|
|---|---|
|PPID spoofing|Behavioral correlation|
|DKOM|Kernel integrity / PatchGuard|
|Token theft|Access violations / log anomalies|
|Process hollowing|Memory + image mismatch|

PPID spoofing lives **entirely in user mode**.

---

## Final mental model (lock this in)

> **PPID spoofing doesn’t tamper with Windows.  
> It supplies misleading—but valid—inputs to Windows.**

Once you internalize this, Windows Internals suddenly makes a lot more sense.