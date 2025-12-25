## Creating Windows Processes (Notes)

### Modern (UWP / Immersive) Processes

- Starting with **Windows 8 / Windows Server 2012**, Windows introduced **modern apps**  
    (also called **UWP apps** or **immersive processes**) to distinguish them from classic desktop apps.
    
- Creating a modern app **cannot be done with a simple `CreateProcess` call**.
    
- Requirements include:
    
    - Specific **command-line arguments**
        
    - An **undocumented process attribute**:
        
        - `PROC_THREAD_ATTRIBUTE_PACKAGE_FULL_NAME`
            
        - Set using `UpdateProcThreadAttribute`
            
        - Value must be the **full Store app package name**
            
- Alternative (documented) way to launch Store apps:
    
    - Use the COM interface **`IApplicationActivationManager`**
        
    - Implemented by `CLSID_ApplicationActivationManager`
        
    - Call `ActivateApplication`
        
    - Requires obtaining the **AppUserModelId**
        
        - Retrieved via `GetPackageApplicationIds`
            
- Typical user launch flow (tapping a tile → CreateProcess) and package naming are covered in **Chapter 9 (Part 2)**.
    

---

### Other Types of Processes (Beyond Desktop & UWP)

Windows supports additional process types that **bypass the Win32 API**:

#### Native Processes

- Example: **Smss (Session Manager)**
    
- Created **directly by the kernel**
    
- Uses **`NtCreateUserProcess`**, not `CreateProcess`
    
- Other examples:
    
    - **Autochk**
        
    - **Csrss (Windows subsystem process)**
        
- Windows applications **cannot create native processes**
    
    - `CreateProcessInternal` rejects images with the **native subsystem type**
        
- Helper API:
    
    - `RtlCreateUserProcess` (exported by `ntdll.dll`)
        
    - Wrapper around `NtCreateUserProcess`
        

#### Minimal & Kernel Processes

- Examples:
    
    - **System process**
        
    - **Memory Compression process**
        
- Created via:
    
    - **`NtCreateProcessEx`**
        
- Some capabilities (e.g., minimal process creation) are **restricted to kernel-mode callers**
    

#### Pico Processes

- Used by providers like **Windows Subsystem for Linux (WSL)**
    
- Created through:
    
    - Internal helper **`PspCreatePicoProcess`**
        
    - Initializes both:
        
        - Minimal process
            
        - Pico provider context
            
- Not exported; accessible only to Pico providers
    

---

### Internal Process Creation Flow

- Despite different system calls:
    
    - `NtCreateUserProcess`
        
    - `NtCreateProcessEx`
        
- **All process creation paths converge internally** on:
    
    - `PspAllocateProcess`
        
    - `PspInsertProcess`
        
- Any process creation method—Win32 API, COM, WMI, PowerShell, or kernel drivers—ultimately ends here.
    

---

### Key Takeaway

> Windows process creation is layered:
> 
> - **Win32 APIs** are only one entry point
>     
> - **Native, minimal, and Pico processes** require lower-level or kernel-only paths
>     
> - **All roads lead to the same internal kernel routines**
>