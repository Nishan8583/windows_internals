# 1️⃣ “Go prevents type punning” — what that means
- Explaining 
```go
buf := make([]byte, bufferSize)
props := (*EVENT_TRACE_PROPERTIES)(unsafe.Pointer(&buf[0]))
```
- We are allocating a (void *) memory and casting it as a type.

### Type punning = treating the same memory as different types

Classic C example:

```c
int x = 0x41424344; 
char *p = (char*)&x; 
printf("%c", p[0]); // reads same memory as bytes
```

C allows this freely.

---

### Go does **not** allow this by default

In Go, this is illegal:

```go
var x int32 = 0x41424344 
p := (*[4]byte)(unsafe.Pointer(&x)) // requires unsafe
```


Without `unsafe`, Go forbids:

- Interpreting memory as another type
- Reinterpreting struct layouts
- Reading raw bytes from typed values
    
This prevents:

- Endianness bugs
- Undefined behavior
- Compiler reordering assumptions
    

So when I said:

> **Go prevents type punning**

I meant:

> Go does not let you reinterpret memory as a different type unless you explicitly opt out with `unsafe`.

---

# 2️⃣ “Go prevents aliasing violations” — what that means

### Aliasing violation (simple explanation)

Aliasing means:

> Two different pointers point to the same memory

In C, this is common — and dangerous.

Example:

```c
int *a; 
float *b; 
*a = 10; 
printf("%f", *b); // undefined behavior
```


C compilers assume:

> “These pointers do NOT alias”

If they do → undefined behavior.

---

### Go forbids this implicitly

Go’s rules:

- A value has **one effective type**
- The compiler assumes:
    - No unexpected aliasing
    - No overlapping typed pointers

That’s why Go normally disallows:

```go
*(*float32)(unsafe.Pointer(&x))
```


Unless you go through `unsafe.Pointer`, which disables those guarantees.

So:

> **Go prevents aliasing violations**  
> means  
> Go enforces stricter aliasing rules unless you explicitly escape them.

---

# Why not use new instead?
## Short answer

Because **`new(EVENT_TRACE_PROPERTIES)` only allocates the struct**,  
but **ETW requires the struct _plus_ variable-length data in the same contiguous buffer**.

`new()` cannot do that.

---

## What `new(EVENT_TRACE_PROPERTIES)` actually gives you

`props := new(EVENT_TRACE_PROPERTIES)`

This allocates:

`[ EVENT_TRACE_PROPERTIES ]`

That’s it.
- Fixed size
- No room after it
- No guarantee about adjacent memory
- Strings would have to live **somewhere else**
    

---

## What ETW actually expects

ETW expects this **exact layout** in memory:

```
base address
│
├── EVENT_TRACE_PROPERTIES
│
├── UTF-16 LoggerName
│
└── UTF-16 LogFileName

```
All in **one allocation**.
And it finds the strings using:

`base + LoggerNameOffset base + LogFileNameOffset`

---

## Why pointers would not work (this is critical)

Imagine ETW was defined like this:

`LPWSTR LoggerName;`

Then you could do:

`props.LoggerName = syscall.StringToUTF16Ptr(name)`

But ETW explicitly **does not do this**.

Why?

- Kernel-mode code cannot trust user-mode pointers
- ETW sessions may cross processes
- ETW buffers may be shared memory
- Pointers break ABI stability
    

So **offsets are mandatory**, not optional.

---

## Why `new()` fundamentally cannot work

Let’s say you tried anyway:

```go
props := new(EVENT_TRACE_PROPERTIES) 
props.LoggerNameOffset = ???
```


Where would the string live?
- Another allocation?
- Different memory region?
- Different address space?
    

ETW would compute:

`(PBYTE)props + LoggerNameOffset`

And land in **garbage memory**.

---

## Why `make([]byte)` works


```go
buf := make([]byte, totalSize)
props := (*EVENT_TRACE_PROPERTIES)(unsafe.Pointer(&buf[0]))
```


Now you have:
- One contiguous memory region
- Known base address
- Known byte offsets
- Kernel-safe layout

This matches **exactly** what ETW was designed for.

---

## Why Go structs can’t express this naturally

Go structs:

- Cannot have trailing variable-sized fields
- Cannot embed raw byte blobs
- Cannot control exact allocation boundaries
    

C allows hacks like:

```c
struct {   
	EVENT_TRACE_PROPERTIES p;
	WCHAR names[]; 
};
```

Go does not.
So we manually emulate this with `[]byte`.

---

## Analogy (this helps a lot)

Think of `new(EVENT_TRACE_PROPERTIES)` as:

