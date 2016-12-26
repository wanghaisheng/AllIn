#pragma once
#include "USBProtocol.h"
#include "WLock.h"

//此参数要比下位机申请的缓冲区大一个字节才行，因为第一个字节是有特殊意义的
#define WRITEBUF_LEN		  65

#define READBINFILEBUF  59
#define READBUFLEN		256
#define UsbDevWaitTime	7000

class CUSBParam
{
public:
	CUSBParam(void);
	~CUSBParam(void);

	//合成发送命令
	void ComposeSendPackage(SendPackage &sendPackage);
	//解析命令
	ReceivePackage* AnalyzeRecvPackage(void);

	//发送报告的缓冲区	
	UCHAR WriteBuffer[WRITEBUF_LEN];
	unsigned int writebuflen;
	//接收报告的缓冲区
	UCHAR ReadBuffer[READBUFLEN];
	UCHAR *pRead;
	//实际接收的长度
	DWORD readbuflen ;

	SendPackage m_sendcmd;	//发送命令
	ReceivePackage m_answercmd;	//响应命令

	Mutex g_Lock;
};

