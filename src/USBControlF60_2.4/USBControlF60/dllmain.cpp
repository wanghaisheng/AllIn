// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "Log.h"
#include "CtrlUSB.h"
//#include <vld.h>



//进程或线程句柄
HINSTANCE hinst = NULL; 
//窗口句柄
HWND hwndDLG = NULL; 

//窗口消息处理函数
LRESULT CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) ;  
//创建窗口消息接收线程
DWORD WINAPI CreateWindowThreadPro(LPVOID lpParameter);


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{//得到进程实例
			hinst = static_cast<HINSTANCE>(hModule);
			//创建窗口线程
			HANDLE hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)CreateWindowThreadPro,NULL,NULL,NULL);
			bool bRet = (hThread != NULL);
			CloseHandle(hThread);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//窗口消息处理
LRESULT CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CCtrlUSB* pCCtrlUSB = CCtrlUSB::sharedCCtrlUSB();
	switch( message )
	{
	case WM_CREATE:		
		if (pCCtrlUSB)
		{
			pCCtrlUSB->Init(hWnd);
			pCCtrlUSB->FindDevPath();
            if (!pCCtrlUSB->GetDevPath().empty())
                CLog::sharedLog()->WriteNormalLog("WinProc->成功确定设备路径");
            else
                CLog::sharedLog()->WriteNormalLog("WinProc->未能确定设备路径");
		}		
		break;
	case WM_DEVICECHANGE:
		if (pCCtrlUSB)
		{
			pCCtrlUSB->OnDeviceChange(wParam, lParam);
		}		
		return (TRUE);
	case WM_DESTROY:		
		::PostQuitMessage(0);
		break;
	default:
		break;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

//窗口创建，接收消息线程
DWORD WINAPI CreateWindowThreadPro(LPVOID lpParameter)
{
	WNDCLASS wc;  
	MSG msg;

	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinst;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = _T("USBControlledStamperDev");

	RegisterClass(&wc);

	hwndDLG = ::CreateWindow(
		_T("USBControlledStamperDev"), //窗口类名称
		_T(""), //窗口标题
		WS_OVERLAPPEDWINDOW, //窗口风格，定义为普通型
		0, //窗口位置的x坐标
		0, //窗口位置的y坐标
		1, //窗口的宽度
		1, //窗口的高度
		NULL, //父窗口句柄
		NULL, //菜单句柄
		hinst, //应用程序实例句柄
		NULL ); //窗口创建数据指针

	if(!hwndDLG) return 1;
	//隐藏窗口
	::ShowWindow(hwndDLG, SW_HIDE);
	//窗口客户区域更新
	::UpdateWindow(hwndDLG);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
