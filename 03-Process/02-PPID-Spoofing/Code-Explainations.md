## 1️⃣ Structure zero-initialization

`STARTUPINFOEXA siex  = {0}; PROCESS_INFORMATION pi = {0}; SIZE_T size = 0;`

### What this does

- Allocates structures on the stack
    
- Sets **all fields to zero**
    

### Why this matters

Windows APIs are **very strict**:

- Uninitialized fields = undefined behavior
    
- Zeroing ensures unused fields are ignored safely
    

---

## 2️⃣ Why `STARTUPINFOEXA` exists

`STARTUPINFOEXA siex;`

There are **two** startup structures:

|Structure|Purpose|
|---|---|
|`STARTUPINFO`|Normal process creation|
|`STARTUPINFOEX`|Extended creation with attributes|

PPID spoofing **requires**:

`STARTUPINFOEX + attribute list`

No `STARTUPINFOEX` → no spoofing.

---

## 3️⃣ Setting the structure size (mandatory)

`siex.StartupInfo.cb = sizeof(STARTUPINFOEXA);`

### Why this line is critical

Windows uses this field to:

- Detect structure version
    
- Know how much memory it can safely read
    

If you forget this:

- `CreateProcess` fails
    
- Or worse, behaves unpredictably
    

This is a **very Windows-specific pattern**.

---

## 4️⃣ First call to `InitializeProcThreadAttributeList`

`InitializeProcThreadAttributeList(NULL, 1, 0, &size);`

### What this does

This is a **query call**.

You are asking Windows:

> “How much memory do I need for an attribute list with **1 attribute**?”

### Important details

- First parameter is `NULL` → means _don’t initialize yet_
    
- `1` → number of attributes (we need only parent process)
    
- `size` → Windows writes required size here
    

After this call:

`size = required number of bytes`

This is a **very common Windows API pattern**.

---

## 5️⃣ Allocating the attribute list

`siex.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(     GetProcessHeap(),     0,     size );`

### What this does

- Allocates **exactly the number of bytes Windows requested**
    
- Memory is taken from the **process heap**
    
- Result is a pointer to a `PROC_THREAD_ATTRIBUTE_LIST`
    

### Why heap allocation?

Because:

- Attribute list size is **runtime-determined**
    
- Stack allocation is unsafe here
    
- Windows will write into this buffer
    

---

## 6️⃣ How this fits into PPID spoofing flow

Here’s the **full picture**:

`STARTUPINFOEX  ├── StartupInfo (base info)  └── lpAttributeList       └── PROC_THREAD_ATTRIBUTE_PARENT_PROCESS            └── HANDLE → EPROCESS`

Right now:

- You’ve created the **container**
    
- You have NOT yet populated it
    

The next steps (you already saw) are:

`InitializeProcThreadAttributeList(siex.lpAttributeList, 1, 0, &size); UpdateProcThreadAttribute(...);`

---

## 7️⃣ Threat-hunting insight (important)

This sequence is a **strong behavioral signal**:

- `InitializeProcThreadAttributeList`
    
- `UpdateProcThreadAttribute`
    
- `CreateProcess` with `EXTENDED_STARTUPINFO_PRESENT`
    

These APIs:

- Are **rare in benign apps**
    
- Appear frequently in:
    
    - Malware
        
    - Red-team tooling
        
    - Sandbox evasion code
        

This is why **EDRs hook these**, not `EPROCESS`.

---

## 8️⃣ One-sentence mental model

> **This code builds a special envelope that lets you whisper instructions to the kernel _before_ a process is born.**

PPID spoofing is just **one instruction** you put in that envelope.

---

# What problem this API is solving

Windows needs:

- A **variable-length** structure
    
- With **N attributes**
    
- Whose size depends on:
    
    - Number of attributes
        
    - Internal alignment
        
    - OS version
        

So Windows cannot let _you_ guess the size.

---

# Step-by-step: what each call does

## 1️⃣ First call — **size query only**

`InitializeProcThreadAttributeList(NULL, 1, 0, &size);`

### What this does

- **Does NOT initialize anything**
    
- Does NOT allocate memory
    
- Does NOT create an attribute list
    

It only answers:

> “If I want an attribute list with **1 attribute**, how many bytes do I need?”

Windows writes that answer into `size`.

### Why NULL?

Because:

- There is no buffer yet
    
- You are _asking for size only_
    

This is equivalent to:

`malloc(sizeof(?))`

but Windows decides the `sizeof(?)`.

---

## 2️⃣ Heap allocation — **you allocate memory**

`siex.lpAttributeList = HeapAlloc(     GetProcessHeap(),     0,     size );`

### What this does

- Allocates exactly the number of bytes Windows told you
    
- Nothing is initialized yet
    
- Memory contains garbage
    

Windows **will not** allocate this for you.

---

## 3️⃣ Second call — **actual initialization**

`InitializeProcThreadAttributeList(     siex.lpAttributeList,     1,     0,     &size );`

### What this does

Now Windows:

- Writes internal headers
    
- Sets up internal bookkeeping
    
- Prepares slots for **1 attribute**
    

After this call:

> `lpAttributeList` becomes a valid, empty attribute list

⚠️ You still haven’t added any attributes yet.

---

## 4️⃣ `UpdateProcThreadAttribute` — **add actual data**

`UpdateProcThreadAttribute(     siex.lpAttributeList,     0,     PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,     &hParent,     sizeof(HANDLE),     NULL,     NULL );`

### What this does

This:

- Inserts **one attribute**
    
- Key: `PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`
    
- Value: `hParent` (handle → EPROCESS)
    

Now the list says:

`“When creating the process, use THIS parent.”`

---

## 5️⃣ Why this cannot be merged into one call

Because these steps are fundamentally different:

|Step|Responsibility|
|---|---|
|Query size|Windows|
|Allocate memory|You|
|Initialize structure|Windows|
|Insert attribute|You|

Windows **cannot**:

- Allocate memory on your behalf
    
- Guess stack vs heap
    
- Reallocate dynamically
    

And you **cannot**:

- Know required size ahead of time
    
- Build the structure yourself
    

So the split is unavoidable.

---

# Mental model (this will lock it in)

Think of it like building a shipping container:

1. **Ask how big the container must be**
    
2. **Buy a container of exactly that size**
    
3. **Let the manufacturer install the rails inside**
    
4. **Put your cargo inside**
    

Trying to skip step 1 or 3 breaks everything.

---

# Why Windows APIs look like this (important insight)

This pattern appears everywhere:

- `InitializeSecurityDescriptor`
    
- `GetTokenInformation`
    
- `NtQueryInformationProcess`
    
- `RegQueryValueEx`
    
- `CryptDecodeObject`
    

Once you recognize it, Win32 becomes predictable.

---

# Security / threat-hunting insight

This exact sequence:

`InitializeProcThreadAttributeList HeapAlloc InitializeProcThreadAttributeList UpdateProcThreadAttribute CreateProcess(EXTENDED)`

is a **high-signal process creation pattern**.

That’s why:

- EDRs hook it
    
- Sysmon alone misses intent
    
- PPID spoofing remains stealthy but detectable with correlation
    

---

# One-sentence takeaway

> **The first call asks “how much,” the second says “build it,” and the third says “fill it.”**