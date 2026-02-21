## 🔹 Thread Creation Overview

- A thread is created by a process (usually from an existing running thread, like `main`).
    
- Request flows:
    
    - **User mode → Windows Executive → Kernel**
        
    - Process Manager allocates a **thread object**
        
    - Kernel initializes **KTHREAD (Thread Control Block)**
        
- Most thread-creation APIs eventually call:
    
    - `CreateRemoteThreadEx` (in `Kernel32.dll`)
        

---

## 🔹 Internal Steps in `CreateRemoteThreadEx`

### 1️⃣ Convert Parameters

- Converts Win32 API parameters to **native NT flags**
    
- Builds:
    
    - `OBJECT_ATTRIBUTES` structure (for kernel object creation)
        

---

### 2️⃣ Build Attribute List

- Creates attribute list containing:
    
    - **Client ID (Thread ID + Process ID)**
        
    - **TEB address (Thread Environment Block)**
        

---

### 3️⃣ Determine Target Process

- Checks if thread is created:
    
    - In current process (`GetCurrentProcess()` → pseudo handle `-1`)
        
    - Or another process (remote thread)
        
- If unclear:
    
    - Calls `NtQueryInformationProcess` to verify
        

---

### 4️⃣ Transition to Kernel

- Calls:
    
    - `NtCreateThreadEx` (in `Ntdll.dll`)
        
- Switches from **user mode → kernel mode**
    

---

### 5️⃣ Executive Thread Creation

Inside kernel:

- Creates and initializes:
    
    - **User-mode thread context** (architecture-specific)
        
- Calls:
    
    - `PspCreateThread`
        
- Creates thread object in **suspended state**
    
- Returns back to user mode
    

---

### 6️⃣ Activation Context Setup

Back in `CreateRemoteThreadEx`:

- Allocates **activation context**
    
    - Used for **side-by-side assemblies (SxS)**
        
- Saves activation stack pointer in:
    
    - Thread’s **TEB**
        

---

### 7️⃣ Resume Thread (If Not Suspended)

- If `CREATE_SUSPENDED` flag NOT set:
    
    - Thread is resumed
        
    - Eligible for scheduling
        
- When thread starts:
    
    - Performs thread initialization (similar to process Stage 7)
        
    - Finally calls:
        
        - **User-specified start address**
            

---

### 8️⃣ Return to Caller

- Returns:
    
    - **Thread handle**
        
    - **Thread ID**
        

---

## 🔹 Key Objects Involved

- **Thread Object** (Executive object)
    
- **KTHREAD** (Kernel thread structure)
    
- **TEB** (User-mode thread environment block)
    
- **User-mode thread context**
    
- **Activation context**


```
CreateRemoteThreadEx (User Mode)
        ↓
NtCreateThreadEx (Ntdll)
        ↓
NtCreateThreadEx (Kernel)
        ↓
PspCreateThread
        ↓
Thread created (Suspended)
        ↓
Resume (unless CREATE_SUSPENDED)
        ↓
Thread runs → Initialization → Start routine

```