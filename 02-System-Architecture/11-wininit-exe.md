# wininit.exe
Windows initialization process
The Wininit.exe process performs the following system initialization functions:
1. 
It marks itself and the main thread critical so that if it exits prematurely and the system is 
booted in debugging mode, it will break into the debugger. (Otherwise, the system will crash.)
2. It causes the process to treat certain errors as critical, such as invalid handle usage and heap 
corruption.
3. It initializes support for state separation, if the SKU supports it.
4. It creates an event named Global\FirstLogonCheck (this can be observed in Process Explorer 
or WinObj under the \BaseNamedObjects directory) for use by Winlogon processes to detect 
which Winlogon is first to launch.
5. It creates a WinlogonLogoff event in the BasedNamedObjects object manager directory to be 
used by Winlogon instances. This event is signaled (set) when a logoff operation starts.
6. It increases its own process base priority to high (13) and its main thread’s priority to 15.
7. 
Unless configured otherwise with the NoDebugThread registry value in the HKLM\Software\
Microsoft\Windows NT\CurrentVersion\Winlogon key, it creates a periodic timer queue, which 
will break into any user-mode process as specified by the kernel debugger. This enables remote 
kernel debuggers to cause Winlogon to attach and break into other user-mode applications.
8. It sets the machine name in the environment variable COMPUTERNAME and then updates and 
configures TCP/IP-related information such as the domain name and host name
9. It sets the default profile environment variables USERPROFILE, ALLUSERSPROFILE, PUBLIC, and 
ProgramData.
10. It creates the temp directory by expanding %SystemRoot%\Temp (for example, C:\Windows\
Temp).
11. It sets up font loading and DWM if session 0 is an interactive session, which depends on the SKU.
12. It creates the initial terminal, which is composed of a window station (always named Winsta0) 
and two desktops (Winlogon and Default) for processes to run on in session 0.
13. It initializes the LSA machine encryption key, depending on whether it’s stored locally or if it 
must be entered interactively. See Chapter 7 for more information on how local authentication 
keys are stored.
14. It creates the Service Control Manager (SCM or Services.exe). See the upcoming paragraphs for 
a brief description and Chapter 9 in Part 2 for more details.
95
CHAPTER 2 System architecture 
15. It starts the Local Security Authentication Subsystem Service (Lsass.exe) and, if Credential Guard 
is enabled, the Isolated LSA Trustlet (Lsaiso.exe). This also requires querying the VBS provision
ing key from UEFI. See Chapter 7 for more information on Lsass.exe and Lsaiso.exe.
16. If Setup is currently pending (that is, if this is the first boot during a fresh install or an update to 
a new major OS build or Insider Preview), it launches the setup program.
17. It waits forever for a request for system shutdown or for one of the aforementioned system 
processes to terminate (unless the DontWatchSysProcs registry value is set in the Winlogon  
key mentioned in step 7). In either case, it shuts down the system