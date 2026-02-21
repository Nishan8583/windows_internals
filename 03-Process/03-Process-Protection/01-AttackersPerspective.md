# How attackers obtain (or appear to obtain) Protected / PPL certificates

_(Defender / Threat-hunting perspective)_

## First: a critical reality check

> **Attackers do NOT normally “get” real Microsoft PPL / WinTcb certificates.**

Those certificates:

- Are issued by Microsoft
    
- Require special EKUs
    
- Are tied to Microsoft’s internal trust chain
    
- Cannot be legitimately requested by third parties
    

So when we see malware running as **Antimalware / WinTcb**, something abnormal happened.

---

## The real-world methods attackers rely on

### 1. **Certificate theft (most realistic)**

**What happens**

- A legitimate vendor’s code-signing certificate is stolen
    
- Malware is signed with it
    
- Trust is abused _temporarily_
    

**Limits**

- ❌ Usually **not sufficient** for PPL / WinTcb
    
- ✔ Can bypass SmartScreen, AV heuristics
    
- ✔ Can load kernel drivers (until revoked)
    

**Threat-hunting signals**

- New binaries signed by a **known vendor**, but:
    
    - Wrong behavior
        
    - Wrong file paths
        
    - Recently issued certificate
        
- Certificate later **revoked by Microsoft**
    

📌 Example incidents:

- CCleaner
    
- SolarWinds
    
- Stolen hardware vendor certs used for drivers
    

---

### 2. **Abusing vulnerable signed drivers (most common PPL bypass path)**

**This is the big one.**

**What happens**

- Attacker loads a **legit, signed kernel driver**
    
- Driver has a vulnerability
    
- Attacker uses it to:
    
    - Modify `EPROCESS.Protection`
        
    - Disable Code Integrity
        
    - Bypass PPL entirely
        

**Key point**

> They don’t need the certificate — they **remove the enforcement**.

**Threat-hunting signals**

- Driver load followed by:
    
    - LSASS access
        
    - Defender losing protection
        
- Known vulnerable drivers:
    
    - Old hardware drivers
        
    - Gaming anti-cheat drivers
        
    - OEM utilities
        

This is the **dominant real-world technique**.

---

### 3. **Microsoft-signed binaries abused via LOLBins**

**What happens**

- Attacker uses **existing Microsoft-signed executables**
    
- Chains them in unintended ways
    
- Leverages trusted execution context
    

**Limits**

- ❌ Cannot create new PPL processes
    
- ✔ Can interact with trusted subsystems
    
- ✔ Often part of **pre-bypass staging**
    

**Threat-hunting signals**

- Microsoft binaries used with:
    
    - Unusual arguments
        
    - Unusual parents
        
    - Non-standard execution chains
        

---

### 4. **Supply-chain compromise (rare, catastrophic)**

**What happens**

- Build system compromised
    
- Legitimate software ships malware
    
- Signed correctly by vendor
    

**Limits**

- Still **unlikely** to grant WinTcb / Antimalware PPL
    
- Massive detection footprint
    
- Rapid ecosystem response
    

**Threat-hunting signals**

- Widespread infection
    
- Trusted software suddenly behaving maliciously
    
- Emergency certificate revocations
    

---

### 5. **They don’t get the cert — they downgrade the target**

This is the **most important mental shift**.

Instead of:

> “How do I get Antimalware PPL?”

Attackers ask:

> “How do I make Defender / LSASS _not protected_?”

Typical flow:

`Load signed vulnerable driver → Disable PPL / CI → Access LSASS / Defender → Dump credentials`

This bypasses **all certificate requirements**.

---

## Why forging the certificate is not realistic

To actually obtain a PPL-capable cert, an attacker would need:

- Microsoft-issued EKUs
    
- Correct issuer strings
    
- Internal trust mapping
    
- CI acceptance at boot time
    

This would require:

- Microsoft CA compromise **or**
    
- Kernel CI compromise
    

Either case = **nation-state level incident**

---

## Threat-hunting takeaway (this is key)

If you see:

- Non-Microsoft process with:
    
    - `PsProtectedSignerAntimalware`
        
    - `PsProtectedSignerWinTcb`
        

Assume:

- Kernel compromise
    
- Or stolen Microsoft trust
    
- Or CI enforcement failure
    

➡️ **Severity: Critical**

---

## Practical hunting rules of thumb

- 🔴 **No third-party process should ever be WinTcb**
    
- 🔴 **Antimalware PPL should match installed EDR**
    
- 🔴 **PPL downgrade = pre-credential dumping**
    
- 🔴 **Driver loads precede PPL bypass**
    

---

## One-line summary to remember

> **Attackers don’t earn trust — they break the system that enforces it.**