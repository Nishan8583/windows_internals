# 📌 Quantum — Summary Notes

---

# 1️⃣ What Is a Quantum?

- A **quantum** = amount of time a thread can run
    
- When quantum expires:
    
    - Windows checks if another thread of **same priority** is ready
        
    - If yes → context switch
        
    - If no → thread gets another quantum
        

---

# 2️⃣ Default Quantum Length

|System Type|Default|Clock Intervals|
|---|---|---|
|Client Windows|Short|2 clock intervals|
|Server Windows|Long|12 clock intervals|

### Why server quantum is longer:

- Reduces context switching
    
- Lets server threads:
    
    - Wake up
        
    - Handle client request
        
    - Go back to wait state
        
    - Without being preempted
        

---

# 3️⃣ Clock Interval Basics

- Clock interval determined by **HAL**, not kernel
    
- Typical values:
    
    - ~10 ms (older x86 uniprocessor)
        
    - ~15.6 ms (modern x86/x64 multiprocessor)
        
- Stored in:
    
    - `KeMaximumIncrement`
        
- Stored in units of:
    
    - Hundreds of nanoseconds
        

---

# 4️⃣ Important Concept: Quantum ≠ Clock Ticks

Although described in clock intervals:

➡ Thread runtime is actually measured in **CPU cycles**, not clock ticks.

At system startup:

CPU frequency (Hz)  
× seconds per clock interval  
= cycles per clock quantum

Stored in:

- `KiCyclesPerClockQuantum`
    

---

# 5️⃣ How Quantum Expiration Works

Each thread gets a:

- **Quantum reset value** (stored in KPROCESS)
    
- Copied into thread’s KTHREAD
    

When thread runs:

- CPU cycles are charged during:
    
    - Context switches
        
    - Interrupts
        
    - Scheduling events
        

When:

Charged CPU cycles ≥ quantum target

→ Quantum end processing triggered  
→ If same-priority thread exists → context switch

---

# 6️⃣ Internal Quantum Units

Important internal detail:

- 1 clock tick = **3 quantum units**
    
- Quantum unit = 1/3 of a clock tick
    

### Default Reset Values

|System|Clock Intervals|Quantum Units|
|---|---|---|
|Client|2|6 (2 × 3)|
|Server|12|36 (12 × 3)|

Because of this:

- `KiCyclesPerClockQuantum` is divided by 3 internally
    

---

# 7️⃣ Why Quantum Was Stored as 1/3 Tick

In older Windows (pre-Vista):

- Quantum expiration depended directly on clock ticks
    
- If thread ran between ticks and waited before tick fired:
    
    - It might not get charged
        
    - Quantum would not decay properly
        

To fix this:

- Quantum stored as fractional tick
    
- Allowed partial decay on wait completion
    

---

# 8️⃣ Modern Behavior (Vista+)

Now:

- Runtime measured in **CPU cycles**
    
- Not dependent on timer interrupts
    
- No need for partial tick adjustments
    
- More accurate accounting
    

---

# 9️⃣ Clock Interval Experiment Notes

- Can check clock resolution using:
    
    - `GetSystemTimeAdjustment`
        
    - Sysinternals `clockres`
        
- Example default:
    
    - 15.6 ms
        

Multimedia timers:

- Can lower timer interval (e.g., 1 ms)
    
- Causes more frequent scheduler wakeups
    
- May degrade performance
    
- ❗ Does NOT change quantum length
    

---

# 🧠 Big Picture Understanding

- Quantum controls fairness between same-priority threads
    
- Client → responsive
    
- Server → throughput-focused
    
- Accounting is cycle-based (modern systems)
    
- Scheduling decisions are not strictly timer-based
    

---

# 🎯 Key Concepts to Remember

- Quantum = time slice per thread
    
- Default: 2 (client) vs 12 (server) clock intervals
    
- Internally stored as 1/3 clock tick
    
- Measured in CPU cycles, not ticks
    
- Expiration → context switch if equal-priority thread exists
    
- Multimedia timers ≠ quantum change
