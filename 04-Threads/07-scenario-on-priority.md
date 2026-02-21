# 🔹 Scenario Breakdown

**Given:**

- A **real-time process** (priority class = Real-Time)
    
- Windows maps Real-Time process class → base priority = **16** (kernel internal)
    
- The process spawns multiple threads
    

---

## 1️⃣ Thread Base Priority

- By default, **any new thread inherits the process base priority**.
    
- So all new threads start with **base priority = 16**.
    

This is the **starting point**.

---

## 2️⃣ Relative Priority

The process (or code that creates the thread) can **adjust the thread’s relative priority** using:

- `SetThreadPriority()` API
    

Relative priority is a **delta** applied to the thread’s base priority.

### Example Table:

|Relative Thread Priority|Delta|
|---|---|
|Time-Critical|+15 (saturation, can go up to 31)|
|Highest|+2|
|Above-Normal|+1|
|Normal|0|
|Below-Normal|-1|
|Lowest|-2|
|Idle|-15 (saturation, minimum = 0)|

---

### 🔹 How Final Priority is Calculated

Thread Final Priority = Process Base Priority + Relative Thread Priority

- Process Base Priority = 16
    
- Thread Relative = +2 (Highest) → Thread final priority = 16 + 2 = 18
    
- Thread Relative = -1 (Below-Normal) → Thread final priority = 16 - 1 = 15
    

**Constraints:** Windows caps priorities:

- Minimum = 0
    
- Maximum = 31
    

So you cannot go beyond 31 or below 0.  
Also, for real-time threads, it’s usually capped at 31.

---

## 3️⃣ Observations in Your Scenario

1. All threads start at **16** (default inherited).
    
2. Main thread can assign **higher or lower relative priorities**.
    
3. Threads with higher final priority than 16 will **preempt other threads at 16**.
    
4. Threads at **same priority** will take turns using **quantum slices** (round-robin).
    

---

## 4️⃣ Real-Time Thread Behavior

- Priority 16+ = **preempts any variable-priority threads (0–15)**.
    
- Among threads of **same priority**, Windows uses **quantum and round-robin scheduling**.
    
- Quantum expiration, wait states, and voluntary yield still allow context switches.
    

---

## 5️⃣ Quick Example

Process: Real-Time (base priority = 16)  
  
Thread1 → relative = 0 → final = 16  
Thread2 → relative = +2 → final = 18  
Thread3 → relative = -1 → final = 15

- **Thread2** will run first if ready
    
- **Thread1** will run after Thread2 if Thread2 blocks or quantum ends
    
- **Thread3** can only run if all higher-priority threads are not ready
    

---

💡 **Key Insight:**

> Relative priority allows a process to control which of its own threads “wins” among its real-time threads, but they will still always preempt normal-priority threads.
