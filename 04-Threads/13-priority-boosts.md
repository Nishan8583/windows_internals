# 📌 Priority Boosts — Summary Notes

---

# 1️⃣ What Are Priority Boosts?

Windows dynamically adjusts a thread’s **current (dynamic) priority** to:

- Reduce latency
    
- Improve responsiveness
    
- Prevent starvation
    
- Prevent priority inversion
    

⚠️ Boosts affect **dynamic priority**, not base priority.

---

# 2️⃣ Real-Time Threads Exception

- Threads in priority range **16–31 (real-time)**:
    
    - ❌ Never boosted
        
    - Scheduling remains predictable
        
- Windows assumes real-time users know what they’re doing
    

---

# 3️⃣ Main Boost Scenarios

## A. Scheduler / Dispatcher Events

Purpose: **Latency reduction**

Triggered when synchronization objects are signaled.

Examples:

- APC queued
    
- Event set/pulsed
    
- Timer reset
    
- Mutex released/abandoned
    
- Process exit
    
- Semaphore released
    
- Queue entry inserted/flushed
    
- Thread alerted/suspended/resumed
    
- UMS thread switch
    

---

## B. I/O Completion

Purpose: **Latency reduction**

(Uses APC-based boosts — mentioned as upcoming detail)

---

## C. UI Input

Purpose:

- Latency reduction
    
- Responsiveness improvement
    

(Handled partly through foreground priority separation)

---

## D. Executive Resource (ERESOURCE) Wait Too Long

Purpose:

- Starvation avoidance
    

---

## E. Ready Thread Not Scheduled for Some Time

Purpose:

- Starvation avoidance
    
- Priority inversion avoidance
    

---

# 4️⃣ Multimedia “Boosting” (Special Case)

Client Windows uses:

- **MMCSS driver (mmcss.sys)**
    

Important:

- Not true boosts
    
- Driver directly sets new priorities
    
- Normal boost rules do not apply
    

---

# 5️⃣ Dispatcher Event Boosts

When a dispatch event occurs:

- `KiExitDispatcher` runs
    
- Can apply a boost increment
    

These are called **AdjustUnwait events**

---

# 6️⃣ Unwait Boosts (Most Common)

Goal:  
Reduce latency between:

Thread wakes up → Thread actually runs

### Default Boost Amount

Most synchronization objects:

- Boost increment = **1**
    

Examples:

- MUTANT_INCREMENT
    
- SEMAPHORE_INCREMENT
    
- EVENT_INCREMENT
    

All are set to:

1

---

### Why Boost = 1?

Assumption:

- Releasing and waiting threads at same priority
    

Boosting waiting thread by +1:

- Causes it to preempt releaser immediately
    

---

### Limitations

On single CPU:

- If waiting thread = priority 4
    
- Releasing thread = priority 8
    
- Boost to 5 won’t preempt 8
    

On multiprocessor:

- Higher chance of getting picked by another CPU
    

---

# 7️⃣ Objects With No Boost

No boost when:

- Timer expires
    
- Process is signaled
    

---

# 8️⃣ Lock-Ownership Boosts (More Advanced)

Problem addressed:  
Priority inversion

Scenario:

- Thread A (priority 8) releases lock
    
- Thread B (priority 5) was waiting
    
- Without boost → B might not run soon
    

Solution:  
Use **AdjustBoost** event.

Triggered by:

- `KeSetEventBoostPriority`
    
- `KeSignalGate`
    

Used by:

- ERESOURCE locks
    
- Certain gate locks
    

---

# 9️⃣ How Lock-Ownership Boost Works

Boost amount:

Releasing thread’s priority  
– GUI foreground boost

Before exit:

- `KiRemoveBoostThread`
    
    - Removes extra boost from releasing thread
        
    - Prevents lock-convoy escalation
        

Purpose:  
Avoid situation where:  
Two threads keep boosting each other higher and higher.

---

# 🔟 Pushlocks Exception

Pushlocks:

- Unfair locks
    
- Random ownership
    
- No ownership tracking
    
- No priority boost
    

Reason:

- Lock becomes free immediately
    
- Boosting would cause unnecessary preemption
    

---

# 🧠 Big Picture Concepts

Priority boosts exist to:

- Reduce latency
    
- Improve responsiveness
    
- Prevent starvation
    
- Reduce priority inversion effects
    

But:

- They are heuristic-based
    
- Not perfect
    
- Not strict lock ownership tracking (except certain locks)
    

---

# 🎯 Key Concepts to Remember

- Boosts affect dynamic priority only
    
- Real-time threads are never boosted
    
- Most unwaits = boost of 1
    
- Lock-ownership boost prevents inversion
    
- Multiprocessor systems benefit more from boosts
    
- MMCSS is separate from normal boosting
    
- Pushlocks do not boost
