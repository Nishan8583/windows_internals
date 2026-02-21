# Information for ETW
- General Information on Event tracing is found here https://learn.microsoft.com/en-us/windows/win32/etw/about-event-tracing
- See C code example https://learn.microsoft.com/en-us/windows/win32/etw/configuring-and-starting-the-nt-kernel-logger-session 

### GUID
- In general A GUID (Globally Unique Identifier) is simply:
`A 128-bit number used as a name that is guaranteed (for practical purposes) to be unique.`

- In ETW, a GUID identifies:
```
Who is producing events
OR
What control interface you are talking to
```

---

- Kernel ETW (your case)
- The kernel is not a provider DLL.
- So Microsoft defines a special GUID:
SystemTraceControlGuid
---
6️⃣ Why the kernel needs a GUID at all

You might ask:

“Why not just a special API for kernel tracing?”

Answer:
Because ETW is generic infrastructure.

Everything in ETW is built around:

GUIDs

Sessions

Providers

Consumers

So kernel tracing is implemented as:

A special provider

With a well-known GUID

Controlled by flags

This keeps:

APIs uniform

Tools generic

Backward compatibility intact
---
Translated:

“This GUID identifies the kernel ETW control interface.
Passing this GUID tells ETW that this session controls kernel-level event production.”


In ETW:

Provider GUID → who emits

Control GUID → what subsystem

Event ID → what happened

---

### For golang 
## 1️⃣ Short answer

Because a GUID is **not just a string** — it is a **binary structure with a fixed layout**, defined by Windows and used across:

- Kernel
    
- User mode
    
- COM
    
- RPC
    
- ETW
    

`Data1`, `Data2`, `Data3`, `Data4` exist because **this is the canonical, ABI-stable in-memory representation of a GUID**.

---

## 2️⃣ The canonical GUID structure (the source of truth)

From Windows headers (`guiddef.h`):

`typedef struct _GUID {     unsigned long  Data1;     unsigned short Data2;     unsigned short Data3;     unsigned char  Data4[8]; } GUID;`

This layout:

- Has existed since **early NT**
    
- Is frozen
    
- Cannot change without breaking Windows
    

Go’s `syscall.GUID` **mirrors this exactly**.

---

## 3️⃣ Why not store GUID as 16 raw bytes?

Because Windows predates:

- Modern UUID libraries
    
- Language-agnostic binary formats
    

More importantly:

> Windows needs GUIDs to be **human-readable, binary-stable, and endian-safe**.

The split fields solve that.

---

## 4️⃣ How GUID fields map to the textual form

Take your GUID:

`9e814aad-3204-11d2-9a82-006008a86939`

Mapping:

|Text|Field|
|---|---|
|`9e814aad`|`Data1` (uint32)|
|`3204`|`Data2` (uint16)|
|`11d2`|`Data3` (uint16)|
|`9a82`|`Data4[0..1]`|
|`006008a86939`|`Data4[2..7]`|

So:

`Data1: 0x9e814aad Data2: 0x3204 Data3: 0x11d2 Data4: [8]byte{0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39}`

---

## 5️⃣ Why the split matters (ABI & endianness)

This is **very important**.

GUIDs are defined so that:

- The first 3 fields (`Data1–3`) are **little-endian integers**
    
- `Data4` is **byte-ordered exactly as written**
    

This allows:

- Stable in-memory comparison
    
- Correct string ↔ binary conversion
    
- Cross-language interoperability
    

If Windows had used:

`uint8 guid[16];`

You’d have:

- Endianness ambiguity
    
- Inconsistent parsing
    
- ABI breakage across architectures
    

---

## 6️⃣ Why Go uses `syscall.GUID` instead of `[]byte`

Because Go needs to:

- Pass GUIDs to Windows APIs
    
- Match the exact C memory layout
    
- Avoid marshaling overhead
    

When you do:

`props.Wnode.Guid = SystemTraceControlGuid`

Go is copying **exactly 16 bytes** in the layout Windows expects.

No conversion. No parsing.

---

## 7️⃣ Why this design survives today

This GUID layout is used by:

- COM (`IID_*`, `CLSID_*`)
    
- ETW providers
    
- WMI
    
- RPC
    
- DirectX
    
- Shell APIs
    

Changing it would:

- Break binary compatibility
    
- Break drivers
    
- Break system tools
    

So it stays.

---

## 8️⃣ Mental model to lock in

Think of a GUID as:

> A **binary struct**, not a string  
> With a **canonical memory layout**  
> That just happens to have a string representation

The string is for humans.  
The struct is for the OS.

---

## 9️⃣ Quick sanity check (important)

❓ Why not define it as:

`var g = [16]byte{...}`

❌ Because Windows APIs expect a `GUID*`, not raw bytes  
❌ Because alignment and ordering matter  
❌ Because the ABI expects field semantics

---

## 🔟 Why this matters for ETW and security work

You now understand:

- Why GUIDs are hardcoded
    
- Why copying them byte-for-byte matters
    
- Why ETW, COM, RPC all feel similar
    
- Why malware and EDR tools hardcode GUIDs too
    

This is **core Windows internals knowledge**.

### More
## The code
