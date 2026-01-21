## Windows Containers (Server Silos) — Notes

### Motivation & Background

- Cloud computing increased demand for **portable, lightweight back-end deployments**.
    
- Containers address:
    
    - Portability across cloud providers and environments
        
    - Faster deployment than virtual machines
        
    - Lower resource overhead than full virtualization
        
- Docker originated on Linux; Microsoft brought Docker support to Windows 10.
    

---

## Windows Container Types

Windows supports two container modes:

### 1. Hyper-V Containers

- Fully isolated
    
- Use hardware virtualization
    
- Heavier, higher security
    
- Supported on client and server
    

### 2. Server Silo Containers

- Lightweight, OS-isolated
    
- Share **host kernel and drivers**
    
- Separate user-mode environment
    
- Supported only on Windows Server
    
- Trade-off: **less isolation, better performance**
    

---

## Job Objects and Silos

- **Server silos are built on top of job objects**
    
- A silo is effectively a **“super-job”** with extra isolation rules
    
- Jobs that support silos are called **hybrid jobs**
    
- Two silo types:
    
    - Application silos (Desktop Bridge)
        
    - **Server silos** (Docker containers)
        

---

## Core Silo Isolation Mechanism

### Object Manager Namespace Isolation

- Each server silo has its own **root object namespace**
    
- Controls access to:
    
    - Files
        
    - Registry keys
        
    - Events, mutexes
        
    - RPC ports
        
    - Named objects
        
- Isolation achieved by:
    
    - Cloning objects
        
    - Symbolic linking to host objects
        
    - Creating silo-private objects
        

---

## User-Mode Isolation Components

Server silo isolation combines namespace isolation with:

- **Base OS image (WIM)**
    
    - Server Core or Nano Server
        
- **Host Ntdll.dll**
    
    - Required because system calls go directly to host kernel
        
- **Sandboxed file system**
    
    - Provided by `Wcifs.sys`
        
    - Temporary changes discarded on shutdown
        
- **Sandboxed registry**
    
    - Provided by `VReg`
        
    - Separate registry hives per container
        
- **Session Manager (Smss.exe)**
    
    - Extended to create **container service sessions**
        

---

## Kernel-Level Silo Isolation Boundaries

Each server silo has isolated versions of:

- **Silo user shared data**
    
    - Custom system paths, session ID, foreground PID, SKU
        
    - Host `KUSER_SHARED_DATA` still exists → partial host leakage
        
- **Object directory root**
    
    - Separate `\SystemRoot`, `\Device`, DOS devices, sessions
        
- **API Set mappings**
    
    - Based on base OS image, not host OS
        
- **Logon sessions**
    
    - SYSTEM + Anonymous + virtual service account LUID
        
- **ETW tracing**
    
    - Isolated logging contexts per silo
        

---

## Silo Contexts

- Kernel and drivers can store **per-silo contextual data**
    
- Implemented via:
    
    - `PsCreateSiloContext`
        
    - Slot-based storage (Silo Local Storage, SLS)
        
- Storage:
    
    - 32 built-in slots
        
    - 256 expansion slots
        
- Each silo:
    
    - Has its own SLS array
        
    - Uses same slot indices but different data
        
- Used by:
    
    - Object Manager
        
    - Security Reference Monitor (SRM)
        
    - Configuration Manager
        
    - Networking drivers (AFD.sys)
        

---

## Host Silo

- Host OS is treated as a **special root silo**
    
- Implemented using `PspHostSiloGlobals`
    
- Allows silo-aware kernel code to function uniformly
    
- `NULL` silo pointer = host silo
    

---

## Silo Monitors

- Kernel drivers can register **silo lifecycle callbacks**
    
- APIs:
    
    - `PsRegisterSiloMonitor`
        
    - `PsStartSiloMonitor`
        
    - `PsUnregisterSiloMonitor`
        
- Monitors:
    
    - Track silo creation/termination
        
    - Insert or update silo contexts
        
    - Allocate additional context slots
        
- Permanent contexts can persist beyond silo lifetime
    

---

## Server Silo Creation Flow

1. **CreateJobObject**
    
    - Job gets a unique Job ID (JID)
        
2. **SetInformationJobObject**
    
    - Mark job as a silo
        
    - Allocate SLS
        
3. **Create virtual object namespace**
    
    - Requires TCB privilege
        
    - Root created as `\Silos\<JID>\`
        
4. **Convert to server silo**
    
    - Initializes:
        
        - Silo shared data
            
        - API set mappings
            
        - SRM / security contexts
            
5. **Boot the silo**
    
    - Smss.exe clones itself
        
    - Launches:
        
        - Csrss.exe
            
        - Wininit.exe
            
        - Lsass.exe
            
        - Services.exe
            
    - Uses virtual service account LUID
        

---

## Driver & Device Support

- Drivers must be **silo-aware**
    
- Use silo monitors to create:
    
    - Per-silo device objects
        
- `PsAttachSiloToCurrentThread`:
    
    - Temporarily executes code in silo context
        
- Needed for:
    
    - Named pipes
        
    - Networking
        
    - Other kernel objects
        

---

## Application Execution Model

- No GUI support
    
- No RDP into containers
    
- **Command-line only**
    
- Uses **CExecSvc.exe**:
    
    - Communicates via named pipes
        
    - Emulates console I/O
        
    - Handles `docker exec`, `docker cp`
        

---

## Container Template (wsc.def)

- Defines:
    
    - Object namespace rules
        
    - Registry virtualization
        
    - File system mappings
        
    - Device access
        
    - Network isolation
        
- Located at:
    
    `%SystemRoot%\System32\Containers\wsc.def`
    
- Uses:
    
    - Symbolic links
        
    - Namespace cloning
        
- Allows selective sharing of host resources
    
- Future silo types could use different templates
    

---

## Key Takeaways

- Server silos provide **lightweight isolation** using:
    
    - Job objects
        
    - Namespace virtualization
        
    - Per-silo kernel contexts
        
- Share kernel and drivers → performance benefit, weaker isolation
    
- Containers rely on:
    
    - Extensive kernel changes
        
    - Silo-aware drivers
        
    - Job object extensions
        
- The host OS itself is treated as a **logical silo** for consistency