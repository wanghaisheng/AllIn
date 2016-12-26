#include "StdAfx.h"
#include "CommunicateToUSB.h"
#include <process.h>
#include "USBControlF60.h"
#include "Log.h"
#include "SealLog.h"

unsigned char CCommunicateToUSB::send_cmd_ = 0xff;
int  CCommunicateToUSB::stamperStatus = 0 ;
Mutex CCommunicateToUSB::log_list_lock;
std::queue<std::string> CCommunicateToUSB::logs_;
HANDLE CCommunicateToUSB::reset_ev = CreateEvent(NULL, TRUE, FALSE, NULL);

// 手动复位, 初始non-signaled
HANDLE CCommunicateToUSB::notify_ev_ = CreateEvent(NULL, TRUE, FALSE, NULL);
Mutex CCommunicateToUSB::notify_lock_;
CCommunicateToUSB::NotifyParam CCommunicateToUSB::notify_;

CCommunicateToUSB::CCommunicateToUSB(void)
{

}

CCommunicateToUSB::~CCommunicateToUSB(void)
{
	//让receive函数退出
	SetEvent(recvevent);

	//通知退出读写线程
	SetEvent(exitevent);
	SetEvent(WriteOverlapped.hEvent);
	SetEvent(WriteOverlapped.hEvent);
	Sleep(100);
	CloseDev();

    if (reset_ev) {
        CloseHandle(reset_ev);
        reset_ev = NULL;
    }

	if (recvevent)
	{
		CloseHandle(recvevent);
		recvevent = NULL;
	}
	
	if (exitevent)
	{
		CloseHandle(exitevent);
		exitevent = NULL;
	}

	if (WriteOverlapped.hEvent)
	{
		CloseHandle(WriteOverlapped.hEvent);
		WriteOverlapped.hEvent = NULL;
	}
	
	if (ReadOverlapped.hEvent)
	{
		CloseHandle(ReadOverlapped.hEvent);
		ReadOverlapped.hEvent = NULL;
	}
}

int CCommunicateToUSB::InitUSB(void)
{
	CLog* plog = CLog::sharedLog();
	plog->WriteNormalLog("InitUSB：USB初始化");

	int iresult = 0;


	m_USBMsgCallBack = NULL;

	hReadHandle = INVALID_HANDLE_VALUE;
	hWriteThread = INVALID_HANDLE_VALUE;

	WriteOverlapped.Offset=0;
	WriteOverlapped.OffsetHigh=0;
	
	WriteOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);	
	hWriteThread = (HANDLE)_beginthreadex(NULL, 0, WriteData2UsbThread, this, 0, NULL);

	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	hReadThread =  (HANDLE)_beginthreadex(NULL, 0, ReadDataFromUsbThread, this, 0, NULL);

    //异步通知线程
    _beginthreadex(NULL, 0, AsyncNotifyThread, this, 0, NULL);

    _beginthreadex(NULL, 0, WriteLogThread, this, 0, NULL);

	recvevent = CreateEvent(NULL,TRUE,FALSE,NULL);
	exitevent = CreateEvent(NULL,TRUE,FALSE,NULL);

	DataInSending = FALSE;
	DataInReading =	FALSE;
	
	isfound = FALSE;

	strDevPathName.clear();
	usbpackage = NULL;

	return iresult;
}

void CCommunicateToUSB::RegisterDevCallBack(PVOID func)
{
	m_USBMsgCallBack = func;
}

int CCommunicateToUSB::OpenDev(const char *file)
{
	CLog* plog = CLog::sharedLog();

	int iresult;
	iresult = 0;

	if (isfound)
	{
		plog->WriteNormalLog("OpenDev->file: %s, 设备已经打开", file);
		return iresult;
	}

	if (file == NULL || 0 == strlen(file))
	{
		iresult = -1;
		plog->WriteNormalLog("OpenDev->file:%s, 设备打开失败，设备文件路径为空", file);
		return iresult;
	}

	hReadHandle=CreateFile(file, 
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL,
        OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL);

	if(hReadHandle==INVALID_HANDLE_VALUE)
	{
        plog->WriteNormalLog("OpenDev->file:%s, 读文件创建失败:%d", file, GetLastError());
		iresult = -2;
		return iresult;
	}

	//写方式打开设备
	hWriteHandle=CreateFile(file, 
		GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL,
        OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL);
	if(hWriteHandle==INVALID_HANDLE_VALUE)
	{ 
		iresult = -3;
		CloseHandle(hReadHandle);
		hReadHandle = NULL;
		plog->WriteNormalLog("OpenDev->file:%s, 写文件创建失败:%d", file, GetLastError());
		return iresult;
	}

	DataInSending=FALSE; 
	strDevPathName = file;
	isfound = TRUE;
	SetEvent(ReadOverlapped.hEvent);

	return iresult;
}

