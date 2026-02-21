## **Windows Executive and Kernel – Notes**

### **1. Windows Executive (Upper Layer of Ntoskrnl.exe)**

- Provides higher-level OS services; kernel is the lower layer.
    
- **Types of functions:**
    
    1. **User-callable system services**: Exported via `Ntdll.dll` (e.g., `NtCreateFile`).
        
    2. **Device driver functions**: Accessed via `DeviceIoControl` (e.g., `ConDrv.sys` for console).
        
    3. **Kernel-mode only, documented in WDK**: `Io*` (I/O manager), `Ex*` (executive support).
        
    4. **Kernel-mode exported, undocumented**: e.g., `Inbv*` for boot video driver.
        
    5. **Global, not exported**: Internal support (`Iop*` – I/O, `Mi*` – memory).
        
    6. **Internal module-only functions**: Used exclusively by executive/kernel.
        
- **Major components:**
    
    - **Configuration Manager**: Manages system registry.
        
    - **Process Manager**: Creates/terminates processes & threads; adds executive-level semantics.
        
    - **Security Reference Monitor (SRM)**: Enforces local security policies.
        
    - **I/O Manager**: Device-independent I/O, dispatches to drivers.
        
    - **Plug and Play (PnP) Manager**: Loads drivers, assigns resources, sends device change notifications.
        
    - **Power Manager**: Coordinates power events (PPM, PoFx), reduces CPU/device power when idle.
        
    - **Windows Driver Model (WDM) & WMI**: Device driver telemetry & management.
        
    - **Memory Manager**: Virtual memory, process private address space, cache manager support.
        
    - **Cache Manager**: Improves disk I/O performance by caching and deferring writes.
        
- **Executive Support Functions:**
    
    1. **Object Manager**: Creates/manages OS objects (process, threads, sync objects).
        
    2. **ALPC**: Local communication between client/server processes.
        
    3. **Run-time library functions**: Strings, arithmetic, conversions.
        
    4. **Executive support routines**: Memory allocation, interlocked access, synchronization.
        
- **Other infrastructure routines:**
    
    - Kernel debugger, User-Mode Debugging Framework, Hypervisor & VBS support, Errata Manager, Driver Verifier, ETW, WDI, WHEA, FSRTL, Kernel Shim Engine.
        

---

### **2. Windows Kernel (Lower Layer of Ntoskrnl.exe)**

- Implements core OS mechanisms; avoids policy decisions (executive handles most policies).
    
- Provides **thread scheduling, synchronization, interrupt/exception handling**, and architecture-dependent hardware support.
    
- Written mainly in **C**, with assembly for architecture-specific tasks.
    
- **Kernel objects**:
    
    - Low-level objects used internally by kernel/executive.
        
    - **Control objects**: APC, DPC, interrupt objects.
        
    - **Dispatcher objects**: Thread, mutex (`mutant`), event, semaphore, timers.
        
- **Processor-specific structures**:
    
    - **KPCR**: Stores processor info (IDT, TSS, GDT, interrupts, shared controller state).
        
    - **KPRCB**: Embedded in KPCR; stores scheduling info, DPC queue, CPU topology, cache sizes, time accounting, statistics (I/O, cache, memory, DPC).
        
- **Hardware support**:
    
    - Abstracts executive/drivers from hardware variation.
        
    - Provides portable interfaces; architecture-specific code exists for context switching, cache support, virtual 8086 (x86), translation buffers.
        
- **HAL (Hardware Abstraction Layer)**:
    
    - `Hal.dll` hides hardware-specific I/O, interrupts, multiprocessor communication.
        
    - HAL extensions allow support for new/IoT hardware (custom signed DLLs, limited APIs).
        
    - Ensures portability across architectures & devices.
        

---

### **3. Device Drivers**

- **Kernel-mode drivers** (`*.sys`), interface between I/O Manager and hardware via HAL.
    
- Run in three contexts: user thread, kernel thread, interrupt context.
    
- **Types**:
    
    1. **Hardware drivers**: Bus, storage, HID, etc.
        
    2. **File system drivers**: Handle file-oriented I/O.
        
    3. **File system filter drivers**: Disk mirroring, encryption, virus scanning.
        
    4. **Network redirectors/servers**
        
    5. **Protocol drivers**: TCP/IP, NetBEUI, IPX/SPX.
        
    6. **Kernel streaming filter drivers**: Audio/video signal processing.
        
    7. **Software drivers**: Kernel-only utilities (e.g., Sysinternals).
        
- **Driver Models**:
    
    - **WDM**: Windows 2000+, supports PnP & Power. Driver types: Bus, Function, Filter.
        
    - **WDF (KMDF/UMDF)**:
        
        - Simplifies WDM driver development.
            
        - KMDF: Kernel-mode drivers, abstracts WDM complexity.
            
        - UMDF: User-mode drivers, crashes isolated to service process.
            
        - UMDF 1.x (C++/COM), UMDF 2.x (similar to KMDF).
            
    - **Universal Windows Drivers**: Write once, run across devices (IoT, Xbox, phones, desktops).
        
- **Viewing drivers**:
    
    - System Information (`Msinfo32.exe` → Software Environment → System Drivers).
        
    - Process Explorer → System process → DLL view.
        
    - Registry path: `HKLM\SYSTEM\CurrentControlSet\Services` (type code 1 = kernel-mode driver).
        

---

### **4. Exploring Undocumented Interfaces**

- Examine exports/global symbols in `Ntoskrnl.exe`, `Hal.dll`, `Ntdll.dll` for insight into internal OS functions.
    
- Naming convention: `<Prefix><Operation><Object>`
    
    - Prefix → component (`Ex` = executive, `Ke` = kernel)
        
    - Operation → action
        
    - Object → resource/object
        
- **Common prefixes** (partial):
    
    - `Ex` – Executive support routines
        
    - `Ke` – Kernel routines
        
    - `Nt` – System services via Ntdll.dll
        
    - `Ob` – Object manager
        
    - `Io` – I/O manager
        
    - `Mm` – Memory manager
        
    - `Ps` – Process support
        
    - `Po` – Power manager
        
    - `Pp` – PnP manager
        
    - `Se` – Security Reference Monitor
        
    - `Etw` – Event Tracing for Windows
        
    - `FsRtl` – File System Runtime Library
        
    - `Kse` – Kernel Shim Engine