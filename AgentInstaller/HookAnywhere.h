#pragma once

#include <Windows.h>

#define RESULT_SYSTEM_ERROR                         0x80000000
#define RESULT_ERROR_BUFFER_LENGTH_MISMATCH         0x80000001
#define RESULT_ERROR_CURRENT_PROCESS_SPECIFIED      0x80000002
#define RESULT_ERROR_SOME_THREADS_SUSPEND_FAILED    0x80000003
#define RESULT_ERROR_SOME_THREADS_RESUME_FAILED     0x80000004
#define RESULT_ERROR_DLL_PATH_IS_TOO_LARGE          0x80000005

#ifdef _DEBUG
EXTERN_C
{
#endif
DWORD WINAPI SetHook(DWORD processId, PVOID hookAddress, PVOID hookProc, SIZE_T hookProcSize, PVOID hookProcData);
DWORD WINAPI UnHook(DWORD processId, PVOID hookAddress);
DWORD WINAPI CheckHook(DWORD processId, PVOID hookAddress, PBOOL result);
BOOL  WINAPI SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
DWORD WINAPI GetMainThreadIdEx(DWORD processID);
DWORD WINAPI StopThreadsEx(DWORD processID);
DWORD WINAPI RunThreadsEx(DWORD processID);
DWORD WINAPI InjectDll(DWORD processId, LPCSTR dllPath, BOOL bLoad);
#ifdef _DEBUG
}
#endif
