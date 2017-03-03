#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <tchar.h>
#include <windows.h> // must located before dbghelp.h
#include <dbghelp.h>
#include "dumper.h"

#ifdef UNICODE
#define _tcssprintf wsprintf
#define tcsplitpath _wsplitpath
#else
#define _tcssprintf sprintf
#define tcsplitpath _splitpath
#endif

MiniDumper       *gs_pMiniDumper = NULL;
LPCRITICAL_SECTION gs_pCriticalSection = NULL;

//-----------------------------------------------------------------------------
// APIs
//-----------------------------------------------------------------------------
// Based on dbghelp.h
typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD dwPid,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
    if (pModuleName == 0)
    {
        return FALSE;
    }

    WCHAR szFileName[MAX_PATH] = { 0 };
    _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

    if (_wcsicmp(szFileName, L"ntdll") == 0)
        return TRUE;

    return FALSE;
}

BOOL WINAPI MiniDumpCallback(PVOID							 pParam,
    const PMINIDUMP_CALLBACK_INPUT   pInput,
    PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
    if (pInput == 0 || pOutput == 0)
        return FALSE;

    switch (pInput->CallbackType)
    {
    case ModuleCallback:
        if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
        {
            if (!IsDataSectionNeeded(pInput->Module.FullPath))
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
        }
        return TRUE;
    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;
    default:;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Name: unhandledExceptionHandler()
// Desc: Call-back filter function for unhandled exceptions
//-----------------------------------------------------------------------------
LONG WINAPI UnhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo)
{
    if (NULL == gs_pMiniDumper)
        return EXCEPTION_CONTINUE_SEARCH;

    return gs_pMiniDumper->WriteMiniDump(pExceptionInfo);
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
void DisableSetUnhandledExceptionFilter()
{
    HMODULE     hModule = LoadLibrary("kernel32.dll");
    void* pAddr = (void*)GetProcAddress(hModule, "SetUnhandledExceptionFilter");
    if (pAddr)
    {
        unsigned char code[16] = { 0 };
        int			  size = 0;

        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag = 0;
        DWORD dwTempFlag = 0;

        VirtualProtect(pAddr, size, PAGE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), pAddr, code, size, NULL);
        VirtualProtect(pAddr, size, dwOldFlag, &dwTempFlag);
    }
    FreeLibrary(hModule);
}

HRESULT MiniDumper::CreateInstance()
{
    if (NULL == gs_pMiniDumper)
    {
        gs_pMiniDumper = new MiniDumper();
    }
    if (NULL == gs_pCriticalSection)
    {
        gs_pCriticalSection = new CRITICAL_SECTION;
        InitializeCriticalSection(gs_pCriticalSection);
    }

    return(S_OK);
}

//-----------------------------------------------------------------------------
// Name: ReleaseInstance()
// Desc: Release gs_pMiniDumper
//-----------------------------------------------------------------------------
HRESULT  MiniDumper::ReleaseInstance()
{
    if (NULL != gs_pMiniDumper)
    {
        delete gs_pMiniDumper;
        gs_pMiniDumper = NULL;
    }
    if (NULL != gs_pCriticalSection)
    {
        DeleteCriticalSection(gs_pCriticalSection);
        gs_pCriticalSection = NULL;
    }

    return(S_OK);
}

//-----------------------------------------------------------------------------
// Name: CMiniDumper()
// Desc: Constructor
//-----------------------------------------------------------------------------
MiniDumper::MiniDumper()
{
    // 使应用程序能够取代每个进程和线程的顶级异常处理程序	
    ::SetUnhandledExceptionFilter(UnhandledExceptionHandler);
    DisableSetUnhandledExceptionFilter();
}

//-----------------------------------------------------------------------------
// Name: ~CMiniDumper()
// Desc: Destructor
//-----------------------------------------------------------------------------
MiniDumper::~MiniDumper(void)
{

}

//-----------------------------------------------------------------------------
// Name: setMiniDumpFileName()
// Desc: 
//-----------------------------------------------------------------------------
void MiniDumper::SetMiniDumpFileName(void)
{
    time_t currentTime;
    time(&currentTime);

    _tcssprintf(m_szMiniDumpPath, _T("%s.%ld.dmp"), m_szAppPath, currentTime);
}

//-----------------------------------------------------------------------------
// Name: getImpersonationToken()
// Desc: The method acts as a potential workaround for the fact that the 
//       current thread may not have a token assigned to it, and if not, the 
//       process token is received.
//-----------------------------------------------------------------------------
BOOL MiniDumper::GetImpersonationToken(HANDLE* phToken)
{
    *phToken = NULL;
    if (!OpenThreadToken(GetCurrentThread(),
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        TRUE,
        phToken))
    {
        if (GetLastError() == ERROR_NO_TOKEN)
        {
            // No impersonation token for the current thread is available. 
            // Let's go for the process token instead.
            if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                phToken))
                return FALSE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: enablePrivilege()
// Desc: Since a MiniDump contains a lot of meta-data about the OS and 
//       application state at the time of the dump, it is a rather privileged 
//       operation. This means we need to set the SeDebugPrivilege to be able 
//       to call MiniDumpWriteDump.
//-----------------------------------------------------------------------------
BOOL MiniDumper::EnablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
{
    BOOL				bOk = FALSE;
    TOKEN_PRIVILEGES	tp;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    bOk = LookupPrivilegeValue(0, pszPriv, &tp.Privileges[0].Luid);
    if (bOk)
    {
        DWORD cbOld = sizeof(*ptpOld);
        bOk = AdjustTokenPrivileges(hToken, FALSE, &tp, cbOld, ptpOld, &cbOld);
    }

    return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
}

//-----------------------------------------------------------------------------
// Name: restorePrivilege()
// Desc: 
//-----------------------------------------------------------------------------
BOOL MiniDumper::RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
{
    BOOL bOk = AdjustTokenPrivileges(hToken, FALSE, ptpOld, 0, NULL, NULL);
    return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
}

//-----------------------------------------------------------------------------
// Name: writeMiniDump()
// Desc: 
//-----------------------------------------------------------------------------
LONG MiniDumper::WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo)
{
    LONG	retval = EXCEPTION_CONTINUE_SEARCH;
    HANDLE	hImpersonationToken = NULL;

    if (!GetImpersonationToken(&hImpersonationToken))
        return FALSE;

    // You have to find the right dbghelp.dll. 
    // Look next to the EXE first since the one in System32 might be old (Win2k)
    HMODULE hDll = NULL;
    if (GetModuleFileName(NULL, m_szAppPath, MAX_PATH))
    {
        WCHAR szDir[MAX_PATH] = { 0 };
        TCHAR	szDbgHelpPath[MAX_PATH] = { 0 };

//         _wsplitpath(m_szAppPath, NULL, szDir, NULL, NULL);
//         _tcscpy(szDbgHelpPath, szDir);
//         _tcscat(szDbgHelpPath, _T("DBGHELP.DLL"));
// 
//         hDll = ::LoadLibrary(szDbgHelpPath);
    }

    if (hDll == NULL)
    {
        // If we haven't found it yet - try one more time.
        hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
    }

    if (hDll)
    {
        // Get the address of the MiniDumpWriteDump function, which writes 
        // user-mode mini-dump information to a specified file.
        MINIDUMPWRITEDUMP MiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
        if (MiniDumpWriteDump != NULL)
        {
            SetMiniDumpFileName();

            // Create the mini-dump file...
            HANDLE hFile = ::CreateFile(m_szMiniDumpPath,
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

                ExInfo.ThreadId = ::GetCurrentThreadId();
                ExInfo.ExceptionPointers = pExceptionInfo;
                ExInfo.ClientPointers = NULL;

                MINIDUMP_CALLBACK_INFORMATION mci;
                mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
                mci.CallbackParam = 0;

                // We need the SeDebugPrivilege to be able to run MiniDumpWriteDump
                TOKEN_PRIVILEGES tp;
                BOOL bPrivilegeEnabled = EnablePrivilege(SE_DEBUG_NAME, hImpersonationToken, &tp);
                BOOL bOk;

                // DBGHELP.dll is not thread-safe, so we need to restrict access...
                EnterCriticalSection(gs_pCriticalSection);
                {
                    // Write out the mini-dump data to the file...
                    bOk = MiniDumpWriteDump(GetCurrentProcess(),
                        GetCurrentProcessId(),
                        hFile,
                        MiniDumpNormal,
                        (NULL == pExceptionInfo) ? (NULL) : (&ExInfo),
                        NULL,
                        &mci);
                }
                LeaveCriticalSection(gs_pCriticalSection);

                // Restore the privileges when done
                if (bPrivilegeEnabled)
                    RestorePrivilege(hImpersonationToken, &tp);

                if (bOk)
                {
                    retval = EXCEPTION_EXECUTE_HANDLER;
                }

                ::CloseHandle(hFile);
            }
        }
    }

    FreeLibrary(hDll);
    if (NULL != pExceptionInfo)
    {
        TerminateProcess(GetCurrentProcess(), 0);
    }

    return retval;
}
