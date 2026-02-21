# Part 1 — Mapping Signals to Sysmon & ETW

## Legend

- 🧩 = Correlation required (single event not enough)
    
- 🔴 = High-confidence malicious
    
- 🟡 = Suspicious, context needed
    

---

## 1. Vulnerable / Abnormal Driver Load (Entry Point)

**Why it matters**

> Almost all real-world PPL bypasses start here.

### Sysmon

**Event ID 6 — Driver Loaded**

- ImageLoaded = `.sys`
    
- Signature:
    
    - Valid but **unexpected**
        
    - Old / revoked / OEM driver
        
- Load location:
    
    - `C:\Windows\Temp\`
        
    - User-writable paths
        

🔴 Red flags:

- Driver loaded **after user logon**
    
- Driver not normally present on baseline
    

---

### ETW

**Providers**

- `Microsoft-Windows-Kernel-Driver`
    
- `Microsoft-Windows-CodeIntegrity`
    

**Signals**

- Driver loads that:
    
    - Barely pass CI
        
    - Trigger warnings
        
    - Are later blocked on other hosts
        

🧩 Correlate with:

- Subsequent LSASS / Defender interaction
    

---

## 2. Protected Process Loses Protection (Critical Signal)

**Why it matters**

> User-mode cannot do this — kernel involvement required.

### Sysmon

❌ Sysmon **cannot directly see** PPL flag changes  
(because this happens in kernel memory)

But you _can_ see **downstream effects**.

---

### ETW (Key)

**Providers**

- `Microsoft-Windows-Kernel-Process`
    
- `Microsoft-Windows-Threat-Intelligence`
    
- (EDR-specific kernel telemetry is often needed)
    

**Signals**

- Process metadata change **without restart**
    
- Protected process suddenly allowing:
    
    - VM_READ
        
    - DLL enumeration
        
    - Handle duplication
        

🔴 If observed → **assume kernel compromise**

---

## 3. LSASS / Defender Access Attempts

**Why it matters**

> This is the attacker’s objective phase.

### Sysmon

**Event ID 10 — ProcessAccess**

Watch for:

- TargetImage:
    
    - `\lsass.exe`
        
    - `\MpDefenderCoreService.exe`
        
- GrantedAccess:
    
    - `0x10`, `0x20`, `0x40`
        
    - `PROCESS_VM_READ`
        
    - `PROCESS_CREATE_THREAD`
        

🟡 Normal:

- AV / backup / crash handlers  
    🔴 Malicious:
    
- Random user process
    
- Recently spawned binary
    
- Unsigned or LOLBin parent
    

---

### ETW

**Providers**

- `Microsoft-Windows-Kernel-Audit-API-Calls`
    
- `Microsoft-Windows-Threat-Intelligence`
    

**Signals**

- Successful memory access to PPL target
    
- Handle grants that should be denied
    

🧩 Correlate with:

- Driver load
    
- PPL downgrade window
    

---

## 4. DLL Enumeration / Memory Reads of PPL

**Why it matters**

> Normally impossible from user mode.

### Sysmon

Indirect detection:

- **Event ID 10** followed by:
    
- **Event ID 7 — ImageLoaded** (DLLs related to dumping)
    

---

### ETW

**Providers**

- `Microsoft-Windows-Kernel-Memory`
    
- `Microsoft-Windows-CodeIntegrity`
    

**Signals**

- Memory reads from protected process
    
- CI checks suddenly succeeding where they should fail
    

🔴 Extremely high confidence

---

## 5. Credential Dump or Post-Access Activity

**Why it matters**

> Confirms intent.

### Sysmon

**Event ID 11 — FileCreate**

- Files created shortly after LSASS access
    
- Examples:
    
    - `.dmp`
        
    - Suspicious temp files
        

**Event ID 1 — ProcessCreate**

- Known dump tools
    
- Or renamed binaries
    

---

### ETW

**Providers**

- `Microsoft-Windows-Security-Auditing`
    
- `Microsoft-Windows-Threat-Intelligence`
    

**Signals**

- Credential material access
    
- LSASS interaction alerts (often EDR-only)
    

---

# Part 2 — Real Attack Chain Timeline (Defensive View)

This is the **most realistic modern sequence** you should memorize.

---

## Phase 0 — Initial Access

- Phishing
    
- Exploit
    
- Initial foothold as **standard user**
    

🧠 No PPL signals yet

---

## Phase 1 — Privilege Escalation

- Local privilege escalation
    
- Attacker becomes **Administrator**
    

🟡 Still normal — Admin alone ≠ dangerous

---

## Phase 2 — Driver Introduction (Inflection Point)

**Key Event**

- Signed but vulnerable driver is loaded
    

📍 Signals:

- Sysmon Event 6
    
- ETW Kernel-Driver
    
- CI warnings or borderline signatures
    

🧠 This is where defenders should already be alert.

---

## Phase 3 — Kernel Manipulation

**What changes**

- PPL / CI enforcement weakened
    
- `EPROCESS.Protection` modified
    

📍 Signals:

- ETW kernel anomalies
    
- Protected process suddenly “behaves like normal”
    

🔴 This is the **point of no return**

---

## Phase 4 — Protected Target Access

**Targets**

- `lsass.exe`
    
- Defender / EDR services
    

📍 Signals:

- Sysmon Event 10 (successful access)
    
- ETW confirms handle grants
    

🔴 Should _never_ succeed legitimately

---

## Phase 5 — Credential Theft / EDR Blinding

**Outcomes**

- Credentials dumped
    
- Defender disabled or bypassed
    

📍 Signals:

- File creation
    
- Unusual child processes
    
- Security service instability
    

---

## Phase 6 — Persistence / Lateral Movement

- New services
    
- New scheduled tasks
    
- Reused credentials
    

🧠 At this point, the breach is **fully weaponized**

---

# Correlation Rules (High Value)

### 🔴 Rule 1 — Driver → LSASS Access

`DriverLoad → Within 5–15 minutes → LSASS ProcessAccess`

### 🔴 Rule 2 — PPL Target Memory Access

`ProcessAccess where Target = LSASS AND GrantedAccess includes VM_READ AND SourceSigner < Lsa`

### 🔴 Rule 3 — Defender Loses Protection

`Defender process state change WITHOUT service restart or update`

---

# Executive Summary (1-Minute Recall)

- **No user-mode process should ever read PPL memory**
    
- **PPL bypass almost always involves a driver**
    
- **LSASS access after driver load = critical**
    
- **If PPL breaks, assume kernel compromise**