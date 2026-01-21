package main

import (
	"fmt"
	"syscall"
	"unsafe"
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

// https://learn.microsoft.com/en-us/windows/win32/etw/wnode-header
type WNODE_HEADER struct {
	BufferSize        uint32
	ProviderId        uint32
	HistoricalContext uint64
	TimeStamp         int64
	Guid              syscall.GUID
	ClientContext     uint32
	Flags             uint32
}

// U can find the original C struct defnition in
// https://learn.microsoft.com/en-us/windows/win32/api/evntrace/ns-evntrace-event_trace_properties
// or in the path mentioned above for evntrace.h
type EVENT_TRACE_PROPERTIES struct {
	Wnode               WNODE_HEADER
	BufferSize          uint32
	MinimumBuffers      uint32
	MaximumBuffers      uint32
	MaximumFileSize     uint32
	LogFileMode         uint32
	FlushTimer          uint32
	EnableFlags         uint32
	AgeLimit            int32
	NumberOfBuffers     uint32
	FreeBuffers         uint32
	EventsLost          uint32
	BuffersWritten      uint32
	LogBuffersLost      uint32
	RealTimeBuffersLost uint32
	LoggerThreadId      uintptr
	LogFileNameOffset   uint32
	LoggerNameOffset    uint32
}

func main() {
	sessionName, err := syscall.UTF16FromString("NT Kernel Logger")
	if err != nil {
		fmt.Println("Error creating session name:", err)
		return
	}
	logFile, err := syscall.UTF16FromString("test.log")
	if err != nil {
		fmt.Println("Error creating log file name:", err)
		return
	}
	_ = logFile
	_ = sessionName

	// Windows expectes UTF16
	// UTF-16 uses 2 bytes per code unit, i.e. each character/Code unit is 2 bytes
	// Example:
	// utf16Name := []uint16{'A', 'B', 0}
	// 41 00  42 00  00 00
	//^---^  ^---^  ^---^
	// 2 B     2 B    2 B
	// len(utf16Name) = 3 (nuumber of code units)
	// Actual memory used is 6 bytes not 3
	// ETW needs to know 6, not 3
	// ETW reads memory like this:
	// base_pointer + LoggerNameOffset → WCHAR*
	// If you forget *2:
	// Offsets point into the middle of data
	bufferSize := uint32(
		unsafe.Sizeof(EVENT_TRACE_PROPERTIES{}) + uintptr(len(sessionName)*2) + uintptr(len(logFile)*2),
	)
}
