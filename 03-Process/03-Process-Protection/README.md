# Windows Protected Processes & PPL — Summary Notes

## 1. Default Windows security model (baseline)

- Any process with **SeDebugPrivilege** (e.g., Administrator) can:
    
    - Read/write other process memory
        
    - Inject code
        
    - Suspend/resume threads
        
    - Query detailed process information
        
- Tools like **Process Explorer / Task Manager** rely on these rights.
    
- This model assumes **admins must control everything**.
    

---

## 2. Why protected processes exist

- DRM requirements (Blu-ray, premium media) **conflict with admin omnipotence**
    
- Windows Vista / Server 2008 introduced **Protected Processes (PP)** to:
    
    - Prevent memory inspection
        
    - Prevent code injection
        
    - Protect high-value content
        
- These processes coexist with normal ones but **restrict access even from admins**
    

---

## 3. Creation & trust requirements (classic Protected Process)

- Any app _can request_ protection
    
- OS grants protection **only if**:
    
    - Executable is signed with a **special Microsoft Media / DRM certificate**
        
- Used by:
    
    - **Protected Media Path (PMP)**
        
    - **Media Foundation (MF)** APIs
        

---

## 4. Examples of classic protected processes

- `audiodg.exe` — decodes protected audio
    
- `mfpmp.exe` — Media Foundation Protected Pipeline
    
- `werfaultsecure.exe` — crash handling for protected processes
    
- `System` process:
    
    - Stores sensitive crypto material (via `ksecdd.sys`)
        
    - Owns **all kernel handles**
        
    - Hosts kernel-mapped user-mode memory (CI data, certs)
        

---

## 5. Kernel enforcement mechanism

- Protection implemented via **flags in `EPROCESS`**
    
- Kernel denies access during:
    
    - Process handle creation
        
    - Thread operations
        
- Allowed process access rights (very limited):
    
    - `PROCESS_QUERY/SET_LIMITED_INFORMATION`
        
    - `PROCESS_TERMINATE` _(not for all PPLs)_
        
    - `PROCESS_SUSPEND_RESUME`
        
- Thread access rights are also restricted (covered later in Thread Internals)
    

---

## 6. Tool behavior (important observation)

- **User-mode tools** (Process Explorer):
    
    - ❌ Cannot enumerate DLLs
        
    - ❌ Cannot read memory
        
- **Kernel debuggers** (WinDbg kernel mode):
    
    - ✅ Can see full details
        
- This difference is **by design**, not a bug
    

---

## 7. Can admins bypass this?

- In theory:
    
    - A kernel driver could flip the `EPROCESS.Protection` flag
        
- In practice:
    
    - Violates PMP policy
        
    - Considered malicious
        
    - Blocked by:
        
        - Kernel Code Signing (x64)
            
        - PatchGuard
            
        - `peauth.sys`
            
- Even on x86:
    
    - PMP policy can halt playback
        
    - Microsoft can revoke the driver signature
        

---

# Protected Process Light (PPL)

## 8. Why PPL was introduced

- Introduced in **Windows 8.1 / Server 2012 R2**
    
- Extends protected processes beyond DRM:
    
    - Windows licensing
        
    - Store apps
        
    - Security components (LSASS, Defender)
        

---

## 9. What PPL adds

- Same core protection as PP:
    
    - No injection
        
    - No memory inspection
        
- **Additional dimension: Signer levels**
    
    - Determines _how protected_ a process is
        
    - Higher signer → more power
        

---

## 10. Signer hierarchy (high → low power)

- Protected processes **always outrank PPL**
    
- Higher signer can access lower signer
    
- Lower signer **cannot access higher signer**
    

### Signer levels (Table 3-2)

|Signer|Level|Used For|
|---|---|---|
|WinSystem|7|System & minimal processes|
|WinTcb|6|Critical Windows components|
|Windows|5|Sensitive Windows services|
|Lsa|4|LSASS|
|Antimalware|3|Defender / EDR|
|CodeGen|2|NGEN|
|Authenticode|1|DRM, fonts|
|None|0|Normal processes|

---

## 11. Valid `EPROCESS.Protection` values (Table 3-1)

- Stored as a **bitfield**
    
- Examples:
    
    - `PS_PROTECTED_WINTCB_LIGHT`
        
    - `PS_PROTECTED_LSA_LIGHT`
        
    - `PS_PROTECTED_ANTIMALWARE_LIGHT`
        
    - `PS_PROTECTED_NONE`
        

---

## 12. Why malware can’t fake protection

- Code Integrity validates:
    
    - Certificate EKUs
        
    - Issuer strings
        
- Special EKU OIDs required:
    
    - `1.3.6.1.4.1.311.10.3.22`
        
    - `1.3.6.1.4.1.311.10.3.20`
        
- Signer value derived from:
    
    - EKUs
        
    - Issuer (e.g., Microsoft Windows)
        
- Example:
    
    - `smss.exe` → allowed as `WinTcb-Light`
        

---

## 13. DLL loading restrictions (critical)

- Each process has:
    
    - **SignatureLevel** (EXE)
        
    - **SectionSignatureLevel** (DLLs)
        
- Code Integrity enforces:
    
    - Protected process can only load DLLs with **equal or higher trust**
        
- Example:
    
    - `WinTcb` process → only Windows-signed DLLs allowed
        
- Prevents:
    
    - DLL planting
        
    - Third-party code execution inside protected context
        

---

## 14. Real-world PPL examples (Windows 10+)

- **WinTcb-Light PPL**:
    
    - `smss.exe`
        
    - `csrss.exe`
        
    - `services.exe`
        
    - `wininit.exe`
        
- **LSASS**:
    
    - Always PPL on ARM
        
    - Optional on x86/x64 (registry / policy)
        
- Other protected services:
    
    - `sppsvc.exe`
        
    - Certain `svchost.exe` instances
        
    - WSL, AppX services
        

---

## 15. Minimum TCB guarantee

- Some binaries **must always** run protected
    
- OS enforces this **regardless of caller**
    
- Known as the **Minimum TCB list**
    

### Examples (Table 3-3)

|Process|Minimum Protection|
|---|---|
|smss.exe|WinTcb-Lite|
|csrss.exe|WinTcb-Lite|
|wininit.exe|WinTcb-Lite|
|services.exe|WinTcb-Lite|
|werfaultsecure.exe|WinTcb-Full|
|sppsvc.exe|Windows-Full|
|winlogon.exe|Windows-Full|
|lsass.exe|Inferred|

- Prevents launching these binaries without correct protection
    
- Critical for kernel and system integrity
    

---

## 16. Key mental model (bookmark this)

> **Protected / PPL processes exist so that even Administrator is “too weak”**  
> Trust is enforced by **signatures + kernel policy**, not user privileges.