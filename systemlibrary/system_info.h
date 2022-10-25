#pragma once
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <stdio.h>
#include <winternl.h>
#include <cstdlib>
#include <Psapi.h>
#include <vector>
#include <Setupapi.h>
#include <winusb.h>
#include <stdlib.h>
#include <Devpkey.h>
#include <iostream>
#include <wchar.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma warning(disable : 4477)
#pragma warning(disable : 4313)

typedef struct _SYSTEM_PROCESS_INFO {
  ULONG NextEntryOffset;
  ULONG NumberOfThreads;
  LARGE_INTEGER Reserved[3];
  LARGE_INTEGER CreateTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;
  UNICODE_STRING ImageName;
  ULONG BasePriority;
  HANDLE ProcessId;
  HANDLE InheritedFromProcessId;
} SYSTEM_PROCESS_INFO, *PSYSTEM_PROCESS_INFO;

void display_procs();       // show all system proccesses

void PrintProcessNameAndID(DWORD processID);       // show all procs & names

std::vector<DWORD> GetAllProcessNameAndIDVector();        // get vector of all processes

BOOL PrintDevice(const wchar_t* id);             // print all properties of device by id