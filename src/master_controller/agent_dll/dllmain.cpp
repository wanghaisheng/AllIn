#include <windows.h>
#include "parse.h"
#include "cnn.h"
#include "common_definitions.h"
#include "mini_dump.h"
#include "api_set.h"

extern AsynAPISet api_agent;

void Start()
{
    InstallCrashReport();
    MC::KillProcessByName(MC::SERVER_NAME.c_str());

    MC::AgentConfig::GetInst()->Parse();
    MC::Cnn::GetInst()->SetAgent(&api_agent);
    MC::Cnn::GetInst()->Start();
}

void Stop()
{
    MC::Cnn::GetInst()->Stop();
    MC::KillProcessByName(MC::SERVER_NAME.c_str());
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
