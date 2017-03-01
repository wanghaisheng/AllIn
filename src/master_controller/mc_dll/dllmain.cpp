#include "stdafx.h"
#include <Windows.h>
#include "tool.h"
#include "event_cpu.h"
#include "parse.h"
#include "recver.h"
#include "tool.h"
#include "USBControlF60.h"

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        int ret = ::F_RegisterDevCallBack(&ConnectCallBack);
        ret = ::FRegisterDevCallBack(&DevMsgCallBack);
        MC::SvrConfig::GetInst()->Parse();
        MC::EventCPUCore::GetInstance()->Start();

        SharedMem::GetInst()->CreateSharedMem();
        MC::Tool::GetInst();
    }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        MC::EventCPUCore::GetInstance()->Stop();
    }
        break;
    default:
        break;
    }

    return TRUE;
}
