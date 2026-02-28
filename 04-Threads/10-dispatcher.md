# 📌 Dispatcher Database — Summary Notes

## 1️⃣ What Is the Dispatcher Database?

- Kernel data structures used for **thread scheduling decisions**.
    
- Tracks:
    
    - Threads that are **ready to execute**
        
    - Which **processors are running which threads**
        

---

## 2️⃣ Scalability in Multiprocessor Systems

### 🔹 Older Windows (pre–Windows 8 / Server 2012)

- **Per-processor ready queues**
    
- **Per-processor ready summary**
    
- Stored in **PRCB (Processor Control Block)**
    

### 🔹 Windows 8+ / Server 2012+

- Introduced **shared ready queue per processor group**
    
- Still keeps per-CPU queues for:
    
    - Threads with **affinity constraints**
        
- Improves:
    
    - Load balancing
        
    - Processor selection decisions
        

---

## 3️⃣ Processor Groups

- Each group shares:
    
    - Ready queues
        
    - Ready summary
        
- Protected by a **spinlock**
    
- To reduce contention:
    
    - **Max group size = 4 logical processors**
        
- If more CPUs exist:
    
    - Multiple groups created
        
    - CPUs distributed evenly
        
    - Example: 6 CPUs → 2 groups of 3
        

---

## 4️⃣ Key Structure: `KSHARED_READY_QUEUE`

- Stored in **PRCB**
    
- Exists per processor
    
- Actively used only by:
    
    - The **first processor of each group**
        
    - Shared with other processors in that group
        

Contains:

- `ReadListHead` → Ready queues
    
- `ReadySummary` → Bitmask
    
- Other scheduling-related data
    

---

## 5️⃣ Ready Queues

- 32 ready queues (one per priority level)
    
- Priority range: **0–31**
    
- Each queue holds threads in **Ready state**
    

---

## 6️⃣ Ready Summary (Performance Optimization)

- 32-bit bitmask
    
- Each bit represents one priority level
    
    - Bit 0 → priority 0
        
    - Bit 1 → priority 1
        
    - …
        
- If bit is set → at least one thread ready at that priority
    

### Why this matters:

Instead of:

- Scanning 32 queues
    

Windows:

- Performs a **single bit scan instruction**
    
- Finds highest priority ready thread
    
- Execution time = **constant (O(1))**
    

➡ Scheduling decision time does **not depend on number of threads**

---

## 7️⃣ Synchronization of Dispatcher Database

### Step 1: Raise IRQL to `DISPATCH_LEVEL (2)`

- Prevents:
    
    - Normal thread preemption (threads run at IRQL 0 or 1)
        

### But that’s not enough:

- Other processors can also:
    
    - Raise IRQL
        
    - Access dispatcher database simultaneously
        

➡ Multiprocessor synchronization requires **additional mechanisms**  
(Handled later via spinlocks etc.)

---

# 🧠 Big Picture Concepts

- Windows scheduling is designed for:
    
    - **Scalability**
        
    - **Low contention**
        
    - **Constant-time scheduling decisions**
        
- Uses:
    
    - Per-group shared queues
        
    - Bitmask-based priority lookup
        
    - IRQL-based protection
        
    - Spinlocks for multiprocessor safety
