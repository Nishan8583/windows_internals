## 1. First: “kernel mode” ≠ “the kernel”

**Kernel mode** is just a **CPU privilege level** (ring 0).

Inside kernel mode, Windows has **multiple components**:

- The **Kernel (ntoskrnl’s low-level core)**
    
- The **Executive**
    
- Device drivers
    
- HAL
    

So when a syscall enters kernel mode, it does **not** go “straight to the kernel” in the abstract sense.

---

## 2. The Executive: _policy and services_

The **Executive** is the **high-level OS**.

Think of it as:

> _“The part of the OS that provides services and enforces policy.”_

### Executive responsibilities

The Executive manages **objects and resources**:

- **Process & thread management**  
    (`Ps*`)
    
- **Memory management**  
    (`Mm*`)
    
- **I/O management**  
    (`Io*`)
    
- **Object manager**  
    (`Ob*`)
    
- **Security & access checks**  
    (`Se*`)
    
- **Configuration (Registry)**  
    (`Cm*`)
    
- **Power management**
    
- **ALPC**
    

When you call:

`NtCreateProcessEx NtReadFile NtOpenKey`

You are mostly talking to **Executive subsystems**.

📌 The Executive:

- Is object-oriented
    
- Uses reference counting
    
- Exposes the **NT system call API**
    
- Talks in terms of **processes, threads, handles, files**
    

---

## 3. The Kernel: _mechanism and primitives_

The **Kernel** is the **lowest-level core**.

Think of it as:

> _“The part that touches the CPU directly.”_

### Kernel responsibilities

- Thread scheduling & dispatching
    
- Interrupt handling
    
- Traps & exceptions
    
- IRQL management
    
- Spinlocks
    
- Context switching
    
- Timers
    
- Deferred Procedure Calls (DPCs)
    

It does **not** understand:

- Files
    
- Registry
    
- Security descriptors
    
- Handles
    

It only knows:

- Threads
    
- CPUs
    
- Interrupts
    
- Time
    

📌 The Kernel provides **mechanisms**, not policies.

---

## 4. Relationship between Executive and Kernel

The relationship is **layered**:

`User Mode  └─ NTDLL (syscall stubs) Kernel Mode  ├─ Executive (policy, objects, services)  │   └─ Uses kernel primitives  ├─ Kernel (scheduler, interrupts, CPU control)  ├─ Drivers  └─ HAL`

Example: creating a process

1. User calls `CreateProcess`
    
2. NTDLL issues a syscall
    
3. Executive:
    
    - Creates process object
        
    - Sets up address space
        
    - Performs security checks
        
4. Kernel:
    
    - Creates initial thread
        
    - Schedules it
        
    - Switches CPU context
        

---

## 5. Why Windows separates them

### Separation of concerns

- **Kernel** → fast, minimal, timing-critical
    
- **Executive** → complex, policy-heavy
    

This gives:

- Better performance
    
- Better maintainability
    
- Easier debugging
    
- Cleaner abstractions
    

The kernel stays **small and deterministic**.

---

## 6. Comparison to other OS designs

- **Microkernel** (Mach): moves many services out of kernel mode
    
- **Linux**: everything is “the kernel”
    
- **Windows**: _hybrid kernel_
    

Windows keeps:

- Executive in kernel mode
    
- But structured as a separate layer
    

---

## 7. Security / threat-hunting angle

- Syscalls mostly land in **Executive code paths**
    
- Rootkits often hook:
    
    - Executive dispatch paths
        
    - Kernel scheduler paths
        
- Abnormal transitions:
    
    - Executive services without expected kernel primitives
        
    - Direct kernel manipulation bypassing executive checks
        

Understanding this split helps spot:

- Privilege escalation
    
- Direct kernel object manipulation (DKOM)
    

---

## 8. One-line takeaway

> The Executive provides high-level OS services and policy, while the Kernel provides low-level CPU and scheduling mechanisms; both run in kernel mode but solve different problems.