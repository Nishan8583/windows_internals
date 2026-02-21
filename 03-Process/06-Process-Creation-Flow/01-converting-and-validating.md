# Step 1: Converting and Validating parameters and flags

## CreateProcessInternalW – Pre-Image Execution Steps (Notes)

### 1. Process Priority Class Resolution

- Priority class is specified via **CreationFlags** (bitmask).
    
- Multiple priority bits can be set; **Windows selects the _lowest_ priority**.
    
- Defined process priority classes:
    
    - Idle / Low → **4**
        
    - Below Normal → **6**
        
    - Normal → **8**
        
    - Above Normal → **10**
        
    - High → **13**
        
    - Real-time → **24**
        
- Priority class affects **thread base priorities**, not the process directly.
    

---

### 2. Default & Privilege-Based Priority Handling

- If no priority specified → **defaults to Normal**.
    
- If **Real-time** requested but caller lacks `SE_INC_BASE_PRIORITY_NAME`:
    
    - Process is created with **High** priority instead.
        
- Process creation **does not fail** due to insufficient privilege.
    

---

### 3. Debugging Setup

- If `DEBUG_PROCESS` flag is set:
    
    - `Kernel32.dll` calls `DbgUiConnectToDbg` in `Ntdll.dll`.
        
    - Debug object handle is retrieved from the **TEB**.
        
- Enables native debugging support.
    

---

### 4. Hard Error Mode Initialization

- Default hard error mode is set if specified in creation flags.
    
- Controls system error dialogs (e.g., critical error pop-ups).
    

---

### 5. Attribute List Processing

- User-supplied attribute list:
    
    - Converted from **Win32 format → native NT format**
        
    - Internal attributes are added
        
- Attribute list can return **extra information** to the caller:
    
    - Initial thread TEB
        
    - Image section info
        
- Required for **protected processes**, since parents can’t query later.
    

---

### 6. Job Object & VDM Handling

- If process belongs to a **job object**:
    
    - `CREATE_SEPARATE_WOW_VDM` is **ignored**.
        
- Job constraints override VDM requests.
    

---

### 7. Security Attributes Conversion

- Process and thread security descriptors:
    
    - Converted into **OBJECT_ATTRIBUTES** (WDK-documented).
        
- Used internally by the kernel.
    

---

### 8. Modern (AppX) Process Determination

Process is considered _modern_ if:

- `PROC_THREAD_ATTRIBUTE_PACKAGE_FULL_NAME` is specified **OR**
    
- Creator is modern **and** no explicit parent is set
    

If modern:

- `BasepAppXExtension` is called
    
- Builds `APPX_PROCESS_CONTEXT`, containing:
    
    - Package moniker
        
    - App capabilities
        
    - Current directory
        
    - Full-trust flag
        
- Full-trust modern apps exist but are **not publicly exposed**
    
    - Example: **SystemSettings.exe**
        

---

### 9. AppContainer / LowBox Setup

- If modern process:
    
    - Security capabilities (`PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES`)  
        are recorded
        
    - `BasepCreateLowBox` prepares the **AppContainer (LowBox)** token
        
- SDK allows AppContainer creation for **legacy desktop apps**
    

---

### 10. Manifest Handling for Modern Processes

- Kernel is instructed to **skip embedded manifest detection**
    
- Modern apps do not rely on embedded manifests
    
    - They use AppX manifests instead
        

---

### 11. Image File Execution Options (IFEO) Debugger Handling

- If `DEBUG_PROCESS` is set:
    
    - IFEO **Debugger** registry entry is **ignored**
        
- Prevents infinite debugger-creation loops.
    

---

### 12. Desktop Association

- Every window belongs to a **desktop object**.
    
- If none specified in `STARTUPINFO`:
    
    - Process uses caller’s current desktop.
        
- Windows 10 Virtual Desktops:
    
    - **Do not create multiple desktop objects**
        
    - Windows are shown/hidden instead
        
- Contrast: `desktops.exe` creates **real** desktop objects.
    

---

### 13. Image Path & Command Line Normalization

- Executable path is converted to **NT object path**:
    
    - Example:  
        `C:\temp\a.exe` → `\Device\HarddiskVolumeX\temp\a.exe`
        
- Required for internal NT APIs.
    

---

### 14. RTL_USER_PROCESS_PARAMETERS Construction

- All gathered data is packed into:
    
    - `RTL_USER_PROCESS_PARAMETERS`
        
- Contains:
    
    - Image path
        
    - Command line
        
    - Environment
        
    - Current directory
        
    - Window settings
        

---

### Final Step

- `CreateProcessInternalW` calls **`NtCreateUserProcess`**
    
- If creation fails:
    
    - Kernel32 determines whether the image is:
        
        - Batch file
            
        - 16-bit
            
        - DOS executable
            
    - Attempts corrective handling