int CCommunicateToUSB::CloseDev(void)
{
	CLog* plog = CLog::sharedLog();
	plog->WriteNormalLog("CloseDev");

	/*if (!isfound)
	{
		return 0;
	}*/

	//让receive函数退出
	SetEvent(recvevent);

	int iresult = 0;
	if(hReadHandle!=INVALID_HANDLE_VALUE)
	{//只关闭读写句柄中的一个？？
		CloseHandle(hReadHandle);
		hReadHandle=INVALID_HANDLE_VALUE;
	}

	//if(hWriteHandle!=INVALID_HANDLE_VALUE)
	//{//只关闭读写句柄中的一个？？
	//	CloseHandle(hWriteHandle);
	//	hWriteHandle=INVALID_HANDLE_VALUE;
	//}

	DataInSending=FALSE;
	strDevPathName.clear();
	isfound = FALSE;

	return iresult;
}

int CCommunicateToUSB::send(SendPackage *sendPackage)
{
	CLog* plog = CLog::sharedLog();
	plog->WriteNormalLog("send");

	//设置接收事件为无效状态
    ResetEvent(recvevent);
    ResetEvent(reset_ev);
	m_param.ComposeSendPackage(*sendPackage);

	unsigned char *pdata = m_param.WriteBuffer;
	int length = m_param.writebuflen;
	//发送命令
	int ret = sendData2Usb();
	if(0 == ret)
	{
        send_cmd_ = sendPackage->m_cmd;
		plog->WriteNormalLog("send 成功");
		return STF_SUCCESS;
	}

	string strLog;
	char szTmp[200] = {0};
	for(int i = 0; i < length; ++i)
	{//
		sprintf_s(szTmp, sizeof(szTmp), _T("%02X"), pdata[i]) ;
		strLog += szTmp;
	}

	plog->WriteNormalLog("send 失败，data=%s, ret=%d", strLog.c_str(), ret);
	return STF_ERR_SENDFAIL;
}


int CCommunicateToUSB::receive(ReceivePackage *recvPackage)
{
	int iresult = STF_SUCCESS;
	//格式 readBuffer : 0x00(1Byte), head(1Byte), cmd(1Byte), len(1Byte), data(N Byte), check(1Byte), end(1Byte)	
    if (CMD_RESET == recvPackage->m_cmd) { //'复位'命令同步处理, 
        DWORD DResult = WaitForSingleObject(reset_ev, 3 * UsbDevWaitTime);
        if (DResult != WAIT_TIMEOUT) {
            DataInReading = true;
            usbpackage = m_param.AnalyzeRecvPackage();
            if (usbpackage->m_cmd == USB_RECEIVE_ERR) {
                iresult = STF_ERR_COMMUNICATION;
            } else {
                if (recvPackage)
                    *recvPackage = *usbpackage;
            }

            DataInReading = false;
            usbpackage = NULL;
            ResetEvent(reset_ev);
        } else {
            CLog::sharedLog()->WriteNormalLog("CCommunicateToUSB::receive->超时未收到下位机复位应答");
            iresult = STF_ERR_RECVFAIL;
        }

        return iresult;
    }

    DWORD DResult = WaitForSingleObject(recvevent, UsbDevWaitTime);
    if (DResult != WAIT_TIMEOUT) {
	    DataInReading = true;
        usbpackage = m_param.AnalyzeRecvPackage();
        if (usbpackage->m_cmd == USB_RECEIVE_ERR) {
          iresult = STF_ERR_COMMUNICATION;//收到错误包
        } else {
            if (recvPackage)
                *recvPackage = *usbpackage;
        } 

		usbpackage = NULL;
  } else {
        CLog::sharedLog()->WriteNormalLog("CCommunicateToUSB::receive->超时未收到命令:%0X的回应",
            recvPackage->m_cmd);
        //硬件无响应
        iresult = STF_ERR_RECVFAIL;
  }

  DataInReading= false;
  CLog* plog = CLog::sharedLog();	
  plog->WriteNormalLog("receive ret=%d", iresult);
  return iresult;
}

int CCommunicateToUSB::GetStamperStauts()
{
	return stamperStatus;
}
void CCommunicateToUSB::SetStamperStauts(int status)
{
	stamperStatus = status;
}

