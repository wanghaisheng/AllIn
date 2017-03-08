// test_dumper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <tchar.h>
#include <windows.h> // must located before dbghelp.h
#include <dbghelp.h>


//生产DUMP文件
int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, PWCHAR pwAppName)
{
    BOOL bOwnDumpFile = FALSE;
    HANDLE hDumpFile = hFile;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
        HANDLE,
        DWORD,
        HANDLE,
        MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION,
        PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION
        );

    MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelp = LoadLibrary("DbgHelp.dll");
    if (hDbgHelp)
        pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    if (pfnMiniDumpWriteDump)
    {
        if (hDumpFile == NULL || hDumpFile == INVALID_HANDLE_VALUE)
        {
            //TCHAR szPath[MAX_PATH] = { 0 };
            char szFileName[MAX_PATH] = { 0 };
            //TCHAR* szAppName = pwAppName;
            TCHAR* szVersion = "v1.0";
            int dwBufferSize = MAX_PATH;
            SYSTEMTIME stLocalTime;

            GetLocalTime(&stLocalTime);
            //GetTempPath(dwBufferSize, szPath);

            //wsprintf(szFileName, L"%s%s", szPath, szAppName);
            CreateDirectory(szFileName, NULL);

            wsprintf(szFileName, "C:\\pj\\bin\\w32d\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
                //szPath, szAppName, szVersion,
                szVersion,
                stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
                stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
                GetCurrentProcessId(), GetCurrentThreadId());
            hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
/*            printf("%s" szFileName);*/
            std::cout << szFileName << std::endl;

            bOwnDumpFile = TRUE;
            OutputDebugString(szFileName);
        }

        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            ExpParam.ThreadId = GetCurrentThreadId();
            ExpParam.ExceptionPointers = pExceptionPointers;
            ExpParam.ClientPointers = FALSE;

            pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                hDumpFile, 
                MINIDUMP_TYPE(MiniDumpWithDataSegs |
                MiniDumpWithPrivateReadWriteMemory |
                MiniDumpWithHandleData |
                MiniDumpWithFullMemoryInfo |
                MiniDumpWithThreadInfo |
                MiniDumpWithUnloadedModules), (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);

            if (bOwnDumpFile)
                CloseHandle(hDumpFile);
        }
    }

    if (hDbgHelp != NULL)
        FreeLibrary(hDbgHelp);

    return EXCEPTION_EXECUTE_HANDLER;
}


LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
    if (IsDebuggerPresent())
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    return GenerateMiniDump(NULL, lpExceptionInfo, L"test");
}


int main()
{
    //加入崩溃dump文件功能 
    SetUnhandledExceptionFilter(ExceptionFilter);

    char *p = NULL;
    *p = 'b';
}

// int _tmain(int argc, _TCHAR* argv[])
// {
//     char str[] = "1234,454,35";
//     char* pch;
//     printf("Splitting string \"%s\" into tokens:\n", str);
//     pch = strtok(str, ",");
//     while (pch != NULL) {
//         printf("%s\n", pch);
//         pch = strtok(NULL, ",");
//     }
//     return 0;
// 
//     char *p = NULL;
//     *p = 'b';
// 
// 	return 0;
// }

