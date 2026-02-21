# 📌 Windows System Resource Manager (WSRM)

- **Available on:** Windows Server 2012 R2 Standard and higher (optional component)
    
- **Purpose:** Manage and limit system resources for processes
    

### 🔹 Features

1. Configure **CPU utilization**, **processor affinity**, and **memory limits** (physical & virtual) for processes
    
2. Generate **resource utilization reports** (for accounting or SLAs)
    
3. Apply policies based on:
    
    - Process image name (+ optional command-line args)
        
    - User or group
        
    - Time schedules or always-on
        

### 🔹 CPU Management

- Monitors **CPU consumption** of managed processes
    
- Adjusts **process base priorities** to meet target CPU allocations
    

### 🔹 Memory Management

- **Physical memory:** `SetProcessWorkingSetSizeEx` sets hard working set maximum
    
- **Virtual memory:** WSRM checks private virtual memory; if exceeded:
    
    - Kill process **or** log event
        
- **Exclusions:** AWE memory, large pages, kernel memory (paged/non-paged pool)
    

### 🔹 Security/Monitoring Use

- Can detect memory leaks before system memory is exhausted
    
- Useful for controlling rogue processes or resource-heavy apps
    

---

# 📌 Thread States in Windows

|State|Description|
|---|---|
|**Ready**|Waiting to execute or be swapped in after a wait; only threads in ready state are considered by the dispatcher|
|**Deferred Ready**|Selected to run on a processor but not yet started; reduces time holding per-processor scheduling lock|
|**Standby**|Selected to run next on a processor; only one thread per processor can be standby; can be preempted before execution|
|**Running**|Actively executing after context switch; runs until quantum ends, preempted, yields, waits, or terminates|
|**Waiting**|Thread is blocked: waiting for an object, I/O, or suspension; when wait ends, goes to ready or runs immediately depending on priority|
|**Transition**|Ready for execution but kernel stack is paged out; moves to ready once stack is in memory|
|**Terminated**|Finished executing or killed; thread object may remain if handles exist; can also be killed by `TerminateThread`|
|**Initialized**|Internal creation state for a thread|

![alt](./images 06-thread-state.png)

### 🔹 Key Notes on Transitions

- **Deferred Ready & Standby**: Temporary, almost always short-lived
    
- Threads in these states quickly move to **Ready**, **Running**, or **Waiting**
    
- Performance Monitor can show numeric values of these states
    
- Helps visualize scheduling flow
    

---

# 🧠 Quick Mental Model

1. WSRM controls CPU/memory allocations → adjusts priorities if needed
    
2. Thread scheduler tracks threads in **various states**
    
3. Only **ready threads** compete for CPU
    
4. **Standby** = “next in line”; **Running** = actively executing
    
5. **Waiting** or **Transition** = temporarily out of CPU contention
    
6. **Terminated** = done, may linger if handles exist
