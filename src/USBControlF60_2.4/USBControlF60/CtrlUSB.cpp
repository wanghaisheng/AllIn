#include "StdAfx.h"
#include "CtrlUSB.h"
#include "USBControlF60.h"
#include <algorithm>
#include "Log.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")

#define USB_VID 0x0483
#define	USB_PID 0x5750


CCtrlUSB::CGarbo::~CGarbo()
{  
	if(CCtrlUSB::m_pInstance)  
	{
		delete CCtrlUSB::m_pInstance;  
		CCtrlUSB::m_pInstance = NULL;
	}
} 

CCtrlUSB* CCtrlUSB::m_pInstance = NULL;
CCtrlUSB::CCtrlUSB(void)
{
}


CCtrlUSB::~CCtrlUSB(void)
{
	CCtrlUSB::m_pInstance = NULL;
	m_hWnd = NULL;
	//m_MapDevPath.clear();
}

CCtrlUSB* CCtrlUSB::sharedCCtrlUSB(void)
{	
	if (m_pInstance == NULL)	{
		//Lock lock(cs);
		if (m_pInstance == NULL)
		{
			m_pInstance = new CCtrlUSB();	
		}			
	}
	return m_pInstance;
}


void CCtrlUSB::Init(HWND hWnd)
{
	if (!hWnd)
	{
		m_hWnd = NULL;
	}
	else
	{
		m_hWnd = hWnd;
		
		memset(&DevBroadcastDeviceInterface, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
		GUID HidGuid;
		//获取HID设备的接口类GUDI
		HidD_GetHidGuid(&HidGuid);
		//设置DevBroadcastDeviceInterface结构体，用来注册设备改变时的通知
		DevBroadcastDeviceInterface.dbcc_size=sizeof(DevBroadcastDeviceInterface);
		DevBroadcastDeviceInterface.dbcc_devicetype=DBT_DEVTYP_DEVICEINTERFACE;
		DevBroadcastDeviceInterface.dbcc_classguid=HidGuid;
		//注册设备改变时收到通知
		::RegisterDeviceNotification(m_hWnd, &DevBroadcastDeviceInterface,
			DEVICE_NOTIFY_WINDOW_HANDLE);//DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
	}

	//m_MapDevPath.clear();
    CLock lk(path_mutex_);
	strdevpath.empty();
}

//设备插拔接收信息
LRESULT CCtrlUSB::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_DEVICEINTERFACE pdbi;
	pdbi=(PDEV_BROADCAST_DEVICEINTERFACE)lParam;
	string tmppathname = "";
	CLog* plog = CLog::sharedLog();
	switch(wParam)
	{
		//设备连接事件
	case DBT_DEVICEARRIVAL:
		if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
		{			
			if (IsST_USBDevPath(pdbi->dbcc_name))
			{
				tmppathname = pdbi->dbcc_name; //保存发生状态改变的设备的路径名
                CLock lk(path_mutex_);
				strdevpath = tmppathname;
				for(unsigned int i = 0 ; i< vecCallback.size();++i)
				{
					pDevConnectCallBack pcb = (pDevConnectCallBack)(vecCallback.at(i));
					if (pcb)
					{
						plog->WriteNormalLog("USB设备连接");
						pcb(tmppathname.c_str(), 1);
					}
				
			      }			
				}
		}
		break;
		//设备拔出事件
	case DBT_DEVICEREMOVECOMPLETE: 
		if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
		{
			tmppathname = pdbi->dbcc_name;
            CLock lk(path_mutex_);
			std::transform(tmppathname.begin(), tmppathname.end(), tmppathname.begin(), toupper);
			std::transform(strdevpath.begin(), strdevpath.end(), strdevpath.begin(), toupper);

			if(tmppathname == strdevpath)
			{
				for(unsigned int i = 0 ; i< vecCallback.size();++i)
				{
					pDevConnectCallBack pcb = (pDevConnectCallBack)(vecCallback.at(i));
					if (pcb)
					{
							
						plog->WriteNormalLog("USB拔出");
						pcb(tmppathname.c_str(), 0);
					}
				}
				strdevpath.empty();
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

//获取设备路径
void CCtrlUSB::FindDevPath()
{
	GUID HidGuid;
	HDEVINFO hDevInfoSet;
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDevDetailData;
	HIDD_ATTRIBUTES DevAttributes;
	DWORD MemberIndex;	
	BOOL Result;
	DWORD RequiredSize;	
	HANDLE hDevHandle;

	DevInterfaceData.cbSize=sizeof(DevInterfaceData);
	DevAttributes.Size=sizeof(DevAttributes);

	//返回HID设备的GUID
	HidD_GetHidGuid(&HidGuid);

	//得到设备信息集指针
	hDevInfoSet=SetupDiGetClassDevs(&HidGuid,
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	MemberIndex=0;

	while(TRUE)
	{
		//获取设备接口信息
		Result=SetupDiEnumDeviceInterfaces(hDevInfoSet,
			NULL,
			&HidGuid,
			MemberIndex,
			&DevInterfaceData);
		if(Result==FALSE) 
		{
			if ( GetLastError()!=NO_ERROR &&
				GetLastError()!=ERROR_NO_MORE_ITEMS )
			{
				printf("ERROR: (%d)",GetLastError());				
			}
			break;
		}
		MemberIndex++;
		//获取设备接口详细信息,第一次失败,但可获取数据大小RequiredSize
		Result=SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			NULL,
			NULL,
			&RequiredSize,
			NULL);
		pDevDetailData=(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		if(pDevDetailData==NULL) //如果内存不足，则直接返回。
		{
			//销毁一个设备信息集合，并且释放所有关联的内存
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return;
		}

		//获取设备路径
		pDevDetailData->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		Result=SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			pDevDetailData,
			RequiredSize,
			NULL,
			NULL);

		if(Result==TRUE) 		
		{
			//	保存本次读到的地址
			hDevHandle=CreateFile(pDevDetailData->DevicePath, 
				NULL,
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if(hDevHandle!=INVALID_HANDLE_VALUE)
			{
				Result=HidD_GetAttributes(hDevHandle,
					&DevAttributes);
				//关闭刚刚打开的设备
				CloseHandle(hDevHandle);
				if(Result==FALSE) 
				{
					free(pDevDetailData);
					pDevDetailData = NULL;
					continue;
				}
				if(DevAttributes.VendorID==USB_VID &&
					DevAttributes.ProductID==USB_PID )
				{
					//找到正确的设备才保存设备路径保存，结束查找设备	
                    CLock lk(path_mutex_);
					strdevpath = pDevDetailData->DevicePath;
					free(pDevDetailData);
					pDevDetailData = NULL;
					break;
				}
			}
		}	
		
		free(pDevDetailData);
		pDevDetailData = NULL;
		continue;
	}

	SetupDiDestroyDeviceInfoList(hDevInfoSet);
}

bool CCtrlUSB::IsST_USBDevPath(const char* path)
{
	BOOL Result;
	HANDLE hDevHandle;
	HIDD_ATTRIBUTES DevAttributes;	
	DevAttributes.Size=sizeof(DevAttributes);	

	hDevHandle=CreateFile(path, 
		NULL,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hDevHandle!=INVALID_HANDLE_VALUE)
	{
		Result=HidD_GetAttributes(hDevHandle,
			&DevAttributes);
		//关闭刚刚打开的设备
		CloseHandle(hDevHandle);
		if(Result != FALSE) 
		{
			if(DevAttributes.VendorID==USB_VID &&
				DevAttributes.ProductID==USB_PID )
			{
				return true;
			}
		}
	}

	return false;
}

map<string, bool>* CCtrlUSB::GetDevPathMap()
{
	return NULL;//&m_MapDevPath;
}

void CCtrlUSB::RegisterDevCallBack(PVOID func)
{
	if(func!=0)
	{
	vecCallback.push_back(func);
	}
	else
	{
		vecCallback.clear();
	}
	//m_CallBack = func;
}

string CCtrlUSB::GetDevPath()
{
    CLock lk(path_mutex_);
	return strdevpath;
}