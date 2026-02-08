## Process Internals — Security-Oriented Notes

### Core representation

- **Every Windows process is represented by exactly one `EPROCESS` structure**
    
- `EPROCESS` is the **authoritative kernel object** for a process
    
- If code executes, an `EPROCESS` **must exist** (cannot be bypassed)
    

**Security takeaway:**  
Attackers can hide _how_ code is loaded, but they cannot hide the existence of an `EPROCESS`.

---

### Related kernel structures

- Each process has **one or more threads**, each represented by an `ETHREAD`
    
- `EPROCESS` maintains links to:
    
    - Thread list
        
    - Handle table
        
    - Token
        
    - Parent/child relationships
        

**Security takeaway:**  
Most attack techniques (injection, hollowing, APCs) inevitably interact with:

- Threads (`ETHREAD`)
    
- Handles
    
- Tokens  
    These interactions leave artifacts.
    

---

### Memory location & trust boundaries

- `EPROCESS` and most related structures live in **kernel (system) address space**
    
- **PEB (Process Environment Block)** lives in **user-mode address space**
    
- Some memory structures are **process-context–specific**, even in kernel space
    

**Security takeaway:**

- User-mode data (PEB) is **more easily manipulated**
    
- Kernel-mode structures (`EPROCESS`, `KPROCESS`) are **much harder to fake**
    
- Trust kernel invariants over user-mode metadata
    

---

### Windows subsystem structures (parallel tracking)

For each Windows GUI process, **multiple subsystems track process state**:

#### `CSR_PROCESS` (Csrss)

- Maintained by the **Windows subsystem (Csrss.exe)**
    
- Exists only for **Windows subsystem processes**
    
- Per-session tracking (each session has its own Csrss)
    

**Security takeaway:**

- Multiple independent structures track the same process
    
- Inconsistencies across them can be a signal
    
- Some processes (e.g., Smss) intentionally lack `CSR_PROCESS`
    

---

#### `W32PROCESS` (Win32k.sys)

- Created when a process first calls a **USER/GDI** syscall
    
- Triggered when `User32.dll` is loaded
    
- Used for GUI/window state tracking
    

**Security takeaway:**

- GUI capability implies additional kernel structures
    
- Headless malware may avoid this
    
- GUI malware cannot
    

---

#### `DXGPROCESS` (DirectX / graphics)

- Created for processes using DirectX / GPU features
    
- Tracks graphics objects and GPU scheduling state
    

**Security takeaway:**

- Mostly irrelevant for threat hunting
    
- Useful mainly for sandboxing / evasion research
    
- Safe to ignore for now
    

---

### Object Manager & handles

- Every `EPROCESS` (except Idle) is wrapped as a **process object**
    
- Process objects are **unnamed**
    
- Access happens through **handles**, not direct object names
    

**Security takeaway:**

- Handle creation is often unavoidable for attacks
    
- Suspicious access rights (`VM_WRITE`, `CREATE_THREAD`) are strong signals
    
- Handles persist longer than many expect
    

---

### Process creation callbacks

- Drivers can register **process creation notification callbacks**
    
- Used heavily by:
    
    - AV
        
    - EDR
        
    - Security tooling
        
- Callbacks can:
    
    - Observe process creation
        
    - Block process creation entirely
        

**Security takeaway:**

- Process creation is a **highly monitored choke point**
    
- Attackers cannot create a process without triggering kernel callbacks
    
- Blocking at creation time is architecturally supported
    

---

### Layering: `EPROCESS` vs `KPROCESS`

- `EPROCESS` (executive layer)
    
- Contains a `KPROCESS` (kernel/scheduler layer)
    
- Scheduler and dispatcher use `KPROCESS`
    
- Executive uses `EPROCESS`
    

**Security takeaway:**

- Separation enforces invariants across layers
    
- Even if executive-level metadata is manipulated, scheduler-level state remains
    
- Harder for attackers to fully desynchronize process state
    

---

### PEB (user-mode structure)

- Lives in **user-mode**
    
- Used by:
    
    - Loader
        
    - Heap manager
        
    - Runtime components
        
- Accessible without syscalls
    

**Security takeaway:**

- PEB is **commonly tampered with**
    
- Do not rely on PEB alone for detection
    
- Cross-check with kernel-backed telemetry
    

---

### Symbol visibility & limits

- WinDbg provides visibility into:
    
    - `EPROCESS`
        
    - `KPROCESS`
        
    - `PEB`
        
- Win32k (`W32PROCESS`) symbols are **not public**
    

**Security takeaway:**

- Lack of symbols ≠ lack of observability
    
- Kernel still enforces invariants even if structures aren’t easily inspectable
    

---

## Key security conclusions (the important part)

- **Processes are tracked redundantly** across subsystems
    
- **Kernel structures are the source of truth**
    
- **User-mode structures are convenience layers**
    
- **Attackers can evade APIs, but not kernel object creation**
    
- **Detection improves when you reason in terms of invariants, not APIs**
    

---

## One sentence to keep

> **If execution exists, the kernel knows — and that knowledge lives in `EPROCESS` and related objects.**