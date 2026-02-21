# 📌 Windows Scheduling – Summary Notes

---

# 1️⃣ Scheduling Model Overview

### 🔹 Type of Scheduler

- **Priority-driven**
    
- **Preemptive**
    
- Scheduling happens at the **thread level** (not process level)
    

> Processes don’t run — **threads run**.

---

# 2️⃣ Core Scheduling Principles

### ✔ Highest-priority runnable thread runs

- At least one of the highest-priority ready threads always executes
    
- Limited by **processor affinity**
    

---

# 3️⃣ Processor Affinity

### 🔹 Definition

Processor affinity = restriction/preference for which logical processors a thread can run on.

### 🔹 Processor Groups

- Each processor group supports **up to 64 CPUs**
    
- By default:
    
    - Threads run only within their **process’s processor group**
        
- A thread can belong to **only one group**
    
- A process can span multiple groups (multigroup process)
    

### 🔹 Affinity Can Be Changed:

- Via Windows APIs
    
- Via image header affinity mask
    
- Via runtime tools
    
- Via extended group-aware APIs
    

---

# 4️⃣ Quantum (Time Slice)

### 🔹 Definition

Quantum = amount of time a thread runs before another thread at same priority gets CPU.

### 🔹 Quantum varies due to:

- System configuration
    
    - Long vs short quantums
        
    - Fixed vs variable
        
    - Priority separation
        
- Foreground vs background process
    
- Job object configuration
    

---

# 5️⃣ Preemptive Scheduling

Windows is **preemptive**, meaning:

- If a **higher-priority thread becomes ready**, it can interrupt the running thread
    
- A thread can be preempted:
    
    - Before finishing its quantum
        
    - Even before starting its quantum
        

---

# 6️⃣ Kernel Dispatcher

There is no single “scheduler module.”

Scheduling logic is distributed in the kernel and collectively called:

> **The Dispatcher**

---

# 7️⃣ Events That Trigger Scheduling (Dispatching)

Dispatching happens when:

1. A thread becomes ready
    
    - Created
        
    - Leaves wait state
        
2. A running thread:
    
    - Finishes quantum
        
    - Terminates
        
    - Yields
        
    - Enters wait
        
3. A thread’s priority changes
    
4. A thread’s processor affinity changes
    

---

# 8️⃣ Context Switch

### 🔹 Definition

Context switch =

- Save current thread’s CPU state
    
- Load new thread’s CPU state
    
- Begin execution of new thread
    

Happens after dispatcher selects a new thread.

---

# 9️⃣ Thread-Level Scheduling (Not Process-Level)

Scheduling decisions:

- Are made strictly per thread
    
- Ignore which process the thread belongs to
    

Example:

- Process A → 10 runnable threads
    
- Process B → 2 runnable threads
    
- All same priority
    

Each thread gets ~1/12 CPU time

❌ Not 50% per process  
✅ Equal share per thread

---

# 🔟 Windows Priority Levels (Kernel View)

Windows has **32 internal priority levels (0–31)**

### 🔹 Real-Time Levels

- 16–31
    

### 🔹 Variable Levels

- 0–15
    
- Level 0 = Zero page thread (special system thread)
    

---

# 1️⃣1️⃣ Windows API Priority Model

## A) Process Priority Classes

|Class|Internal Index|
|---|---|
|Real-Time|4|
|High|3|
|Above Normal|6|
|Normal|2|
|Below Normal|5|
|Idle|1|

Set using:

- `SetPriorityClass()`
    

---

## B) Thread Relative Priority

Applied as a **delta** to process base priority.

|Level|Delta|
|---|---|
|Time-Critical|+15 (saturation)|
|Highest|+2|
|Above-Normal|+1|
|Normal|0|
|Below-Normal|-1|
|Lowest|-2|
|Idle|-15 (saturation)|

Set using:

- `SetThreadPriority()`
    

---

# 1️⃣2️⃣ How Final Thread Base Priority Is Calculated

1. Process priority class → mapped to base priority via:
    
    - `PspPriorityTable`
        
    - Fixed mapping
        
2. Thread relative priority → applied as delta
    

Example:

- Process base priority = 8
    
- Thread relative priority = +2 (Highest)
    
- Final thread base priority = 10
    

---

# 🧠 Key Concepts to Remember

- Scheduling is **thread-based**
    
- Highest-priority ready thread runs
    
- Scheduler is preemptive
    
- Quantum controls time slice
    
- Affinity restricts processor selection
    
- 32 priority levels (0–31)
    
- API priority ≠ internal kernel priority (mapping happens)
    

---

# 🎯 Threat Hunting Relevance Quick Notes

Important concepts for detection work:

- Priority abuse (real-time threads)
    
- Excessive thread creation
    
- High-priority background threads
    
- Affinity manipulation
    
- Unusual quantum behavior
    
- Frequent context switching
    
- Threads constantly waking from wait state
