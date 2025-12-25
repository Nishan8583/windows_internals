# Detection/Hunting
- I run different code and scenarios here and see what events are generated and how we can detect it.
- Running sysmon with configuration from https://github.com/SwiftOnSecurity/sysmon-config.
- Log would be in 
```
Applications and Services Logs
  └ Microsoft
    └ Windows
      └ Sysmon
        └ Operational
```

### 1. Process Creation
 - Process Creation.
 - [code](./code/create_process.c).
 - Pretty straight forward, Sysmon Event ID is 1, We can see parent in ParentImage. Important fields, `Image`, `ParentImage`, `hashes`.
```

 RuleName - 
  UtcTime 2025-12-24 16:53:07.613 
  ProcessGuid {8750eb8f-1a73-694c-a001-000000000800} 
  ProcessId 2272 
  Image C:\Program Files\WindowsApps\Microsoft.WindowsNotepad_11.2508.38.0_x64__8wekyb3d8bbwe\Notepad\Notepad.exe 
  FileVersion - 
  Description - 
  Product - 
  Company - 
  OriginalFileName - 
  CommandLine notepad.exe 
  CurrentDirectory C:\Users\test\Desktop\ 
  User DESKTOP-H3EHHNK\test 
  LogonGuid {8750eb8f-1884-694c-06b2-010000000000} 
  LogonId 0x1b206 
  TerminalSessionId 1 
  IntegrityLevel Medium 
  Hashes MD5=DBCFB74F8064F337EE812D39EECB1D52,SHA256=3D5B56EC5CBE064E10B9525EF74359A4E6B83A9C3341F5FBAAC629BCE80B6817,IMPHASH=F5D5C33FBED3FE69F8850F260FA2939D 
  ParentProcessGuid {8750eb8f-1a72-694c-9b01-000000000800} 
  ParentProcessId 11996 
  ParentImage C:\Users\test\Desktop\a.exe 
  ParentCommandLine "C:\Users\test\Desktop\a.exe"  
  ParentUser DESKTOP-H3EHHNK\test 

```