## 1. DLL search order hijacking?

> Attacker places a malicious DLL in the application directory  
> User executes a legitimate binary  
> The binary loads the attacker’s DLL

This is classically called:

- **DLL Search Order Hijacking**  
    (also called _DLL planting_ or _binary planting_ in older literature)
    

It exploits the fact that:

- The loader searches certain directories **before** the intended system DLL location
    
- The application calls `LoadLibrary("foo.dll")` without a full path
    

Common real-world cases:

- App directory hijacking
    
- Current directory hijacking
    
- `%PATH%` hijacking
    

---

## 2. Why this still works even with Safe DLL Search Mode

Safe DLL Search Mode **does NOT remove the application directory** from the search path.

Search order still starts with:

1. **Application directory**
    
2. System32
    
3. …
    

So if:

- A legitimate EXE is copied to a user-writable directory
    
- Or the app itself lives in a writable directory
    

👉 A malicious DLL with the same name **wins**.

---

## 3. Is `SetCurrentDirectory()` a defense?

❌ **No — and relying on it is dangerous.**

Why:

- The **application directory is still searched first**
    
- `SetCurrentDirectory()` only affects where _current directory_ appears in the list
    
- Many DLL loads happen:
    
    - Before your code runs
        
    - In third-party libraries
        
    - During static imports (before `main`)
        

So even if you do:

`SetCurrentDirectory(L"C:\\Windows\\System32");`

You are **not protected** against:

- Application-directory DLL hijacking
    
- Early-load DLL hijacking
    
- Dependencies loaded by other DLLs
    

---

## 4. Correct defenses (what Windows actually recommends)

### ✅ 1. Use `LoadLibraryEx` with explicit search flags

Best practice for **dynamic loads**:

`LoadLibraryEx(     L"foo.dll",     NULL,     LOAD_LIBRARY_SEARCH_SYSTEM32 );`

Or restrict explicitly:

`LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32`

Even better:

`SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);`

This removes:

- Current directory
    
- `%PATH%`  
    from DLL resolution entirely.
    

---

### ✅ 2. Always use **full paths** for DLLs you control

`LoadLibrary(L"C:\\Program Files\\MyApp\\foo.dll");`

This bypasses search order entirely.

---

### ✅ 3. Enable **Prefer System32 Images** mitigation

- Prevents application directory from beating `System32`
    
- Especially important for elevated or auto-elevated processes
    

This is a **process mitigation**, not an API call.

---

### ✅ 4. Use manifests / SxS (Fusion)

- Explicitly bind to versions of runtime libraries
    
- Prevents accidental DLL resolution
    

---

### ✅ 5. For modern apps: packaged / MSIX model

- No traditional DLL search path
    
- Only package dependency graph is used
    
- DLL hijacking is largely eliminated
    

---

## 5. Threat-hunting / detection angle (important for you)

Since you’re studying **Windows Internals + threat hunting**, this matters:

Red flags:

- EXE launched from:
    
    - `%TEMP%`
        
    - Downloads
        
    - User-writable directories
        
- DLL loaded from:
    
    - Same directory as EXE
        
    - Not signed
        
- Sysmon Event ID 7:
    
    - ImageLoaded
        
    - DLL path ≠ expected system location
        

Common attacker TTP:

- Copy trusted EXE (e.g., `notepad.exe`) to user dir
    
- Drop malicious `version.dll`, `winmm.dll`, etc.
    
- Execute → code execution under victim’s context (or elevated via UAC)
    

---

## 6. One-sentence takeaway

> **DLL search order hijacking occurs when an application loads a DLL by name and the attacker controls an earlier directory in the search order; `SetCurrentDirectory()` is not a reliable defense — proper mitigation requires restricted DLL search flags, full paths, or process-level mitigations.**