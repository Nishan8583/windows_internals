# What You Should Understand as a Threat Hunter

## 1. Process Creation & Parentage (Very High Value)

### What to Understand

- **Process creation flow**
    
    - `CreateProcessInternalW`
        
    - `NtCreateUserProcess`
        
- **When a process “exists”**
    
    - Before `main()` / `WinMain()`
        
- **PPID assignment**
    
    - Why PPID spoofing must happen at creation time
        
- **Suspended process state**
    
    - Why injection before execution is possible
        

### Why It Matters

Attackers commonly:

- Spoof PPID to masquerade as trusted parents
    
- Inject before first instruction executes
    
- Use unusual parents (`svchost.exe`, `services.exe`, `smss.exe`)
    

### Hunt For

- Parent/child relationships that violate normal trees
    
- Suspended → resumed processes with memory writes
    
- Processes without expected initialization behavior
    

---

## 2. Image Loading & DLL Behavior (Extremely High Value)

### What to Understand

- DLL **search order**
    
- Known DLLs
    
- Side-by-side (SxS) & redirection
    
- API Sets (`api-ms-win-*`)
    
- Loader data in the **PEB**
    
    - `PEB_LDR_DATA`
        
    - InLoadOrder / InMemoryOrder lists
        

### Why It Matters

Classic and modern attacks:

- DLL search order hijacking
    
- Phantom DLLs loaded before legit ones
    
- Manual mapping (DLL not in loader lists)
    

### Hunt For

- DLLs loaded from writable directories
    
- Missing DLLs replaced by attacker-controlled ones
    
- Loaded modules not backed by files
    
- Memory-only DLLs not listed in PEB
    

---

## 3. Memory Injection & Execution (Critical)

### What to Understand

- Virtual memory layout
    
- `VirtualAlloc`, `NtMapViewOfSection`
    
- RW → RX transitions
    
- Thread creation:
    
    - `CreateRemoteThread`
        
    - APC injection
        
- Section objects vs private memory
    

### Why It Matters

Most malware eventually:

- Writes executable memory
    
- Executes from non-image regions
    

### Hunt For

- Executable memory not backed by an image
    
- Threads starting at non-module addresses
    
- Suspicious memory protection changes
    
- PE headers in heap or private memory
    

---

## 4. Handle, Object & Namespace Abuse (High Value)

### What to Understand

- Object Manager namespace:
    
    - `\BaseNamedObjects`
        
    - `\Device`
        
- Named pipes
    
- Mutexes, events, sections
    
- Handle duplication & inheritance
    

### Why It Matters

Attackers use:

- Named pipes for C2
    
- Mutexes for infection tracking
    
- Section objects for stealth injection
    

### Hunt For

- Processes opening high-privilege handles
    
- Unusual named objects
    
- Cross-session or cross-silo access
    

---

## 5. Job Objects, Silos & Containers (Medium–High Value)

### What to Understand

- Job objects as **control boundaries**
    
- Server silos (Windows containers)
    
- What is isolated vs shared
    
- Silo namespace redirection
    

### Why It Matters

Attackers can:

- Escape or abuse weak isolation
    
- Hide activity inside containers
    
- Abuse shared kernel resources
    

### Hunt For

- Processes unexpectedly running in silos
    
- Access to host namespaces from containers
    
- Container workloads spawning suspicious binaries
    

> You don’t need to _build_ silos—but you must recognize when activity crosses silo boundaries.

---

## 6. Token, Logon Session & Privileges (Very High Value)

### What to Understand

- Access tokens
    
- LUIDs
    
- Impersonation vs primary tokens
    
- Privilege enabling/disabling
    

### Why It Matters

Credential abuse relies on:

- Token theft
    
- Impersonation
    
- Privilege escalation
    

### Hunt For

- Token duplication
    
- Unexpected privilege enablement
    
- SYSTEM tokens outside expected processes
    
- Service accounts used interactively
    

---

## 7. Persistence Mechanisms (High Value)

### What to Understand

- Registry autoruns
    
- Services & scheduled tasks
    
- DLL load triggers
    
- COM hijacking
    
- WMI event subscriptions
    

### Why It Matters

Persistence is:

- Noisy in internals
    
- Predictable in structure
    

### Hunt For

- Autoruns pointing to user-writable locations
    
- Services executing from temp paths
    
- WMI consumers spawning LOLBins
    

---

## 8. LOLBins & Native Abuse (Critical for Threat Hunting)

### What to Understand

- `rundll32`
    
- `mshta`
    
- `powershell`
    
- `wmic`
    
- `regsvr32`
    

### Why It Matters

Attackers prefer:

- Living off the land
    
- Avoiding dropped binaries
    

### Hunt For

- LOLBins executing with abnormal arguments
    
- Network activity from tools that shouldn’t talk to the network
    
- Child processes that don’t match normal usage patterns
    

---

## 9. ETW, Logging & Visibility Gaps (Advanced but Valuable)

### What to Understand

- ETW providers
    
- User vs kernel providers
    
- Event suppression & tampering
    

### Why It Matters

Advanced attackers:

- Blind your telemetry
    
- Abuse legitimate providers
    

### Hunt For

- Gaps in expected telemetry
    
- Processes disabling logging
    
- ETW providers stopped unexpectedly
    

---

## 10. What You Can Safely Ignore (Most of the Time)

You **do NOT** need to deeply memorize:

- Exact kernel structures offsets
    
- Full syscall tables
    
- GUI internals
    
- Rare legacy subsystems
    

Know **what exists**, not **every field**.

---

# Mental Rule for Threat Hunters

> **Attackers violate normal OS behavior.**  
> Your job is to know what “normal” looks like.

If you understand:

- How processes are born
    
- How code is loaded
    
- How memory is executed
    
- How identity (tokens) works
    

You can detect **90% of real-world malware**.