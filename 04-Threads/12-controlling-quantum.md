# 📌 Controlling the Quantum — Summary Notes

---

# 1️⃣ Global Quantum Settings

You can globally choose between:

- **Short quantum** → 2 clock ticks (default on Client)
    
- **Long quantum** → 12 clock ticks (default on Server)
    

Only one mode can be selected system-wide.

🔎 Exception:

- **Job objects** can override quantum for processes inside the job (on long-quantum systems).
    

---

# 2️⃣ GUI Configuration

Path:

This PC → Properties → Advanced System Settings →  
Performance Settings → Advanced

Two options:

### 🔹 Programs

- Short, **variable** quantums
    
- Default for:
    
    - Client Windows
        
    - Terminal Server (application mode)
        
- Optimized for responsiveness
    

### 🔹 Background Services

- Long, **fixed** quantums
    
- Default for:
    
    - Server systems
        
- Optimized for throughput & reduced context switching
    

---

# 3️⃣ Variable vs Fixed Quantums

## Variable Quantums (Client Default)

- Uses **PspVariableQuantums table**
    
- Copied into:
    
    - `PspForegroundQuantum`
        
- Algorithm decides quantum based on:
    
    - Whether process owns **foreground window**
        

If process is:

- Background → default quantum (index 0)
    
- Foreground → boosted quantum (based on priority separation)
    

---

# 4️⃣ Priority Separation & Foreground Boost

- Foreground threads get:
    
    - Priority boost (up to +2)
        
    - Extra quantums
        

Rule:

- Each extra priority level → +1 quantum
    

Default:

- Priority separation = 2
    
- Foreground threads get:
    
    - 2 extra quantums
        
    - Total = 3 quantums
        

---

# 5️⃣ Practical Effect (Client System)

When you switch windows:

- Foreground process:
    
    - Quantum = 6 clock ticks
        
- Background processes:
    
    - Quantum = 2 clock ticks
        

➡ Foreground process gets proportionally more CPU

This improves UI responsiveness.

---

# 6️⃣ Quantum Value Table (Internal Units)

Recall:

- 1 clock tick = 3 quantum units
    

### Short (Client)

|Index|Units|
|---|---|
|0|6|
|1|12|
|2|18|

### Long (Server)

|Index|Units|
|---|---|
|0|36|
|1|36|
|2|36|

- Long quantums are fixed
    
- No foreground extension
    

---

# 7️⃣ Registry Control

Registry key:

HKLM\SYSTEM\CurrentControlSet\Control\PriorityControl

Value:

Win32PrioritySeparation

6-bit value divided into three 2-bit fields:

---

## 🔹 Short vs Long

- 1 → Long
    
- 2 → Short
    
- 0 or 3 → System default
    

---

## 🔹 Variable vs Fixed

- 1 → Variable quantums
    
- 0 or 3 → System default
    

---

## 🔹 Priority Separation

- Value up to 2
    
- Determines:
    
    - Foreground priority boost
        
    - Quantum index
        

Stored in:

- `PsPrioritySeparation`
    

---

# 8️⃣ Default Registry Values

### Client Systems:

- Value = 2
    
- Short quantums
    
- Variable
    
- Priority separation = 2
    

### Server (Application Server Mode):

- Value = 0x26
    
- Behaves like client (Programs mode)
    

### Server (Normal Mode):

- Long
    
- Fixed
    
- No foreground boost
    

---

# 9️⃣ Special Cases

- Threads in **Idle priority class process**:
    
    - Always get single quantum
        
    - Ignore all quantum settings
        

---

# 🔟 Kernel Structures Affected

Changing setting modifies:

- `PsPrioritySeparation`
    
- `PspForegroundQuantum`
    
- `QuantumReset` in `_KPROCESS`
    

Example values:

### Programs Mode:

- PrioritySeparation = 2
    
- PspForegroundQuantum = 06 0C 12
    
- QuantumReset = 6
    

### Background Services Mode:

- PrioritySeparation = 0
    
- PspForegroundQuantum = 24 24 24
    
- QuantumReset = 36
    

---

# 🧠 Big Picture Understanding

- Client systems prioritize **responsiveness**
    
- Server systems prioritize **throughput**
    
- Foreground boost = priority + longer quantum
    
- Registry controls both:
    
    - Length
        
    - Variability
        
    - Priority separation
        

---

# 🎯 Key Concepts to Remember

- Two global modes: Programs vs Background Services
    
- Client = short + variable
    
- Server = long + fixed
    
- Foreground process can get 3× CPU time (client)
    
- Controlled via `Win32PrioritySeparation`
    
- Idle-class processes ignore quantum tuning
