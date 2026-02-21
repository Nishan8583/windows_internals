# 1️⃣ Why Are There Two Types of Priority Levels?

Windows has **32 internal priority levels (0–31)**.

They are split into:

|Type|Range|Purpose|
|---|---|---|
|**Variable**|0–15|Normal applications|
|**Real-Time**|16–31|Time-critical system or special tasks|

---

## 🔹 Variable Priority (0–15)

This is what **almost all applications use**.

Key property:

> The OS can automatically adjust (boost or lower) these priorities.

Windows dynamically changes them to:

- Improve responsiveness
    
- Prevent starvation
    
- Favor foreground apps
    
- Boost I/O-bound threads
    

Example:

- You click a window → Windows boosts that thread
    
- A thread wakes up after disk I/O → gets a temporary boost
    

These are **dynamic priorities**.

---

## 🔹 Real-Time Priority (16–31)

These are special.

Key property:

> Windows does NOT dynamically adjust them.

They are meant for:

- Hardware control
    
- Audio processing
    
- Industrial systems
    
- Special time-critical services
    

If a real-time thread runs, it will **preempt everything below 16**.

That means:

- It can starve the entire system.
    
- Even mouse/keyboard might lag.
    

That’s why malware running at real-time priority is dangerous.

---

# 2️⃣ Why Split Them?

Because Windows needs:

- A flexible area (0–15) for normal programs
    
- A protected, strict area (16–31) for guaranteed execution
    

Think of it like:

0–15  → Adjustable zone (Windows can help manage fairness)  
16–31 → Strict zone (You said it's critical, so Windows obeys)

---

# 3️⃣ What Is Relative Priority?

Now we move to the confusing part.

Windows exposes priorities in two layers:

## Layer 1: Process Priority Class

When you create a process, it gets a **priority class**:

- Idle
    
- Below Normal
    
- Normal
    
- Above Normal
    
- High
    
- Real-Time
    

This determines the **base priority** of threads in that process.

---

## Layer 2: Thread Relative Priority

Inside the process, each thread can have a **relative priority**.

Relative means:

> It is an offset (adjustment) from the process’s base priority.

---

# 🔥 Example (This Makes It Click)

Suppose:

Process Priority Class = **Normal**

Internally, Normal maps to base priority = **8**

Now thread relative priority:

|Relative Level|Delta|
|---|---|
|Normal|0|
|Above Normal|+1|
|Highest|+2|
|Below Normal|-1|
|Lowest|-2|

So:

- Thread A: Normal → 8 + 0 = 8
    
- Thread B: Highest → 8 + 2 = 10
    
- Thread C: Lowest → 8 - 2 = 6
    

All three are in same process, but different final priorities.

---

# 🚨 Special Case: Saturation Values

Two special relative levels:

- Time-Critical → +15
    
- Idle → -15
    

These don’t behave like normal offsets — they map to specific levels.

---

# 4️⃣ Big Picture Flow

When Windows schedules:

1. It looks at the **final thread priority number (0–31)**.
    
2. It picks the highest priority runnable thread.
    
3. That thread runs.
    

The process priority class + relative thread priority is just a way to compute that final number.

---

# 5️⃣ Important Cybersecurity Insight

Why this matters:

- Malware can raise thread priority.
    
- Real-time priority can freeze the system.
    
- Cryptominers often raise priority slightly.
    
- EDRs watch for suspicious priority changes.
    

Understanding this helps you reason:

> “Why is this thread dominating CPU?”

---

# 🧠 Simplified Mental Model

Think of it like this:

- Process Priority Class = Floor of a building
    
- Relative Priority = Room adjustment on that floor
    
- Final Priority = Exact height in the building
    
- Scheduler = Elevator that always goes to highest floor
    

---

# 🎯 The Key Difference

|Variable|Real-Time|
|---|---|
|0–15|16–31|
|Dynamically adjusted|Not dynamically adjusted|
|Normal apps|Critical apps|
|Safe|Can starve system|
