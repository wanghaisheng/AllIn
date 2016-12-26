#pragma once
extern "C" {
#include "hidsdi.h"
#include "setupapi.h"
}
#include "WLock.h"
#include <list>
#include <map>
#include <vector>
#include <Dbt.h>
#include <string>
using std::string;
using std::list;
using std::map;
using std::pair;


class CCtrlUSB
{
public:
	static CCtrlUSB* sharedCCtrlUSB(void);	
	~CCtrlUSB(void);
private:
	CCtrlUSB(void);
	static CCtrlUSB* m_pInstance;	

	class CGarbo   //它的唯一工作就是在析构函数中删除CSingleton的实例  
	{  
	public:  
		~CGarbo();		
	};  
	static CGarbo Garbo;
public:
	
	//接收信息的窗口句柄
	HWND m_hWnd;

	//用来注册设备通知事件用的广播接口。
	//要使用该结构体，需要在StdAfx.h中将增加语句#define WINVER 0x0500
	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;

	//PVOID m_CallBack;
	std::vector<PVOID> vecCallback;

    Mutex path_mutex_;
	string strdevpath;
public:
	void Init(HWND hWnd);

	//设备插拔接收信息
	LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);

	//获取所有设备路径
	void FindDevPath();

	//判断是否是印控机设备
	bool IsST_USBDevPath(const char* path);

	//获取设备路径map<string, bool>
	//string 设备路径
	//bool true 已经有使用，false,未被使用
	map<string, bool>* CCtrlUSB::GetDevPathMap();
	//注册回调函数
	void RegisterDevCallBack(PVOID func);
	
	string GetDevPath();
};

