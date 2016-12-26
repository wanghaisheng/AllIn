#pragma once

#include <iostream>
#include <queue>
#include "USBParam.h"

//class CUSBParam;
class CCommunicateToUSB
{
public:
	CCommunicateToUSB(void);
	~CCommunicateToUSB(void);

	int InitUSB(void);
	
	void RegisterDevCallBack(PVOID func);
	int OpenDev(const char *file);
	int CloseDev(void);
	int send(SendPackage *sendPackage);
	int receive(ReceivePackage *recvPackage);
	int GetStamperStauts();
	void SetStamperStauts(int status);

    static void signal();

public:
	//回调函数指针
	PVOID m_USBMsgCallBack;
	
	//发送报告用的OVERLAPPED。
	OVERLAPPED WriteOverlapped;
	//接收报告用的OVERLAPPED。
	OVERLAPPED ReadOverlapped;


	//用来保存读数据的设备句柄
	HANDLE hReadHandle;
	//用来保存写数据的设备句柄
	HANDLE hWriteHandle;
	//数据接收线程	
	HANDLE hReadThread;
	//数据发送线程
	HANDLE hWriteThread;

	//接收数据事件
	HANDLE recvevent;

	//读写线程退出事件
	HANDLE exitevent;

	//正在发送数据的标志, TRUE 正在发送中
	BOOL DataInSending;
	//设备是否已经打开
	BOOL isfound;

	BOOL DataInReading;

	//参数解析
	CUSBParam m_param;

    struct NotifyParam {
        unsigned int uMsg;
        unsigned int wParam;
        long lParam;
        unsigned char* data;
        unsigned char len;
    };

    static HANDLE notify_ev_;
    static Mutex notify_lock_;
    static NotifyParam notify_;

protected:
	static UINT __stdcall WriteData2UsbThread(LPVOID pParam);
	static UINT __stdcall ReadDataFromUsbThread(LPVOID pParam);
    static UINT __stdcall AsyncNotifyThread(LPVOID pParam);
    static UINT __stdcall WriteLogThread(LPVOID pParam);

private:
	//发送数据
	int sendData2Usb(void);		
	std::string strDevPathName;	//设备路径
	ReceivePackage* usbpackage;

    static unsigned char send_cmd_;
	static int  stamperStatus ;
    static HANDLE reset_ev;
    static Mutex log_list_lock;
    static std::queue<std::string> logs_;
};

