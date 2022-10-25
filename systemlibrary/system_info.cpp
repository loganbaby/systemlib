#include "system_info.h"

void display_procs() {
  NTSTATUS status;
  PVOID buffer =
      VirtualAlloc(NULL, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  PSYSTEM_PROCESS_INFO spi;

  if (!buffer) {
    printf("\nError: Unable to allocate memory for process list (%d)\n",
           GetLastError());
    std::exit(1);
  }

  _tprintf(TEXT("\nProcess list allocated at address %#x\n"), buffer);
  spi = (PSYSTEM_PROCESS_INFO)buffer;

  if (!NT_SUCCESS(status = NtQuerySystemInformation(SystemProcessInformation,
                                                    spi, 1024 * 1024, NULL))) {
    _tprintf(TEXT("\nError: Unable to query process list (%#x)\n"), status);
    VirtualFree(buffer, 0, MEM_RELEASE);
    std::exit(1);
  }

  while (spi->NextEntryOffset) {
    _tprintf(TEXT("\nProcess name: %ws | Process ID: %d\n"), spi->ImageName.Buffer,
           spi->ProcessId);
    spi = (PSYSTEM_PROCESS_INFO)((LPBYTE)spi + spi->NextEntryOffset);
  }
}

void PrintProcessNameAndID(DWORD processID) {
  TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                FALSE, processID);

  if (NULL != hProcess) {
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
      GetModuleBaseName(hProcess, hMod, szProcessName,
                        sizeof(szProcessName) / sizeof(TCHAR));
    }
  }

  _tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);
  CloseHandle(hProcess);
}

std::vector<DWORD> GetAllProcessNameAndIDVector() {
  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  std::vector<DWORD> result;
  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    std::exit(1);
  }
  cProcesses = cbNeeded / sizeof(DWORD);

  for (i = 0; i < cProcesses; i++) {
    if (aProcesses[i] != 0) {
      result.push_back(aProcesses[i]);
    }
  }

  if (result.empty()) {
    _tprintf(TEXT("No procs. WTF :>"));
    std::exit(1);
  }

  return result;
}

BOOL PrintDevice(const wchar_t* id) {
  unsigned index;
  HDEVINFO hDevInfo;
  SP_DEVINFO_DATA DeviceInfoData;
  TCHAR id_upper[1024] = L"";
  TCHAR buf[1024] = L"";
  DEVPROPTYPE dpt = 0;
  DEVPROPKEY arr[100];
  DWORD count = 0;

  for (int i = 0; i < wcslen(id); i++) {
    id_upper[i] = toupper(id[i]);
  }

  hDevInfo =
      SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
  for (index = 0;; index++) {
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData)) {
      return FALSE;
    }

    BOOL res = SetupDiGetDeviceProperty(hDevInfo, &DeviceInfoData,
                                        &DEVPKEY_Device_InstanceId, &dpt,
                                        (PBYTE)buf, 1000, NULL, 0);
    if (res == FALSE) continue;

    if (wcscmp(buf, id_upper) == 0) {
      res = SetupDiGetDeviceProperty(hDevInfo, &DeviceInfoData,
                                     &DEVPKEY_Device_DeviceDesc, &dpt,
                                     (PBYTE)buf, 1000, NULL, 0);
      if (res) wprintf(L"* %s's properties: *\n\n ", buf);

      res = SetupDiGetDevicePropertyKeys(hDevInfo, &DeviceInfoData, arr, 100,
                                         &count, 0);

      for (int i = 0; i < count; i++) {
        res = SetupDiGetDeviceProperty(hDevInfo, &DeviceInfoData, &arr[i], &dpt,
                                       (PBYTE)buf, 1000, NULL, 0);

        if (res == FALSE) {
          continue;
        }

        wprintf(
            L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%"
            L"02hhX; %3d}: ",
            arr[i].fmtid.Data1, arr[i].fmtid.Data2, arr[i].fmtid.Data3,
            arr[i].fmtid.Data4[0], arr[i].fmtid.Data4[1], arr[i].fmtid.Data4[2],
            arr[i].fmtid.Data4[3], arr[i].fmtid.Data4[4], arr[i].fmtid.Data4[5],
            arr[i].fmtid.Data4[6], arr[i].fmtid.Data4[7], arr[i].pid);

        switch (dpt) {
          case DEVPROP_TYPE_STRING:
            wprintf(L"String   \t%s\n ", buf);
            break;
          case DEVPROP_TYPE_STRING_LIST:
            wprintf(L"Strings \tFirst line: %s\n ", buf);
            break;
          case DEVPROP_TYPE_BOOLEAN:
            wprintf(L"Bool     \t%d\n ", (bool)*((LPBYTE)(&buf)));
            break;
          case DEVPROP_TYPE_UINT16:
            wprintf(L"%Uint16    \t%d\n ", *((LPWORD)(&buf)));
            break;
          case DEVPROP_TYPE_UINT32:
            wprintf(L"Uint     \t%d\n ", *((LPUINT)(&buf)));
            break;
          case DEVPROP_TYPE_GUID:
            wprintf(
                L"GUID   "
                L"\t{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%"
                L"02hhX%02hhX}\n ",
                (*((GUID*)(&buf))).Data1, (*((GUID*)(&buf))).Data2,
                (*((GUID*)(&buf))).Data3, (*((GUID*)(&buf))).Data4[0],
                (*((GUID*)(&buf))).Data4[1], (*((GUID*)(&buf))).Data4[2],
                (*((GUID*)(&buf))).Data4[3], (*((GUID*)(&buf))).Data4[4],
                (*((GUID*)(&buf))).Data4[5], (*((GUID*)(&buf))).Data4[6],
                (*((GUID*)(&buf))).Data4[7]);

            break;
          case DEVPROP_TYPE_BINARY:
            wprintf(L"(Binary data)\n ");
            break;

          default:
            wprintf(L"Other    \tType: 0x%x\n ", (int)dpt);
            break;
        }
      }

      SetupDiDestroyDeviceInfoList(hDevInfo);
      return TRUE;
    }
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
  return FALSE;
}