UINT __stdcall CCommunicateToUSB::WriteData2UsbThread(LPVOID pParam)
{
	if (NULL == pParam)
	{
		return 0;
	}

	CCommunicateToUSB* pusb = (CCommunicateToUSB*)pParam;
	
	while(TRUE)
	{
		//线程退出通知
		int retexit = 0;
		retexit = WaitForSingleObject(pusb->exitevent, 0);
		if (retexit == WAIT_OBJECT_0)
			break;
		
		//设置事件为无效状态
		ResetEvent(pusb->WriteOverlapped.hEvent);
		//等待事件触发
		WaitForSingleObject(pusb->WriteOverlapped.hEvent, INFINITE);
		//清除数据正在发送标志
		pusb->DataInSending=FALSE;
	}

	return 0;
}

UINT __stdcall CCommunicateToUSB::AsyncNotifyThread(LPVOID pParam)
{
    CLog* plog = CLog::sharedLog();
    CCommunicateToUSB* pusb = (CCommunicateToUSB*) pParam;
    if (NULL == pusb)
        return 0;

    while (TRUE) {
        int retexit = WaitForSingleObject(pusb->exitevent, 0);
        if (retexit == WAIT_OBJECT_0)
            break;

        DWORD ret = WaitForSingleObject(notify_ev_, INFINITE);
        if (ret == WAIT_TIMEOUT)
            continue;

        //下位机异步通知消息
        NotifyParam para;
        {
            CLock lk(notify_lock_);
            para = notify_;
        }

        pfnDevMsgCallBack pDevMsgCallBack = (pfnDevMsgCallBack) pusb->m_USBMsgCallBack;
        if (pDevMsgCallBack) {
            plog->WriteNormalLog("回调函数开始, 回调类型:%0X, 回调参数:%0X",
                para.uMsg,
                para.wParam);

            pDevMsgCallBack(para.uMsg, para.wParam, para.lParam, para.data, para.len);
            plog->WriteNormalLog("回调函数结束");
        } else {
            plog->WriteNormalLog("回调函数为空");
        }

        ResetEvent(notify_ev_);

        if (para.uMsg == CMD_STAMPER_DOWN) {
            stamperStatus = 1;
        } else if (para.uMsg == CMD_STAMPER_COMPLETE) {
            stamperStatus = 2;
        } else {
            stamperStatus = 0;
        }
    }

    return 0;
}

UINT __stdcall CCommunicateToUSB::ReadDataFromUsbThread(LPVOID pParam)
{
	CLog* plog = CLog::sharedLog();	
	CCommunicateToUSB* pusb = (CCommunicateToUSB*)pParam;
	if (NULL == pusb)
		return 0;
	
	//该线程是个死循环，直到程序退出时，它才退出
	while(TRUE) {
		//线程退出通知
		int retexit = 0;
		retexit = WaitForSingleObject(pusb->exitevent, 0);
		if (retexit == WAIT_OBJECT_0)
			break;

// 		if(pusb->DataInReading) {
// 			Sleep(100);
// 			continue;
// 		}

        UCHAR ReadBuffer[READBUFLEN];
		//设置事件为无效状态
		ResetEvent(pusb->ReadOverlapped.hEvent);
		//如果设备已经找到
		if (pusb->isfound) {
			if(pusb->hReadHandle == INVALID_HANDLE_VALUE) { //如果读句柄无效
				;//读句柄无效
			} else {
				CLock lock(pusb->m_param.g_Lock);
				ReadFile(pusb->hReadHandle,
					ReadBuffer,
					READBUFLEN,
					NULL,
					&(pusb->ReadOverlapped));
			}

			//等待事件触发
			WaitForSingleObject(pusb->ReadOverlapped.hEvent, INFINITE);
			//如果等待过程中设备被拔出，也会导致事件触发，但此时MyDevFound
			//被设置为假，因此在这里判断MyDevFound为假的话就进入下一轮循环。
			if (pusb->isfound == FALSE)
                continue;

			//如果设备没有被拔下，则是ReadFile函数正常操作完成。
			//通过GetOverlappedResult函数来获取实际读取到的字节数。
			GetOverlappedResult(
                pusb->hReadHandle,
                &(pusb->ReadOverlapped),
				&(pusb->m_param.readbuflen), 
                TRUE);
			//如果实际接收字节数不为0, 则将读到的数据显示到信息框中
			if (pusb->m_param.readbuflen != 0) {
				if (ReadBuffer[1] == CMD_DEBUG_HEADER) { //调试信息
                    CLock lk(log_list_lock);
                    logs_.push(std::string((char*)ReadBuffer + 4));
					continue;
				}

				plog->WriteUSBdata("接收", ReadBuffer + 1, 64);
				if (ReadBuffer[1] != CMD_RECVHEADER) {
					plog->WriteNormalLog("来自下层未知通知的消息值为:%0X", ReadBuffer[2]);
					continue;
				}

                unsigned char cmd = ReadBuffer[2];
                plog->WriteNormalLog("来自下层通知的消息值为:%0X", cmd);
				//格式ReadBuffer : 0x00(1Byte), head(1Byte), cmd(1Byte), len(1Byte), data(N Byte), check(1Byte), end(1Byte)
				if (cmd == CMD_STAMPER_DOWN             ||			
					cmd == CMD_STAMPER_ARMBACK          ||  
                    cmd == CMD_STAMPER_COMPLETE         ||
                    cmd == CMD_STAMPER_ERR              ||
					cmd == CMD_STAMPER_PAPERDOORCLOSE   ||
					cmd == CMD_STAMPERPROC_ERROR        ||
                    cmd == CMD_SIDEDOOR_CLOSE           ||
                    cmd == CMD_TOP_CLOSE                ||
                    cmd == CMD_ELEC_LOCK
					) {
                            if (CMD_STAMPER_COMPLETE == cmd) {
                                CLock lk(notify_lock_);
                                notify_.uMsg = cmd;
                                notify_.wParam = ReadBuffer[4];
                                notify_.lParam = 0;
                                notify_.data = &ReadBuffer[5];
                                notify_.len = 46;
                            } else {
                                CLock lk(notify_lock_);
                                notify_.uMsg = cmd;
                                notify_.wParam = ReadBuffer[4];
                                notify_.lParam = 0;
                                notify_.data = NULL;
                                notify_.len = 0;
                            }

                            SetEvent(notify_ev_);
                            continue;
				}
				
                memcpy(pusb->m_param.ReadBuffer, ReadBuffer, READBUFLEN);
                if (cmd == CMD_RESET)
                    SetEvent(reset_ev);

                if (cmd == send_cmd_) {
                    SetEvent(pusb->recvevent);
                    send_cmd_ = 0xff;
                }
			}			
		} else {
			//阻塞线程，直到下次事件被触发
			WaitForSingleObject(pusb->ReadOverlapped.hEvent, INFINITE);
		}
	}

    plog->WriteNormalLog("ReadDataFromUsbThread->接收消息线程结束");
	return 0;
}

