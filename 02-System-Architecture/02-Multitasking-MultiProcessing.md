# Symmetric Multiprocessing (SMP) & Scalability – Windows Internals Notes

## 1. Multitasking vs Multiprocessing

- **Multitasking**:
    
    - Single CPU
        
    - OS time-slices CPU among threads
        
    - Appears concurrent, but isn’t truly parallel
        
- **Multiprocessing**:
    
    - Multiple CPUs
        
    - Threads execute **simultaneously**
        
    - One thread per processor
        

---

## 2. Windows SMP Model

### Symmetric Multiprocessing (SMP)

- Windows is an **SMP OS**
    
- Characteristics:
    
    - **No master processor**
        
    - OS kernel code and user threads can run on **any CPU**
        
    - All processors share **one physical memory space**
        
- Contrast: **ASMP (Asymmetric Multiprocessing)**
    
    - One CPU runs kernel code
        
    - Other CPUs run only user code
        
    - Windows does **not** use this model
        

---

## 3. Processor System Types Supported by Windows

### 1. Multicore

- Multiple **physical cores** on the same CPU package
    
- Windows treats each core as a discrete processor
    
- Special handling for:
    
    - Licensing
        
    - Cache topology optimization
        
- Important for cache-aware scheduling
    

---

### 2. Simultaneous Multithreading (SMT)

- Example:
    
    - Intel Hyper-Threading
        
    - AMD Zen SMT
        
- Key points:
    
    - One physical core → **2 logical processors**
        
    - Logical processors:
        
        - Separate CPU state
            
        - Shared execution engine + cache
            
- Marketing confusion:
    
    - “4 cores, 8 threads” = **8 logical processors**
        
- Scheduler optimizations:
    
    - Prefer idle **physical cores**
        
    - Avoid loading logical siblings if one is already busy
        

---

### 3. NUMA (Non-Uniform Memory Access)

- CPUs grouped into **nodes**
    
- Each node has:
    
    - Local processors
        
    - Local memory
        
- Memory access:
    
    - Local node memory = faster
        
    - Remote node memory = slower
        
- Windows behavior:
    
    - Still **SMP**
        
    - Schedules threads near their memory
        
    - Prefers node-local memory allocation
        
    - Falls back to other nodes if necessary
        

---

### 4. Heterogeneous Multiprocessing (ARM – big.LITTLE)

- Used on ARM platforms
    
- Cores differ in:
    
    - Performance
        
    - Power consumption
        
- Still SMP:
    
    - All cores run the same instructions
        
- Purpose:
    
    - Balance performance vs power usage
        
- Windows supports **heterogeneous scheduling policies**
    
    - High performance (big cores)
        
    - Balanced
        
    - Low-power (little cores only)
        
- Scheduler integrates with power manager
    

---

## 4. Processor Limits & Groups

### Affinity Masks

- Windows tracks CPUs using a **bitmask**
    
- Mask size = native word size:
    
    - 32-bit or 64-bit
        
- Original limitation:
    
    - Max CPUs = bits in a native word
        

---

### Processor Groups

- Introduced to scale beyond native bitmask limits
    
- A **processor group**:
    
    - Set of CPUs addressable by one affinity mask
        
- Key facts:
    
    - Max groups: **20**
        
    - Max logical processors: **640**
        
- Behavior:
    
    - Modern apps: enumerate groups
        
    - Legacy apps: see **only their current group**
        
- Kernel + apps explicitly select group during affinity ops
    
- Grouping also related to NUMA topology
    

---

## 5. Licensing & Processor Limits

- Licensed processor count depends on **Windows edition**
    
- Stored in:
    
    `%SystemRoot%\ServiceProfiles\LocalService\AppData\Local\ Microsoft\WSLicense\tokens.dat`
    
- Key variable:
    
    - `kernel-RegisteredProcessors`
        

---

## 6. Scalability Challenges in SMP Systems

- Problems:
    
    - Resource contention
        
    - Lock contention
        
    - Cache coherency
        
- Windows design goals:
    
    - Correctness
        
    - Parallelism
        
    - Performance
        

---

## 7. Windows SMP Scalability Features

- OS code can run:
    
    - On **any processor**
        
    - On **multiple processors simultaneously**
        
- Multiple threads per process
    
- **Fine-grained synchronization**:
    
    - Spinlocks
        
    - Queued spinlocks
        
    - Pushlocks
        
- Scalable programming primitives:
    
    - I/O Completion Ports (IOCP)
        

---

## 8. Evolution of Kernel Scalability

- **Windows Server 2003**
    
    - Per-CPU scheduling queues
        
    - Fine-grained scheduler locks
        
- **Windows 7 / Server 2008 R2**
    
    - Eliminated global scheduler lock during waits
        
- Similar improvements in:
    
    - Memory manager
        
    - Cache manager
        
    - Object manager
        

---

## 9. Client vs Server Windows Editions

### Editions Overview

- **Windows 10 client**:
    
    - Home, Pro, Education, Enterprise, LTSB
        
    - Mobile, IoT variants
        
- **Windows Server 2016**:
    
    - Datacenter, Standard, Essentials, Hyper-V Server, etc.
        

---

## 10. Key Differences Between Client & Server

- Core kernel files are **identical**
    
- Differences controlled by:
    
    - Licensing policies
        
    - Registry configuration
        
- Differences include:
    
    - Processor limits
        
    - Memory limits
        
    - Hyper-V container support
        
    - Network connection limits
        
    - Feature availability (BitLocker, Hyper-V, AppLocker, etc.)
        
    - Default performance tuning
        

---

## 11. Product Type Detection

### Registry Keys

`HKLM\SYSTEM\CurrentControlSet\Control\ProductOptions`

- **ProductType values**:
    
    |Edition|Value|
    |---|---|
    |Client|WinNT|
    |Server (DC)|LanmanNT|
    |Server (non-DC)|ServerNT|
    
- Query methods:
    
    - User mode: `VerifyVersionInfo`
        
    - Kernel mode: `RtlGetVersion`, `RtlVerifyVersionInfo`
        

---

### ProductPolicy

- Cached copy of licensing data from `tokens.dat`
    
- Controls feature enablement per edition
    

---

## 12. Operational Differences (Client vs Server)

### Server Editions

- Optimized for:
    
    - Throughput
        
    - Scalability
        
- Larger:
    
    - System pools
        
    - System caches
        
    - Worker thread counts
        
- Scheduler:
    
    - Longer default time slice
        

### Client Editions

- Optimized for:
    
    - Interactive responsiveness
        
- Scheduler:
    
    - Shorter time slice
        
- Memory manager favors:
    
    - Foreground applications
        

---

## 13. Key Takeaways

- Windows SMP design:
    
    - No master CPU
        
    - Full kernel concurrency
        
- Scheduler is:
    
    - NUMA-aware
        
    - SMT-aware
        
    - Power-aware (ARM)
        
- Processor groups enable massive scaling
    
- Client vs server differences are **policy-driven**, not kernel-driven