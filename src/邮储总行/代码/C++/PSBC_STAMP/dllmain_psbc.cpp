#include "stdafx.h"
#include <Windows.h>
#include "USBControlF60.h"
#include "parse.h"

// BOOL APIENTRY DllMain(
//     HMODULE hModule,
//     DWORD  ul_reason_for_call,
//     LPVOID lpReserved
//     )
// {
//     switch (ul_reason_for_call) {
//     case DLL_PROCESS_ATTACH: {
//         PSBCConfig::GetInst()->Parse();
//         FOpenDev(NULL);
//     }
//         break;
//     case DLL_THREAD_ATTACH:
//         FCloseDev();
//         break;
//     case DLL_THREAD_DETACH:
//         break;
//     case DLL_PROCESS_DETACH:
//         break;
//     default:
//         break;
//     }
// 
//     return TRUE;
// }
