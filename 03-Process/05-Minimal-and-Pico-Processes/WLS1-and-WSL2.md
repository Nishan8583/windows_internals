## 1. Clear split: WSL 1 vs WSL 2

|Feature|WSL 1|WSL 2|
|---|---|---|
|Linux kernel|❌ Emulated|✅ Real Linux kernel|
|Execution model|Pico processes|Virtual machine|
|Pico Provider|✅ Yes (`LxCore.sys`)|❌ No|
|Syscall handling|Translated → NT|Native Linux|
|Compatibility|Partial|Near-100%|
|Architecture|OS-personality layer|Lightweight VM|

---

## 2. What happened to Pico in WSL 2?

WSL 2 completely **abandons** the Pico execution model.

Instead:

- Windows launches a **managed Hyper-V VM**
    
- The VM runs:
    
    - A **real Linux kernel**
        
    - Standard Linux process model
        
- Linux syscalls never enter:
    
    - Windows Executive
        
    - NT syscall dispatcher
        
    - Pico callbacks
        

So:

> There is no Pico Provider registered when WSL 2 is used.

---

## 3. Why Microsoft moved away from Pico

Pico was elegant, but had hard limits:

### Problems with Pico (WSL 1)

- Incomplete syscall coverage
    
- Subtle behavior mismatches
    
- Kernel corner cases were extremely hard to emulate
    
- High maintenance cost
    
- Hard to keep up with evolving Linux kernel APIs
    

### Benefits of WSL 2

- Real Linux kernel → no syscall translation
    
- Full compatibility (Docker, systemd, eBPF, etc.)
    
- Better filesystem semantics
    
- Less NT kernel complexity
    

In short:

> Emulating Linux inside NT was harder than running Linux itself.

---

## 4. Where Pico processes still exist today

Even though WSL 2 doesn’t use Pico:

- Pico infrastructure **still exists** in Windows
    
- Used historically by:
    
    - WSL 1
        
    - Research / internal components
        
- Not used by:
    
    - Win32 apps
        
    - WSL 2
        
    - Typical third-party software
        

As of today:

- Only **one Pico Provider** can exist
    
- On modern systems, it’s usually **unused**
    

---

## 5. Important security / internals implication

### If you see Pico processes on a system:

- It almost certainly means:
    
    - WSL 1 is enabled
        
    - Or legacy compatibility mode
        

### On WSL 2 systems:

- You should see:
    
    - `vmmem`
        
    - `vmmemWSL`
        
- **No Pico processes**
    
- **No LxCore.sys acting as a provider**
    

This is a clean detection signal.

---

## 6. Mental model to keep forever

`WSL 1:  Linux personality inside Windows kernel (Pico) WSL 2:  Linux kernel inside a VM (Hyper-V)`

---

## 7. One-line takeaway

> WSL 2 does not use Pico processes or Pico Providers; it runs a real Linux kernel inside a managed VM, completely bypassing the NT syscall and Executive layers.

If you want next, we can:

- Compare **syscall flow diagrams** (WSL 1 vs WSL 2)
    
- Map **ETW / Sysmon signals** for WSL 2
    
- Discuss **security tradeoffs** of VM vs Pico
    
- Explain **why systemd works only in WSL 2**
    

Just say the word 👌