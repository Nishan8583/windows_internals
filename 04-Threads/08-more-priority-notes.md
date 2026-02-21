# 📌 Windows Thread Priorities – Summary Notes

---

## 1️⃣ Windows API & Kernel Priority Mapping

- **Process Priority Class**: Real-Time, High, Above-Normal, Normal, Below-Normal, Idle
    
- **Thread Relative Priority**: Time-Critical, Highest, Above-Normal, Normal, Below-Normal, Lowest, Idle
    
- **Final Thread Base Priority** = Process Base + Thread Relative
    
- **Saturation Values**:
    
    - Time-Critical (+15) → always highest possible priority
        
    - Idle (–15) → always lowest possible priority
        
- Formula (for saturation):
    
    - Time-Critical: `(HIGH_PRIORITY + 1) / 2`
        
    - Idle: `-((HIGH_PRIORITY + 1)/2)`
        
- Saturated threads: future changes to process base priority **do not affect them**
    

---

## 2️⃣ Seven Windows API Thread Levels

- From API perspective: 7 levels (6 for High priority class)
    
- Real-Time process class allows any priority **16–31**
    
- Extra values can be set using `SetThreadPriority` with deltas: `–7, –6, –5, –4, –3, 3, 4, 5, 6`
    

**Key Point:** Scheduler **only cares about final thread priority**, not how it was calculated.

---

## 3️⃣ Thread Priority Values

- **Each thread has two priority values**:
    
    - **Base priority**: inherited from process or explicitly set
        
    - **Current (dynamic) priority**: used by scheduler
        
- Dynamic priority changes (boosts) happen only for **variable-range threads (1–15)**
    
- Real-Time threads (16–31) **do not change dynamically**
    

---

## 4️⃣ Process & Thread Base Priority Inheritance

- Thread base priority = process base priority
    
- Process base priority = inherits from parent process (overridable at creation)
    
- Can change process base with:
    
    - `SetPriorityClass()`
        
    - Tools: Task Manager, Process Explorer
        
    - Command line: `start /<priority>`
        
- Changing process base priority shifts **all threads** by same delta; relative thread priorities stay unchanged
    

---

## 5️⃣ Default Priorities

- Most user apps: Normal priority → thread starts at **level 8**
    
- System processes (Session Manager, SCM, LSASS): slightly higher than 8
    

---

## 6️⃣ Real-Time Thread Priorities

- Entering Real-Time range requires **SeIncreaseBasePriorityPrivilege**
    
- Windows kernel threads run in real-time range (16–31)
    
- Threads in Real-Time range:
    
    - Can only run at **16–31**
        
    - Cannot mix dynamic (0–15) and real-time threads in same process via standard API
        
    - Priority changes constrained to standard deltas (–2, –1, 0, +1, +2) unless using CSRSS or another real-time process
        

**Important:** “Real-Time” ≠ true real-time OS. Windows only **gives these threads highest priority**, no guaranteed timing.

---

## 7️⃣ Setting & Viewing Priorities

- Process base priority:
    
    - Change: Task Manager, Process Explorer, `SetPriorityClass`, `start /<priority>`
        
- Thread priorities:
    
    - View/change: Process Explorer, Performance Monitor, WinDbg
        
- Adjusting **individual thread priorities** usually requires developer understanding
    
- To start a process at specific priority from command line:
    

cmd /c start /low Notepad.exe   → starts Notepad at Idle priority

---

## 8️⃣ Threat-Hunting / Security Notes

- Threads can be manipulated for **CPU monopolization**
    
- Real-time threads can **starve system or security services**
    
- Monitoring thread priority changes helps detect:
    
    - Malware raising threads to real-time
        
    - CPU-intensive malicious processes
        
    - Priority boosting abuse
