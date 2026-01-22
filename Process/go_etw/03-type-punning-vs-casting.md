
## Type casting (safe, semantic conversion)

**Type casting** means:

> Convert a value into another type **with meaning preserved**.

Examples:

### In Go
```go
var x int = 10 
var y int64 = int64(x)
```


- Value is converted
- Representation may change
- Compiler understands it
- Always defined behavior
    

Another example:

`var f float64 = 3.14 var i int = int(f) // truncation, but defined`

---

## Type punning (reinterpret the same bits)

**Type punning** means:

> Treat the _same memory bits_ as a completely different type.

Example in C:

```c
float f = 1.0f;
int i = *(int*)&f;  // same bits, different meaning
```

No conversion happens.  
Only interpretation changes.

This is:

- Low-level
- ABI-sensitive
- Often undefined behavior in high-level languages
    

---

## Why type punning is dangerous

Because:

- Compilers optimize assuming types don’t alias
    
- Reinterpreting bits breaks those assumptions
    
- Behavior becomes unpredictable
    

This is why:

- C has “strict aliasing rules”
    
- Go forbids it by default
    
- Rust requires `unsafe` + `repr(C)`
    

---

## Where your ETW code fits

This line:

`props := (*EVENT_TRACE_PROPERTIES)(unsafe.Pointer(&buf[0]))`

Is **type punning**.

You are saying:

> “Interpret these bytes as an EVENT_TRACE_PROPERTIES struct.”

No data conversion occurs.  
Only interpretation changes.

---

## Why Go normally disallows this

Because Go wants to guarantee:

- Memory safety
    
- Predictable optimization
    
- No aliasing bugs
    

So without `unsafe.Pointer`, Go will **refuse**.

---

## Why casting is allowed but punning is not

|Operation|Allowed|Reason|
|---|---|---|
|`int → int64`|✅|Value conversion|
|`float → int`|✅|Defined semantics|
|`[]byte → string`|❌ (without copy)|Representation differs|
|`*byte → *struct`|❌|Type punning|
|`unsafe.Pointer`|✅|Explicit opt-out|

---

## Key rule to remember

> **Casting changes the value**  
> **Punning changes the interpretation**

---

## ETW takeaway

You used:

- **Casting** → almost nowhere
    
- **Type punning** → essential for ABI compatibility
    

And Go forces you to be explicit about it.

---

## TL;DR

- Type casting = safe value conversion
    
- Type punning = reinterpret memory
    
- Go forbids punning unless you use `unsafe`
    
- ETW requires type punning by design
    

This is exactly the kind of thing Windows Internals + EDR work lives on.