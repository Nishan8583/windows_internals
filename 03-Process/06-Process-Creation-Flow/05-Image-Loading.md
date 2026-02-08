## 1. Loaded Module Database (PEB → PEB_LDR_DATA)

### What it is

- Every process maintains a **loader database** of:
    
    - The main executable
        
    - All loaded DLLs
        
- Stored in **PEB → Ldr → PEB_LDR_DATA**
    
- Tracks modules using:
    
    - **3 doubly linked lists** (load order, memory order, init order)
        
    - **2 red-black trees**:
        
        - Sorted by **base address**
            
        - Sorted by **hash of module name**
            
- Each module is represented by an **LDR_DATA_TABLE_ENTRY**
    

---

### Security relevance

- This database is the **ground truth of what code is mapped into a process**
    
- Used heavily in:
    
    - Malware analysis
        
    - Process injection detection
        
    - DLL hijacking investigation
        
    - In-memory (fileless) malware detection
        

---

### Attacker focus

- Malware often:
    
    - Walks the **PEB loader lists** to find loaded modules
        
    - Locates **ntdll/kernel32** without calling APIs (to avoid hooks)
        
- Linked lists are **easy to find and walk**
    
- Red-black tree roots are **intentionally hidden from the PEB** → harder for shellcode to locate under **ASLR**
    

---

### Defender focus

- Compare:
    
    - Modules in **linked lists**
        
    - vs. memory mappings
        
- Mismatches → **unlinked / hidden DLLs**
    
- Example detection idea:
    
    > “Memory mapped PE not present in PEB_LDR_DATA lists”
    

---

## 2. LDR_DATA_TABLE_ENTRY — Key Fields (Security-Critical)

### High-value fields for security analysis

|Field|Why it matters|
|---|---|
|`FullDllName`|Detect DLLs loaded from suspicious paths (e.g. writable dirs)|
|`DllBase`|Identify injected or manually mapped DLLs|
|`EntryPoint`|Target for malicious execution / patching|
|`OriginalBase`|Detect ASLR bypass or relocation abuse|
|`LoadReason`|Distinguish static vs dynamic vs delay-loaded DLLs|
|`ParentDllBase`|Track dependency chains (who loaded what)|
|`SigningLevel`|Code Integrity / signed vs unsigned module|
|`TimeDateStamp`|Spot timestomping / suspicious build times|
|`Flags`|Understand loader state & execution stage|

---

### Security insight

- Legitimate software tends to have:
    
    - Signed modules
        
    - Consistent load reasons
        
    - Expected load order
        
- Malware often:
    
    - Loads unsigned DLLs dynamically
        
    - Manually maps code → missing or inconsistent fields
        
    - Injects after process initialization
        

---

## 3. Loader Flags (Table 3-10) — Security Interpretation

### Flags defenders care about most

|Flag|Security meaning|
|---|---|
|`Load In Progress`|Process mid-load → common injection timing|
|`Entry Processed`|Safe point; injection after this is suspicious|
|`Process Attach Called`|Confirms DllMain execution|
|`Process Attach Failed`|Possible malicious or broken DLL|
|`Don’t Relocate`|ASLR disabled → exploit-friendly|
|`Protect Delay Load`|CFG protection active|
|`COR Image / IL Only`|.NET malware classification|
|`Shim DLL`|AppCompat abuse (used in persistence)|

---

### Attacker focus

- Inject **before** `Entry Processed` to blend in
    
- Abuse:
    
    - Shims
        
    - Delay-load hooks
        
    - DllMain side effects
        

---

### Defender focus

- Alert on:
    
    - Late DLL loads in sensitive processes
        
    - Non-Microsoft DLLs in protected processes
        
    - CFG / ASLR-disabled modules
        

---

## 4. Import Parsing & DLL Loading — Attack Surface

### What happens

- Loader:
    
    1. Reads import table
        
    2. Resolves DLL paths
        
    3. Applies KnownDLL logic
        
    4. Handles relocation (ASLR)
        
    5. Resolves IAT (by name or ordinal)
        
    6. Handles forwarders
        
    7. Repeats recursively for dependencies
        

---

### Security risks

- **DLL search order hijacking**
    
- **.local redirection**
    
- **Manifest-based DLL redirection**
    
- **Forwarder abuse**
    
- **Ordinal imports (harder to inspect)**
    

---

### Attacker techniques

- Drop malicious DLL in:
    
    - Application directory
        
    - Current working directory
        
- Abuse `.local` to bypass KnownDLLs
    
- Exploit missing relocation info to crash or hijack execution
    

---

### Defender detection ideas

- DLL loaded from:
    
    - User-writable paths
        
    - App directory instead of System32
        
- DLL name matches system DLL but wrong path
    
- Ordinal-only imports in unusual binaries
    

---

## 5. Post-Import Initialization (High-Risk Phase)

### What happens

- `DllMain(DLL_PROCESS_ATTACH)` executed for every DLL
    
- TLS callbacks executed
    
- Shims applied
    
- Subsystem initialization runs
    
- ETW “process started successfully” event emitted
    

---

### Why this is critical

- **Last chance to stop process startup**
    
- Malware frequently:
    
    - Executes payload in `DllMain`
        
    - Uses TLS callbacks to run **before main()**
        
    - Injects here to blend into normal startup
        

---

### Attacker focus

- TLS callbacks = stealthy execution
    
- DllMain abuse for:
    
    - Process hollowing
        
    - Early persistence
        
    - Security bypass before EDR fully hooks APIs
        

---

### Defender focus

- Monitor:
    
    - TLS callback execution
        
    - Unusual DllMain behavior
        
    - ETW gaps (process runs but “load success” event missing or delayed)
        

---

## 6. User-Mode vs Kernel-Mode Loader

### Key distinction

- User mode:
    
    - PEB → `PEB_LDR_DATA`
        
- Kernel mode:
    
    - `PsActiveModuleList`
        
    - `_KLDR_DATA_TABLE_ENTRY`
        

---

### Security relevance

- Rootkits may:
    
    - Unlink kernel drivers from `PsActiveModuleList`
        
- Cross-view detection:
    
    - Compare kernel module list vs memory mappings
        

---

## 7. How This Helps in Cybersecurity Work

### Malware analysis

- Identify:
    
    - Hidden DLLs
        
    - Manual mapping
        
    - Early-execution tricks (TLS, DllMain)
        

### Threat hunting

- Build detections around:
    
    - Unusual load paths
        
    - Unsigned modules
        
    - Late dynamic loads
        

### EDR / DFIR

- Verify process integrity:
    
    - Does the loader database match expected state?
        
- Correlate:
    
    - ETW process start events
        
    - Loaded module timelines