> Renting just the house

ETW requires:

> House + driveway + garage in one property

If you rent them separately, the mailman (kernel) won’t find the garage.

---

## TL;DR (burn this in)

- `new()` allocates **only the struct**
- ETW requires **struct + trailing UTF-16 data**
- ETW uses **offsets, not pointers**
- Therefore **one flat buffer is mandatory**
- `make([]byte)` + `unsafe.Pointer` is the only correct way
    

---

## Why this matters for security / EDR work

This design:

- Prevents pointer confusion
    
- Prevents kernel dereferencing user pointers
    
- Makes ETW resilient against malformed inputs
    

And also:

- Makes Go ETW code _harder_, but safer if done right
    

---

You’ve now hit the exact conceptual boundary between:

> “I know Go”  
> and  
> “I understand Windows internals”

If you want next:

- How attackers abuse ETW buffer mis-sizing
    
- Why ETW survives across reboots
    
- How kernel ETW differs from user providers
    
- How to add the consumer side correctly


---

# Further Detailed Code Explaination

Because **`new(EVENT_TRACE_PROPERTIES)` only allocates the struct**,  
but **ETW requires the struct _plus_ variable-length data in the same contiguous buffer**.

`new()` cannot do that.

---

## What `new(EVENT_TRACE_PROPERTIES)` actually gives you

`props := new(EVENT_TRACE_PROPERTIES)`

This allocates:

`[ EVENT_TRACE_PROPERTIES ]`

That’s it.
- Fixed size
- No room after it
- No guarantee about adjacent memory
- Strings would have to live **somewhere else**
    

---

## What ETW actually expects

ETW expects this **exact layout** in memory:

```
base address
│
├── EVENT_TRACE_PROPERTIES
│
├── UTF-16 LoggerName
│
└── UTF-16 LogFileName

```
All in **one allocation**.
And it finds the strings using:

`base + LoggerNameOffset base + LogFileNameOffset`

---

## Why pointers would not work (this is critical)

Imagine ETW was defined like this:

`LPWSTR LoggerName;`

Then you could do:

`props.LoggerName = syscall.StringToUTF16Ptr(name)`

But ETW explicitly **does not do this**.

Why?

- Kernel-mode code cannot trust user-mode pointers
- ETW sessions may cross processes
- ETW buffers may be shared memory
- Pointers break ABI stability
    

So **offsets are mandatory**, not optional.

---

## Why `new()` fundamentally cannot work

Let’s say you tried anyway:

```go
props := new(EVENT_TRACE_PROPERTIES) 
props.LoggerNameOffset = ???
```


Where would the string live?
- Another allocation?
- Different memory region?
- Different address space?
    

ETW would compute:

`(PBYTE)props + LoggerNameOffset`

And land in **garbage memory**.

---

## Why `make([]byte)` works


```go
buf := make([]byte, totalSize)
props := (*EVENT_TRACE_PROPERTIES)(unsafe.Pointer(&buf[0]))
```


Now you have:
- One contiguous memory region
- Known base address
- Known byte offsets
- Kernel-safe layout

This matches **exactly** what ETW was designed for.

---

## Why Go structs can’t express this naturally

Go structs:

- Cannot have trailing variable-sized fields
- Cannot embed raw byte blobs
- Cannot control exact allocation boundaries
    

C allows hacks like:

```c
struct {   
	EVENT_TRACE_PROPERTIES p;
	WCHAR names[]; 
};
```

Go does not.
So we manually emulate this with `[]byte`.

---

## Analogy (this helps a lot)

Think of `new(EVENT_TRACE_PROPERTIES)` as:

> Renting just the house

ETW requires:

> House + driveway + garage in one property

If you rent them separately, the mailman (kernel) won’t find the garage.

---

## TL;DR (burn this in)

- `new()` allocates **only the struct**
- ETW requires **struct + trailing UTF-16 data**
- ETW uses **offsets, not pointers**
- Therefore **one flat buffer is mandatory**
- `make([]byte)` + `unsafe.Pointer` is the only correct way
    

---

## Why this matters for security / EDR work

This design:

- Prevents pointer confusion
    
- Prevents kernel dereferencing user pointers
    
- Makes ETW resilient against malformed inputs
    

And also:

- Makes Go ETW code _harder_, but safer if done right
    

---

You’ve now hit the exact conceptual boundary between:

> “I know Go”  
> and  
> “I understand Windows internals”

If you want next:

- How attackers abuse ETW buffer mis-sizing
    
- Why ETW survives across reboots
    
- How kernel ETW differs from user providers
    
- How to add the consumer side correctly