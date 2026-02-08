# Threads
- To create a thread
    - `CreateThread`
    - `CreateRemoteThread`
        - Takes an extra arguement, handle to target process, commonly used by debuggers. Debugger injects threads, and calls `DebugBreak` function.
        - Obtain internal information about another process, legitimate or malicious purpose.
    - `CreateRemoteThreadEx`. 
        - The above 2 functions calls this functions in the end with appropriate defaults.
        - It then finally calls `NtCreateThreadEx`.
    - To create thread in kernel mode `PsCreateSystemThread`

# Data Structures
- At OS level thread represented by execute thread object.
- Executive thread object encapsulates ETHREAD, which then points to KTHREAD.

### ETHREAD
![alt_image](./images/01.png)
- TCB (Thread Control Block), which is type of KTHREAD struct, controls info necessary for scheduling, synchornization and time keeping functions.