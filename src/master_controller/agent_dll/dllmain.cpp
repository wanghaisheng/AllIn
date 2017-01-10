#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include "parse.h"
#include "cnn.h"
#include "api_set.h"

extern AsynAPISet api_agent;

void KillProcessByName(const char *filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (strcmp(pEntry.szExeFile, filename) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL && pEntry.th32ProcessID != GetCurrentProcessId()) {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

void Start()
{
    MC::Config::GetInst()->Parse();
    MC::Cnn::GetInst()->SetAgent(&api_agent);
    MC::Cnn::GetInst()->Start();
}

void Stop()
{
    MC::Cnn::GetInst()->Stop();
    KillProcessByName("mc_exed.exe");
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Start();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Stop();
        break;
    default:
        break;
    }

    return TRUE;
}
