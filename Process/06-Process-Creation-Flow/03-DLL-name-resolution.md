
## DLL Name Resolution

### What DLL name resolution is

- Process of mapping a **DLL base name** (no full path) to a **physical file**.
    
- Applies to:
    
    - Implicit binary dependencies
        
    - `LoadLibrary` calls without full paths
        
- Necessary because directory locations cannot be hardcoded at link time.
    

### Default DLL search path (Safe DLL Search Mode – enabled by default)

Safe DLL search mode mitigates **binary planting attacks** by deprioritizing the current directory.

Search order:
1. The directory from which the application was launched 
2. The native Windows system directory (for example, C:\Windows\System32) 
3. The 16-bit Windows system directory (for example, C:\Windows\System) 
4. The Windows directory (for example, C:\Windows) 
5. The current directory at application launch time 
6. Any directories specified by the %PATH% environment variable 

> The search path is **recomputed for every DLL load**.

### Security concern: Binary planting

- Without safe search mode, attackers could place a malicious DLL in the current directory.
    
- Even with safe mode:
    - DLL search order hijacking.
    - If a **system binary is copied** to a user-writable directory and executed from there,
    - A malicious DLL in the same directory may still load first.
        
- Mitigation:
    
    - **Prefer System32 Images** process-mitigation policy
        
    - Swaps order so `System32` is searched before the application directory.
        

---

## Modifying DLL Search Behavior

### Process-wide modifications

- `%PATH%` → `SetEnvironmentVariable`
    
- Current directory → `SetCurrentDirectory`
    
- DLL directory override → `SetDllDirectory`
    
    - Replaces the current directory
        
    - Disables safe DLL search mode for the process
        

### LoadLibraryEx flags

- `LOAD_WITH_ALTERED_SEARCH_PATH`
    
    - Uses the DLL’s directory instead of the application directory
        
    - **Unsafe if relative paths are used**
        
    - Ignored for Desktop Bridge apps
        

### Safer alternatives (recommended)

- `LOAD_LIBRARY_SEARCH_*` flags:
    
    - `APPLICATION_DIR`
        
    - `SYSTEM32`
        
    - `DLL_LOAD_DIR`
        
    - `USER_DIRS`
        
- Can be combined (e.g., `LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`)
    
- Can be enforced globally via `SetDefaultDllDirectories`
    

---

## Packaged Application DLL Resolution (UWP / MSIX)

- Traditional DLL search paths are **disabled**
    
- Uses **package dependency graph** instead
    
- Controlled by `<PackageDependency>` in the app manifest
    
- Prevents loading arbitrary DLLs
    
- `LoadPackagedLibrary` enforces this model
    
- Application-controlled DLL search APIs are ignored (except Desktop Bridge apps)
    

---

## DLL Name Redirection (Applied _Before_ Path Search)

Redirection extends or overrides the DLL namespace.

### 1. API Set (MinWin) Redirection

- Maps logical API contracts to physical DLLs
    
- Allows Windows to change internal implementations transparently
    
- Used heavily by modern Windows and Universal CRT
    

### 2. `.LOCAL` Redirection

- Forces DLL loads to resolve to application-local copies
    
- Mechanisms:
    
    - `MyDll.dll.local` file
        
    - `.LOCAL` directory containing the DLL
        
- Applies **even when full paths are specified**
    
- Disabled if the executable has a manifest
    
- Disabled by default
    
- Can be globally enabled via:
    
    - `HKLM\...\Image File Execution Options`
        
    - `DevOverrideEnable = 1`
        
- Treated similarly to SxS redirection
    

### 3. Fusion (Side-by-Side / SxS) Redirection

- Uses embedded **manifests** to specify versioned dependencies
    
- Introduced to allow multiple versions of system components (e.g., `comctl32.dll`)
    
- Uses **activation contexts**:
    
    - System-level
        
    - Process-level
        
    - Per-thread activation context stack
        
- Lookup order:
    
    1. Thread activation context
        
    2. Process activation context
        
    3. System activation context
        
- Activation contexts are managed:
    
    - Explicitly (`ActivateActCtx`, `DeactivateActCtx`)
        
    - Implicitly (e.g., during `DllMain`)
        

### 4. Known DLL Redirection

- Maps specific DLL names to trusted system directory files
    
- Prevents replacement or hijacking via alternate locations
    

---

## Architecture Mismatch Edge Case

- If a located DLL has the **wrong architecture** (e.g., 64-bit DLL for 32-bit app):
    
    - Loader **ignores the error**
        
    - Continues searching the next path entry
        
- Designed to allow mixed 32-bit and 64-bit entries in `%PATH%`