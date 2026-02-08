## The line in question

```go
copy( 	
	buf[props.LoggerNameOffset:],
	(*(*[1 << 20]byte)(unsafe.Pointer(&sessionName[0])))[:len(sessionName)*2], )
```


---

## First: what problem are we solving?

ETW expects memory to look like this:

```
buf
│
├── EVENT_TRACE_PROPERTIES
│
├── UTF-16 session name   ← LoggerNameOffset points here
│
└── UTF-16 log file name

```

So we must:

1. Take a UTF-16 string (`[]uint16`)
    
2. Treat it as **raw bytes**
    
3. Copy those bytes into `buf` at a precise offset
    

Go **does not let you do this safely**, so we use `unsafe`.

---

## Break it into pieces

### 1️⃣ Destination slice

`buf[props.LoggerNameOffset:]`

This means:

- Start writing **exactly** where ETW expects the session name
- `LoggerNameOffset` is in **bytes**
- This is raw memory placement, not Go string assignment
    

Think:

> “Start copying here.”

---

### 2️⃣ Source: the scary part

`(*(*[1 << 20]byte)(unsafe.Pointer(&sessionName[0])))`

Let’s go inside-out.

#### `&sessionName[0]`

- Address of the first `uint16`
    
- Type: `*uint16`
    

This points to memory like:

`[ 0x004d ][ 0x0079 ][ 0x0053 ][ 0x0000 ]`

(each cell = 2 bytes)

---

#### `unsafe.Pointer(&sessionName[0])`

- Removes type information
- Now it’s just “some memory”
    

---

#### `*([1 << 20]byte)`

This is the key trick.

It says:

> “Pretend this memory is a **huge byte array**.”

Why `1 << 20`?

- Big enough to cover any realistic string
- We will slice it immediately (so we won’t overread)
    

This is **controlled type punning**.

---

### 3️⃣ Slice to correct byte length

`[:len(sessionName) * 2]`

Why `* 2`?

- `sessionName` is `[]uint16`
- Each `uint16` = 2 bytes
- ETW wants bytes, not elements
    

This gives you:

`[]byte{ 4d 00 79 00 53 00 00 00 }`

Which is exactly what Windows expects.

---

### 4️⃣ `copy(...)`

Now Go does a **plain byte copy**:

`source bytes → destination bytes`

No encoding  
No interpretation  
No pointer following

Just raw memory movement.

---

## Why we _must_ do it this way

### ❌ Why not `copy(buf, sessionName)`?

Because:

- `sessionName` is `[]uint16`
    
- `buf` is `[]byte`
    
- Go will not implicitly reinterpret types
    

---

### ❌ Why not `StringToUTF16Ptr`?

Because:

- That returns a **pointer**
    
- ETW does not accept pointers
    
- ETW only understands **offsets inside one buffer**
    

---

### ❌ Why not `binary.Write`?

Because:

- It writes element-by-element
    
- Endianness & padding risks
    
- Less explicit than raw byte copy
    

---

## This is not a Go thing — it’s an ETW thing

Even in **C**, you’d do something similar:

`memcpy(   (BYTE*)props + LoggerNameOffset,   sessionName,   len * sizeof(WCHAR) );`

Go just makes you be _very explicit_ about it.

---

## Mental model (lock this in)

> We are **manually constructing a C memory layout**, byte by byte.

Not strings  
Not slices  
Not structs

**Bytes.**

---

## Why this matters for Windows Internals / security

This pattern appears everywhere in:

- ETW
    
- PEB parsing
    
- Syscall stubs
    
- Kernel IOCTL buffers
    
- Malware loaders
    
- EDR sensors
    

If you understand _this line_, you understand:

- ABI boundaries
    
- Why unsafe exists
    
- Why offsets beat pointers
    
- Why kernel interfaces look “old”
    

---

## TL;DR

That line:

- Reinterprets UTF-16 memory as bytes
    
- Copies the exact byte count
    
- Places it at a precise offset
    
- Builds a kernel-safe ETW buffer
    

It’s ugly **because correctness matters more than beauty here**.