package main

import (
	"fmt"
	"syscall"
)

const (
	EVENT_TRACE_FLAG_NETWORK_TCPIP = 0x00000010
	EVENT_TRACE_FILE_MODE_CIRCULAR = 0x00000002
	WMODE_FLAG_TRACED_GUID         = 0x00020000

	EVENT_TRACE_CONTROL_STOP = 1
)

// importing the library and procedures
var (
	advapi32          = syscall.NewLazyDLL("advapi32.dll")
	procStartTraceW   = advapi32.NewProc("StartTraceW")
	procControlTraceW = advapi32.NewProc("ControlTraceW")
)

// SystemTraceControlGuid we will need later on, in C we have already included it from evntrace.h
// but in Go we have to define it ourselves
/* In C
const GUID SystemTraceControlGuid =
{ 0x9e814aad, 0x3204, 0x11d2,
  { 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39 } };

*/
// Find it in C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um\evntrace.h or search for the filename
// Path version may be different on system, search for string SystemTraceControlGuid
var SystemControlGuid = syscall.GUID{
	Data1: 0x9e814aad,
	Data2: 0x3204,
	Data3: 0x11d2,
	Data4: [8]byte{0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39},
}

func main() {
	sessionName, err := syscall.UTF16FromString("NT Kernel Logger")
	if err != nil {
		fmt.Println("Error creating session name:", err)
		return
	}

	_ = sessionName
}
