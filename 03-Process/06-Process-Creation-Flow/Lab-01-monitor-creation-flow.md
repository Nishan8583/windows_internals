
## Part 1: General Steps to Observe Normal Process Creation Flow

These steps apply to _any_ simple process (Notepad, calc, whoami, custom EXE).

### 1. Choose a Parent and Child Process

- Pick a **simple parent** (e.g., `cmd.exe`, `powershell.exe`, `explorer.exe`)
    
- Pick a **simple child** (e.g., `notepad.exe`)
    
- Ensure **no existing instances** are running
    

Purpose:

> Lets you clearly separate _parent-side_ process creation work from _child-side_ initialization.

---

### 2. Configure ProcMon Filters (Critical)

Filter **INCLUDE only**:

- `Process Name is cmd.exe`
    
- `Process Name is notepad.exe`
    

Exclude everything else.

Why this matters:

> Process creation noise is enormous. You want _causal_ events only.

---

### 3. Start Parent Process with Capture OFF

- Disable event capture
    
- Launch `cmd.exe`
    

Why:

> You want a clean boundary between “idle” and “process creation.”

---

### 4. Enable Capture → Launch Child

- Enable capture
    
- Run `notepad.exe`
    
- Let it fully initialize
    
- Stop capture
    

Expected:

- Hundreds to a few thousand events (500–3500 is normal)
    

---

### 5. Reduce Visual Noise

- Hide **Sequence** and **Time of Day**
    
- Focus on:
    
    - Operation
        
    - Path
        
    - Result
        
    - Detail
        
    - Stack (very important)
        

---

### 6. Identify the Major Phases in the Trace

You should mentally split the trace into **three phases**:

---

#### Phase A: Parent-Side (Pre-Creation, User Mode)

Events originate from **cmd.exe** before the process exists.

Look for:

- Registry reads
    
- Compatibility checks
    
- Setup logic
    

---

#### Phase B: Kernel Process Creation

Events show **transition into kernel mode**.

Look for:

- `NtCreateUserProcess`
    
- Thread + process object creation
    
- Initial image mapping
    

---

#### Phase C: Child-Side Initialization

Events now originate from **notepad.exe**.

Look for:

- DLL loads
    
- Prefetch activity
    
- Loader initialization
    
- Eventually → app code (`WinMain`)
    

---

## Part 2: Normal / Expected Behavior You Should Mentally Ignore

This is the **baseline**. Seeing these does **not** mean anything malicious by itself.

---

## 1. Image File Execution Options (IFEO) Check

**What you’ll see**

`RegOpenKey HKLM\Software\Microsoft\Windows NT\CurrentVersion\ Image File Execution Options\notepad.exe`

**Why it happens**

- Windows checks:
    
    - Debugger settings
        
    - Global flags
        
    - Compatibility options
        

**Normal behavior**

- Key exists or doesn’t
    
- Usually returns `NAME NOT FOUND`
    

**APIs involved**

- `CreateProcessInternalW`
    
- `NtCreateUserProcess`
    
- `PspAllocateProcess`
    

**Ignore unless**

- A `Debugger` value exists
    
- Unexpected redirection occurs
    

---

## 2. Application Compatibility (Shim Engine) Checks

**What you’ll see**

- Reads from:
    
    - `AppCompat`
        
    - `.sdb` files
        
- Registry access related to compatibility flags
    

**Why**

- Windows decides whether to apply shims
    

**Normal**

- `.sdb` files opened
    
- No shim actually applied
    

**Ignore unless**

- Custom shim DBs
    
- Non-Microsoft app unexpectedly shimmed
    

---

## 3. Side-by-Side (SxS), Manifest, and MUI Reads

**What you’ll see**

- Reads to:
    
    - `WinSxS`
        
    - Manifest paths
        
    - Language / MUI registry keys
        

**Why**

- Determines:
    
    - Visual styles
        
    - CRT versions
        
    - Localization
        

**Normal**

- Lots of registry + file reads
    
- Often repetitive
    

**Ignore unless**

- Abnormal SxS redirection
    
- Missing / tampered manifests
    

---

## 4. Initial DLL Load Notifications

**What you’ll see**

- Image load events for:
    
    - `notepad.exe`
        
    - `ntdll.dll`
        

**Why**

- First thread enters user mode
    
- Loader initializes process context
    

**Normal APIs**

- `LdrInitializeThunk`
    
- `LdrpInitializeProcess`
    

**Key insight**

> This is the **first point** where code executes _inside the new process_.

---

## 5. Prefetcher Activity

**What you’ll see**

- Access to:
    
    - `\Windows\Prefetch\NOTEPAD.EXE-*.pf`
        
- Early reads of DLLs _before_ standard loading
    

**Why**

- Performance optimization
    
- Predicts future DLL loads
    

**Normal**

- DLLs loaded earlier than expected
    
- Appears “out of order”
    

**Ignore unless**

- Prefetch disabled unexpectedly
    
- Prefetch accessing strange binaries
    

---

## 6. Standard DLL Loading (Loader Phase)

**What you’ll see**

- Image loads for:
    
    - `kernel32.dll`
        
    - `user32.dll`
        
    - `gdi32.dll`
        
    - `advapi32.dll`
        
    - etc.
        

**Normal behavior**

- Loader walks import tables
    
- Loads dependencies
    
- Resolves imports
    

**Normal APIs**

- `LdrLoadDll`
    
- `LdrpMapDll`
    
- `LdrpResolveImports`
    

**Ignore**

> This is _pure boilerplate_ for almost all Windows programs.

---

## 7. Transition to Application Code

**What you’ll see**

- Events originating from:
    
    - `WinMain`
        
    - Application-specific registry or file access
        

**This is the key boundary**

> Everything before this = Windows doing its job  
> Everything after this = developer (or attacker) code

---

## Part 3: Practical Mental Model for Threat Hunting

### You should mostly ignore:

- IFEO checks (unless debugger set)
    
- AppCompat / SxS / MUI reads
    
- Prefetch activity
    
- Standard DLL loads
    
- `ntdll → kernel32 → user32` chains
    

---

### You should focus on:

- **Unusual parent processes**
    
- **Nonstandard IFEO entries**
    
- **DLL loads from writable directories**
    
- **Early network activity**
    
- **Process hollowing indicators**
    
- **Code executing before expected loader stages**
    
- **Unexpected child processes during startup**