UINT __stdcall CCommunicateToUSB::WriteLogThread(LPVOID pParam)
{
    CCommunicateToUSB* pusb = (CCommunicateToUSB*)pParam;
    if (NULL == pusb)
        return 0;

    CLog* plog = CLog::sharedLog();
    while (TRUE)
    {
        //线程退出通知
        int retexit = 0;
        retexit = WaitForSingleObject(pusb->exitevent, 0);
        if (retexit == WAIT_OBJECT_0)
            break;

        CLock lk(log_list_lock);
        if (logs_.empty()) {
            Sleep(5000);
            continue;
        }

        std::string log = logs_.front();
        logs_.pop();
        plog->WriteUSBDebugLog((unsigned char*)log.c_str(), 0);
    }

    return 0;
}

int CCommunicateToUSB::sendData2Usb(void)
{
    CLog *plog = CLog::sharedLog();
	int iresult = 0;
	BOOL Result;
	UINT LastError;
	if (!isfound || hWriteHandle==INVALID_HANDLE_VALUE||
		DataInSending==TRUE)
	{
        if (!isfound)
            plog->WriteNormalLog("sendData2Usb->设备已关闭");
        if (hWriteHandle == INVALID_HANDLE_VALUE)
            plog->WriteNormalLog("sendData2Usb->写数据句柄无效");
        if (DataInSending == TRUE)
            plog->WriteNormalLog("sendData2Usb->数据正在发送中");

		return -1;
	}
	//设置正在发送标志
	//TRACE("hWriteHandle：%d\n",(int)hWriteHandle);

	plog->WriteUSBdata("发送", m_param.WriteBuffer+1, 64);

	DataInSending=TRUE;
	Result=WriteFile(hWriteHandle,
		&(m_param.WriteBuffer), 
		WRITEBUF_LEN,
		NULL,
		&WriteOverlapped);
	//TRACE("hWriteHandle：%d\n",(int)hWriteHandle);
	if(Result==FALSE)
	{
		LastError=GetLastError();
		if((LastError!=ERROR_IO_PENDING)&&(LastError!=ERROR_SUCCESS))
		{
			DataInSending=FALSE;
			iresult = LastError;
            plog->WriteNormalLog("CCommunicateToUSB::sendData2Usb->WriteFile, err:%d", LastError);
		}
	}
	else
	{
		DataInSending=FALSE;
	}

	return iresult;
}

void CCommunicateToUSB::signal()
{
    SetEvent(reset_ev);
}
