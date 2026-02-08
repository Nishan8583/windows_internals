# Protected Processes & PPL — Threat-Hunting Signal Map

## 1. Admin omnipotence vs Protected Processes

**Concept**

- Normally, Admin + SeDebugPrivilege = full access
    
- Protected / PPL breaks this rule
    

**Threat signal**

- Admin process **fails** to open another process
    
- `ACCESS_DENIED` on `OpenProcess`, `NtReadVirtualMemory`
    

**Why it matters**

- Expected for LSASS / Defender
    
- Unexpected for normal processes
    

**Hunt**

- Alert when:
    
    - High-privilege process repeatedly attempts memory access to PPL targets
        
    - Especially `lsass.exe`, `MpDefenderCoreService.exe`
        

---

## 2. Rare presence of Protected / PPL processes

**Concept**

- Very few processes should be protected
    

**Threat signal**

- **Unknown / third-party process** marked as:
    
    - `PsProtectedSignerAntimalware`
        
    - `PsProtectedSignerWinTcb`
        

**Why it matters**

- Malware cannot legitimately obtain these signer levels
    
- Indicates kernel tampering or stolen certificate
    

**Hunt**

- Baseline:
    
    - Expected protected process list
        
- Alert on:
    
    - New protected process name
        
    - Protected process not signed by Microsoft / known EDR
        

---

## 3. Empty DLL list in user-mode tools

**Concept**

- PPL blocks reading PEB / loader lists
    

**Threat signal**

- DLL list **visible** for:
    
    - `lsass.exe`
        
    - Defender
        
    - `csrss.exe`
        

**Why it matters**

- Strong sign of:
    
    - PPL disabled
        
    - Kernel driver abuse
        

**Hunt**

- Endpoint integrity check:
    
    - Compare visibility of DLLs over time
        
- Alert if:
    
    - Previously hidden DLL list becomes readable
        

---

## 4. Limited access rights to PPL

**Concept**

- Only limited process rights allowed
    

**Threat signal**

- Successful handle with:
    
    - `PROCESS_VM_READ`
        
    - `PROCESS_VM_WRITE`
        
    - `PROCESS_CREATE_THREAD`
        
    - against a PPL
        

**Why it matters**

- Should never happen from user mode
    
- Indicates kernel-level bypass
    

**Hunt**

- Sysmon / ETW:
    
    - Detect high-risk access masks to PPL targets
        
- Kernel telemetry:
    
    - Handle duplication anomalies
        

---

## 5. Protected Process Light signer hierarchy

**Concept**

- Higher signer can access lower, never vice versa
    

**Threat signal**

- Lower signer process interacting with higher signer
    
    - e.g., user process → WinTcb PPL
        

**Why it matters**

- Violates protection boundary
    
- Indicates signer spoofing or kernel manipulation
    

**Hunt**

- Track:
    
    - Source signer vs target signer
        
- Alert on:
    
    - Cross-signer access violations
        

---

## 6. LSASS running without PPL

**Concept**

- LSASS should be PPL (especially on modern systems)
    

**Threat signal**

- `lsass.exe`:
    
    - Protection = None
        
    - Or downgraded from `Lsa-Light`
        

**Why it matters**

- Classic pre-dump step
    
- Seen before credential dumping
    

**Hunt**

- Continuous compliance check:
    
    - LSASS protection state
        
- Alert on:
    
    - Protection downgrade
        
    - LSASS restart without PPL
        

---

## 7. Defender / EDR without Antimalware PPL

**Concept**

- Defender runs as `PsProtectedSignerAntimalware-Light`
    

**Threat signal**

- Defender service:
    
    - Not protected
        
    - Or terminable by user process
        

**Why it matters**

- Indicates EDR blinding attempt
    
- Often follows vulnerable driver load
    

**Hunt**

- Correlate:
    
    - Driver load → Defender protection loss
        
- Alert on:
    
    - Defender process termination allowed
        

---

## 8. Kernel drivers modifying EPROCESS

**Concept**

- PPL state stored in `EPROCESS.Protection`
    

**Threat signal**

- Sudden protection change without reboot
    

**Why it matters**

- Only kernel can do this
    
- Strong DKOM indicator
    

**Hunt**

- Kernel telemetry:
    
    - EPROCESS field integrity checks
        
- Alert on:
    
    - Protection flag toggles
        

---

## 9. DLL Signature Level enforcement

**Concept**

- PPL can only load trusted DLLs
    

**Threat signal**

- PPL loads:
    
    - Unsigned DLL
        
    - Third-party DLL
        

**Why it matters**

- Either:
    
    - Code Integrity bypass
        
    - Kernel compromise
        

**Hunt**

- CI logs:
    
    - Blocked DLL loads in protected processes
        
- Alert on:
    
    - Unexpected DLL load attempts into PPL
        

---

## 10. Minimum TCB list enforcement

**Concept**

- Some binaries must _always_ run protected
    

**Threat signal**

- `smss.exe`, `csrss.exe`, `wininit.exe`:
    
    - Running unprotected
        
    - Or restarted without protection
        

**Why it matters**

- System trust boundary broken
    
- Extremely high-severity indicator
    

**Hunt**

- Startup validation:
    
    - Verify protection + signer level
        
- Alert immediately on:
    
    - Minimum TCB violation
        

---

## 11. Driver loading before PPL bypass

**Concept**

- User-mode cannot bypass PPL directly
    

**Threat signal**

- Sequence:
    
    1. Vulnerable driver loaded
        
    2. PPL downgraded
        
    3. LSASS accessed
        

**Why it matters**

- Canonical real-world attack chain
    

**Hunt**

- Correlate events:
    
    - Driver load (especially known vulnerable)
        
    - Followed by PPL changes
        
- Time-window correlation is key
    

---

## 12. Tool behavior mismatch

**Concept**

- User-mode tools limited, kernel tools are not
    

**Threat signal**

- Attacker tool shows more visibility than expected
    

**Why it matters**

- Suggests kernel access
    
- Or malicious driver present
    

**Hunt**

- Monitor:
    
    - Unexpected kernel debugger attachment
        
    - Driver-backed user-mode tools
        

---

## 13. Core hunting mental model

> **Protected processes are “canaries” for kernel compromise**

If you see:

- Protection downgrade
    
- Unexpected access
    
- Visibility where none should exist
    

➡️ **Assume kernel-level activity until proven otherwise**

## Threat-hunting insight

From a defender’s perspective:

🚨 **Suspicious signs**:

- Defender process where DLL list **is visible**
    
- LSASS DLLs readable by user-mode tools
    
- PPL process allowing memory reads
    

That usually means:

- PPL was disabled
    
- Kernel protection was bypassed
    
- Vulnerable driver abuse
    

Attack chains often look like:

`Load vulnerable driver → disable PPL → dump LSASS`

---

## How attackers bypass this (high level, no how-to)

- Kernel-mode drivers
    
- Direct kernel object manipulation (DKOM)
    
- PPL downgrade via driver bugs
    

This is why modern EDRs:

- Monitor driver loading
    
- Watch for PPL state changes
    
- Alert on access attempts to protected processes