# Scenario: Credential Theft via Kernel Driver (Real-World Attack)

## Attacker Goal

Steal **domain credentials** from a Windows machine to move laterally.

This is a **very common real-world objective** (APT, ransomware crews, red teams).

---

## Part 1: System **WITHOUT** VBS (Traditional Windows)

### Step 1: Initial Foothold

- Attacker gains **local admin** (phishing, exploit, weak password).
    
- This is already bad—but **not game over yet**.
    

---

### Step 2: Load a Malicious or Vulnerable Driver

- Attacker:
    
    - Loads a malicious kernel driver
        
    - Or abuses a **signed but vulnerable driver** (BYOVD attack)
        

Examples seen in the wild:

- GDRV.sys
    
- ASUS, MSI, Gigabyte drivers
    
- Cheat engine drivers abused by ransomware
    

Result:

- Attacker now has **ring 0 execution**
    

---

### Step 3: Read LSASS Memory

- Driver directly reads kernel memory
    
- Extracts:
    
    - NTLM hashes
        
    - Kerberos TGTs
        
    - Cached credentials
        

Tools:

- Custom driver
    
- Mimikatz kernel mode
    
- Direct memory scraping
    

---

### Step 4: Lateral Movement

- Attacker uses stolen credentials:
    
    - Pass-the-Hash
        
    - Pass-the-Ticket
        
- Domain compromise follows
    

➡️ **Complete system compromise**

---

# Part 2: Same Attack **WITH VBS + Credential Guard**

Now let’s replay the **exact same attack**.

---

## Step 1: Initial Foothold

✅ Same as before  
Attacker gets admin.

VBS does **not** prevent admin compromise.

---

## Step 2: Load Malicious Driver

⚠️ Driver **still loads**  
Attacker still gets:

- Kernel-mode execution
    
- Ring 0 in **VTL 0**
    

This is key:  
**VBS is not about stopping kernel exploits.**

---

## Step 3: Attempt to Read LSASS Memory

❌ **Attack fails**

Why?

### What Changed

- LSASS secrets are now stored in:
    
    - **VTL 1 memory**
        
    - Managed by the **secure kernel**
        
- Kernel drivers run in:
    
    - **VTL 0**
        
- Hypervisor enforces:
    
    - **SLAT-based memory isolation**
        

Result:

- Kernel driver **cannot see LSASS secrets**
    
- Memory reads return garbage or fail
    

Even though:

- Attacker has ring 0
    
- Attacker has a kernel driver
    

---

## Step 4: Attempt DMA Attack

Attacker tries:

- PCIe DMA
    
- Thunderbolt attack
    
- Malicious device firmware
    

❌ Blocked by:

- **I/O MMU**
    
- DMA remapping
    
- Hypervisor protections
    

---

## Step 5: Attempt to Run Code in VTL 1

Attacker tries to:

- Inject a VTL 1 process
    
- Execute code in Isolated User Mode
    

❌ Impossible

Why?

- Only **Microsoft-signed Trustlets** may run in VTL 1
    
- Secure kernel has hard-coded Trustlet identities
    

---

## Final Result

- Attacker still has:
    
    - Admin
        
    - Kernel access
        
- But **cannot steal credentials**
    
- Lateral movement is blocked
    
- Blast radius is dramatically reduced
    

➡️ **Attack is contained**

---

# Why This Matters (Big Picture)

### Traditional Model

> “If kernel is compromised, game over”

### VBS Model

> “Even if kernel is compromised, some secrets remain protected”

This is a **fundamental shift** in OS security.

---

# Another Scenario (Short): Ransomware Defense

## Without VBS

- Ransomware loads driver
    
- Disables AV
    
- Hooks kernel
    
- Dumps credentials
    
- Encrypts domain-wide
    

## With VBS + Device Guard

- Driver execution restricted
    
- Unauthorized kernel code blocked
    
- Memory execution controlled
    
- Credential theft fails
    

---

# Threat-Hunting Signals (What You’d Look For)

If you’re thinking like a **threat hunter**, VBS gives you:

- Kernel driver load events that **don’t lead to credential theft**
    
- Failed LSASS access attempts
    
- Suspicious drivers + no successful credential usage
    
- Abnormal hypervisor / secure kernel ETW events
    

This mismatch itself is a **high-confidence signal**.

---

# One-Sentence Summary

**Virtualization-based security prevents kernel-level attackers from accessing the most sensitive secrets by placing them in a higher-trust execution environment enforced by hardware, not software.**