#include "StdAfx.h"
#include <iostream>
#include "ContrlledStamper.h"
#include "CommunicateToUSB.h"
#include "CtrlUSB.h"
#include "algorithm.h"
#include "Log.h"
#include <vector>
#include <math.h>

struct point
{
	point(unsigned short  a,unsigned short b)

	{
		_x = a;
			_y= b;
	}
	unsigned short _x;
	unsigned short _y;
};


#define SEAL_TABLE_WIDTH        (275.0)                        //电机运行最大宽度
#define SEAL_TABLE_HEIGHT       (250.0)                        //电机运行最大高度

#define SEAL_TABLE_X_MIN        (18)                         //限制X方向的最小值
#define SEAL_TABLE_X_MAX        (SEAL_TABLE_WIDTH  - 18)     //限制X方向的最大值

#define SEAL_TABLE_Y_MIN        (63)                           //限制Y方向的最小值
#define SEAL_TABLE_Y_MAX        (SEAL_TABLE_HEIGHT  - 5) 

const unsigned short MAX_STAMPER_MEM = 512;

#define ASSIGMENT(a ,b)    \
	memset(szPoint,0,sizeof(szPoint));\
	algorithm::convert::U16ToLeBuf(a,szPoint,0);\
	memcpy(points,szPoint,sizeof(szPoint));\
	points +=sizeof(szPoint);\
    memset(szPoint,0,sizeof(szPoint));\
	algorithm::convert::U16ToLeBuf(b,szPoint,0);\
	memcpy(points,szPoint,sizeof(szPoint));\
	points +=sizeof(szPoint);

CCommunicateToUSB* CContrlledStamper::m_USB=NULL;
CContrlledStamper::CContrlledStamper(void)
{
	m_USB = new CCommunicateToUSB;
	if (m_USB)
	{
		m_USB->InitUSB();
	}

	InitializeCriticalSection(&m_csIORes);
	CCtrlUSB* pCCtrlUSB = CCtrlUSB::sharedCCtrlUSB();
	pCCtrlUSB->RegisterDevCallBack(&CContrlledStamper::DevConnectCallBack);
// 	EnterCriticalSection(&m_csIORes);	
// 	LeaveCriticalSection(&m_csIORes);
}


CContrlledStamper::~CContrlledStamper(void)
{
	if (m_USB)
	{
		/*m_USB->CloseDev();*/
		
		delete m_USB;
		m_USB = NULL;
	}

	DeleteCriticalSection(&m_csIORes);
}


int CContrlledStamper::SendCommand(SendPackage* sPackage, ReceivePackage* rPackage)
{
    CLock lk(send_mtx_);
/*	EnterCriticalSection(&m_csIORes);	*/
	if(STF_SUCCESS != m_USB->send(sPackage))
	{
/*		LeaveCriticalSection(&m_csIORes);*/
		return STF_ERR_SENDFAIL;
	}

    rPackage->m_cmd = sPackage->m_cmd;
	if(STF_SUCCESS != m_USB->receive(rPackage))
	{
/*		LeaveCriticalSection(&m_csIORes);*/
		return STF_ERR_RECVFAIL;
	}

/*	LeaveCriticalSection(&m_csIORes);*/
	return STF_SUCCESS;
}

int CContrlledStamper::SendCommand(
    unsigned char cmd, 
    unsigned char paramlen, 
    unsigned char *params, 
    void* pRetData, 
    int len)
{
	CLog* plog = CLog::sharedLog();

	SendPackage sendPackage(cmd, paramlen, params);
	ReceivePackage rPackage;
	rPackage.m_cmd = sendPackage.m_cmd;
	int nRet = SendCommand(&sendPackage, &rPackage);
	if(STF_SUCCESS != nRet){
		plog->WriteNormalLog(_T("Error! SendCommand(%d, %d, %s) = %d"), cmd, paramlen, params, nRet);
		return nRet;
	}

	if(rPackage.m_cmd != cmd){
		plog->WriteNormalLog(_T("Error! 从机返回的应答码(%0X, %0X)不正确"), rPackage.m_cmd, cmd);
		return STF_ERR_COMMUNICATION;
	}

	if(pRetData){
		len = len>rPackage.m_len?rPackage.m_len:len;
		unsigned char* pData = static_cast<unsigned char*>(pRetData);
		if (len>0)
			memcpy(pData, rPackage.m_data, len);
	}
	return nRet;
}

int CContrlledStamper::firewareUpdate(unsigned char cmd, unsigned char* data, int len)
{
	if (cmd<CMD_UPDATE_NOTICE || cmd>CMD_FIREWARE_MCUCHECK)
	{
		return STF_ERR_SENDFAIL;
	}

	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(cmd, len, data, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::DevConnectCallBack(const char* DevPath, unsigned int uMsg)
{
	if(uMsg == 0)
	{
        CCommunicateToUSB::signal();
	 m_USB->CloseDev();
	}
	return  0;
}

int CContrlledStamper::RegisterDevCallBack(pfnDevMsgCallBack pfn)
{
	if (m_USB)
	{
		m_USB->RegisterDevCallBack((PVOID)pfn);
		return 0;
	}
	
	return -1;
}

int CContrlledStamper::OpenDev(const char *file)
{
	if (m_USB)
	{
		return	m_USB->OpenDev(file);
	}

	return 1;
}

int CContrlledStamper::CloseDev(void)
{
	if (m_USB)
	{
		m_USB->CloseDev();
	}
	return 0;
}


/*   复位 
 *   返回值0表示成功，1 失败
 */

int CContrlledStamper::QuitMaintainMode(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_QUIT_MAINTAIN, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
/*   重新启动 
 *   返回值0表示成功，1 失败
 */
int CContrlledStamper::Restart(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_RESTART, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
/*   获取最系统信息
 *   返回值 0 获取信息成功,  -2 发送数据错误
 */
int CContrlledStamper::GetSystemMsg(char* pRtn, int nLen)
{
	return SendCommand(CMD_GETSYSTEMMSG, 0, NULL, pRtn, nLen);
}

int CContrlledStamper::GetFirwareVer(OUT char* strVer, int len)
{	
	return SendCommand(CMD_GETSYSVER, 0, NULL, strVer, len);	
}

int CContrlledStamper::SetFirwareSN(char* strSN, int len)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char* sn = new unsigned char[len];
	if (len>0)
		memcpy_s(sn, len, strSN, len);
	ret = SendCommand(CMD_SETDEVSN, len, sn, &recvdata, sizeof(recvdata));
	delete[] sn;
	sn = NULL;
	return ret;
}

int CContrlledStamper::GetFirwareSN(OUT char* strSN, int len)
{
	  int ret = 0 ;
      ret= SendCommand(CMD_GETDEVSN, 0, NULL, strSN, len);

	  if(ret == STF_SUCCESS)
	  {
		  if(algorithm::project::ZeroArrayJudge(strSN,len))
		  {

			  ret = 0;
		  }
		  else
		  {
			   ret = -3;
		  }
	  }
	  else
	  {
		  ret = -2;
	  }
	return ret;
}

int CContrlledStamper::GetDevStatus(OUT unsigned char* strStatus, int len)
{
	return SendCommand(CMD_QUERY_DEVSTATUS, 0, NULL, strStatus, len);
}
int CContrlledStamper::MaintenanceMode(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_SET_DEVPROTECT, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}

int CContrlledStamper::GetInfraRedStatus(OUT char* strInfraRed, int len)
{	
	return SendCommand(CMD_GET_INFRARED_STATUS, 0, NULL, strInfraRed, len);	
}

int CContrlledStamper::GetSealPresent(OUT char* strSealStatus, int len)
{
	return SendCommand(CMD_SEAL_PRESENT, 0, NULL, strSealStatus, len);	
}
int CContrlledStamper::GetDoorsPresent(OUT char* strDoorsStatus, int len)
{
	return SendCommand(CMD_DEV_DOOR_STATUS, 0, NULL, strDoorsStatus, len);	
}
int CContrlledStamper::OpenDoorPaper(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_OPENDOOR_PAPER, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::OpenDoorSafe(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_OPENDOOR_SAFE, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::CloseDoorSafe(void)
{
	char recvdata = 0;	
	int ret = 0;
	ret = SendCommand(CMD_CLOSENDOOR_SAFE, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::LinghtCtrl(char light, int op)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = light;
	params[1] = op;
	ret = SendCommand(CMD_LIGHT_CTRL, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::MoveX(unsigned short x_point)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = (unsigned char)x_point;
	params[1] = (unsigned char)(x_point>>8);
	ret = SendCommand(CMD_X_MOVE, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::MoveY(unsigned short y_point)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = (unsigned char)y_point;
	params[1] = (unsigned char)(y_point>>8);
	ret = SendCommand(CMD_Y_MOVE, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::TurnSeal(unsigned short angle)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = (unsigned char)angle;
	params[1] = (unsigned char)(angle>>8);
	ret = SendCommand(CMD_STAMPER_TURN, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}
int CContrlledStamper::StartStamper(STAMPERPARAM *pstamperparam)
{
	char recvdata[4] = {};	
	int ret = 0;
	unsigned char params[18] = {0};
	//RFID版修改
    memcpy(params, &pstamperparam->serial_number, sizeof(pstamperparam->serial_number));
    const unsigned char begin = 4;
	params[begin] = pstamperparam->seal[0];
	params[begin + 1] = pstamperparam->seal[1];
    params[begin + 2] = pstamperparam->seal[2];
    params[begin + 3] = pstamperparam->seal[3];
    params[begin + 4] = pstamperparam->isPadInk;
    params[begin + 5] = (unsigned char)(pstamperparam->x_point);
    params[begin + 6] = (unsigned char)(pstamperparam->x_point >> 8);
    params[begin + 7] = (unsigned char)(pstamperparam->y_point);
    params[begin + 8] = (unsigned char)(pstamperparam->y_point >> 8);
    params[begin + 9] = (unsigned char)(pstamperparam->angle);
    params[begin + 10] = (unsigned char)(pstamperparam->angle >> 8);
    params[begin + 11] = (unsigned char)(pstamperparam->w_time);
    params[begin + 12] = (unsigned char)(pstamperparam->w_time >> 8);
    params[begin + 13] = pstamperparam->type;

	ret = SendCommand(CMD_BEGINSTAMP_NORMAL, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		if(recvdata[0] == 0x00)
		{
			ret = 0;
		}
		else
		{
			ret = recvdata[1];
		}
	}

	return ret;
}
int CContrlledStamper::SelectStamper(unsigned int serial, unsigned int seal_id, char isPadInk)
{	
    char recvdata[2] = { 0 };
	int ret = 0;
	unsigned char params[9] = {0};
    memcpy(params, &serial, sizeof(serial));
    memcpy(params + sizeof(serial), &seal_id, sizeof(seal_id));
	params[8] = isPadInk;
	ret = SendCommand(CMD_SELECTSTAMP, sizeof(params), params, recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata[0];
        if (ret == 1)
            ret = recvdata[1];
	}

	return ret;
}
int CContrlledStamper::CancleStamper(void)
{	
	char recvdata = 0;	
	int ret = 0;	
	ret = SendCommand(CMD_BEGINSTAMP_CANCLE, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}
int CContrlledStamper::BeepCtrl(char beep, char interval)
{	
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = beep;
	params[1] = interval;
	ret = SendCommand(CMD_BEEP_CTRL, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}
int CContrlledStamper::LightBrightness(char light, char brightness)
{	
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = light;
	params[1] = brightness;
	ret = SendCommand(CMD_LIGHTING_BRIGHTNESS, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}

int CContrlledStamper::SealBack(void)
{
	char recvdata = 0;	
	int ret = 0;	
	ret = SendCommand(CMD_SEALBACK, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}
int CContrlledStamper::InTestMode(void)
{
	char recvdata = 0;	
	int ret = 0;	
	ret = SendCommand(CMD_INTESTMODE, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}
int CContrlledStamper::OutTestMode(void)
{
	char recvdata = 0;	
	int ret = 0;	
	ret = SendCommand(CMD_OUTTESTMODE, 0, NULL, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}
int CContrlledStamper::GetMacAdress(char num, OUT unsigned char* strmac, int len)
{
	unsigned char param = num;
	
	int ret = 0;	
	ret = SendCommand(CMD_GETMAC, sizeof(param), &param, strmac, len);	
	return ret;	
}
int CContrlledStamper::BindMac(char op, char num, char* strmac, int len)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[8] = {0};
	params[0] = op;
	params[1] = num;

	if (len>0)
	{
		memcpy_s(params+2, len, strmac, 6);
	}

	ret = SendCommand(CMD_MACPCBIND, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;
}

int CContrlledStamper::SetDevCode(char* code, int len)
{
	int ret = 0;
	char recvdata = 0;	
	unsigned char* data = new unsigned char[len];
	if (len>0)
		memcpy_s(data, len, code, len);
	ret = SendCommand(CMD_SETDEVCODE, len, data, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}
	delete[] data;
	data = NULL;
	return ret;
}
int CContrlledStamper::GetDevCode(OUT char* code, int len)
{
	return SendCommand(CMD_GETDEVCODE, 0, NULL, code, len);	
}

//Debug调试信息开关
int CContrlledStamper::DebugLogSwitch(char op)
{
	char recvdata = 0;	
	unsigned char param = op;
	int ret = 0;	
	ret = SendCommand(CMD_SET_USB_DEBUG, sizeof(param), &param, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}

//int CContrlledStamper::Calibration( char* points,int len)
//{
//	memset(points,0,len);
//	//获取印章ID
//	 char str[256] = {0};
//	 char szID[4] = {0};
//	int posStart ; 
//	int posEnd ;
//	int ret = -1;
//
//	 char data[4]={0};
//
//	data[0] = (unsigned char)0x12;
//	data[1] = (unsigned char)0x3c;
//	data[2] = (unsigned char)0xf0;
//	data[3] = (unsigned char)0xe6;
//	 char* pData = static_cast< char*>(points);
//	memcpy(pData,data,4);
//
//	ret =::FGetSealPresent((char*)str, sizeof(str));
//	for(unsigned int i  = 0 ; i <6 ; i++)
//	{
//	 posStart = 1;
//	 posEnd = 5;
//	 memset(szID,0,4);
//	 posStart += i*4 ;
//	 posEnd += i*4;
//	 memcpy(szID,(void*)&str[posStart],4);
//	 points[0] = (unsigned char)0x12;
//	 points[1] = (unsigned char)0x3c;
//	 points[2] = (unsigned char)0xfa;
//	 points[3] = (unsigned char)0xf5;
//	 if(szID[0]&0x01||szID[1]&0x01||szID[2]&0x01||szID[3]&0x01)
//	 {
//		 
//		 //选择第一个被寻找到的印章
//		 FStartStamper((char*)szID, 1, (unsigned short)18, (unsigned short)60,0,15);
//		 DWORD dwTimeOut = 15000;
//		 DWORD dTimeOut = 0;
//		 DWORD dStartTime = GetTickCount();
//		 while(1)
//		 {
//			 Sleep(500);
//			 if(m_USB->GetStamperStauts()==2)
//			 {
//				 FStartStamper((char*)szID, 1, (unsigned short)250, (unsigned short)245,0,15);
//				 break;
//			 }
//			 DWORD	dEndTime = GetTickCount();
//			 DWORD  dTimeOut = dEndTime - dStartTime;
//
//
//			 if (dTimeOut> dwTimeOut)
//			 {
//				 break;
//			 }
//		 }
//		
//	 }
//	
//
//	}
//
//	//FStartStamper((char*)stampID, g_SThis->yinyou.GetCheck(), g_SThis->x, g_SThis->y,g_SThis->angle,g_SThis->wtime);
//	return  0;
//}

int CContrlledStamper::CalculatePos(double* x1, double* y1, double* x2, double* y2, double* scalex,double*scaley)
{
	int sx1 = 0 ;
	int sx2 = 0;
	int sy1 = 0; 
	int sy2 = 0 ;
	double calex = (sx2-sx1)/(x2-x1);
	scalex = &calex;
	double caley = (sy2 -sy1 )/(y2-y1);
	scaley = &caley;
	return   0;
}
//该函数为正确函数
int CContrlledStamper::Calibration( char* points,int len)
{

	//测试
	memset(points,0,len);
	BYTE szPoint[2];
	//获取印章ID
	BYTE str[256] = {0};
	BYTE szID[4] = {0};
	BYTE szSelectID[4] ={0};
	int posStart ; 
	int posEnd ;
	int  ret = -1;
	char data[8]={0};
	unsigned short _x1 =0 ;
	unsigned short _y1 =0 ;
	unsigned short _x2 =0 ;
	unsigned short _y2 =0 ;

	unsigned short _x3 =0 ;
	unsigned short _y3 =0 ;
	unsigned short _x4 =0 ;
	unsigned short _y4 =0 ;
	unsigned short _x5 =0 ;
	unsigned short _y5 =0 ;

	_x1 =(unsigned short)  SEAL_TABLE_X_MIN;
	_y1 =(unsigned short)  SEAL_TABLE_Y_MIN;
	_x2 =(unsigned short)  SEAL_TABLE_X_MAX;
	_y2 =(unsigned short)  SEAL_TABLE_Y_MAX;

	_x3 =(unsigned short)SEAL_TABLE_X_MIN ;
	_y3 =(unsigned short) SEAL_TABLE_Y_MAX;
	_x4 =(unsigned short) SEAL_TABLE_X_MAX;
	_y4 = (unsigned short) SEAL_TABLE_Y_MIN;

	_x5 =(unsigned short)( SEAL_TABLE_X_MAX+SEAL_TABLE_X_MIN)/2;
	_y5 = (unsigned short) (SEAL_TABLE_Y_MIN+SEAL_TABLE_Y_MAX)/2;



	char * pOrigPoints = points;
	if(len>15)
	{
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);
	
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x3,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y3,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x4,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y4,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	}
	points = pOrigPoints;
	ret =::FGetSealPresent((char*)str, sizeof(str));
	for(unsigned int i  = 0 ; i <6 ; i++)
	{
		posStart = 1;
		posEnd = 5;
		memset(szID,0,4);
		posStart += i*4 ;
		posEnd += i*4;
		memcpy(szID,(void*)&str[posStart],4);
		if(szID[0]&0x01||szID[1]&0x01||szID[2]&0x01||szID[3]&0x01)
		{
			memcpy(szSelectID,szID,sizeof(szSelectID));
			break;
		}

	}

	 //选择第一个被寻找到的印章
	FStartStamper(0, (char*)szID, 1, _x1,_y1,0,15);
	DWORD dwTimeOut = 15000;
	DWORD dTimeOut = 0;
	DWORD dStartTime = GetTickCount();

	while(1)
	{
		Sleep(100);
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			FStartStamper(0, (char*)szID, 1, _x2, _y2,0,15);
			break;
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		if (dTimeOut> dwTimeOut)
		{
			break;
		}
	}
	Sleep(15000);

	FStartStamper(0, (char*)szID, 1, _x3, _y3,0,15);
	Sleep(15000);
	FStartStamper(0, (char*)szID, 1, _x4, _y4,0,15);

	Sleep(15000);
	FStartStamper(0, (char*)szID, 1, _x5, _y5,0,15);

	/*while(1)
	{
		Sleep(100);
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			FStartStamper((char*)szID, 1, _x3, _y3,0,15);
			break;
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		if (dTimeOut> dwTimeOut)
		{
			break;
		}
	}

	while(1)
	{
		Sleep(100);
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			FStartStamper((char*)szID, 1, _x4, _y4,0,15);
			break;
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		if (dTimeOut> dwTimeOut)
		{
			break;
		}
	}
*/
	return  0;
}


#if 0
//物理坐标沿逆时针方向上
const int WL_POINT_ARRAY[5][2]={{257,63},{257,245},
{18,245},{18,63},
{137,154}
};
//像素坐标沿逆时针方向上
const int XS_POINT_ARRAY[5][2]={{231,286},{214,1807},
{2231,1846},{2235,305},
{1230,1058}
};


const int szStamper[80][5] = {0};
#endif
//物理坐标沿逆时针方向上
const int WL_POINT_ARRAY[20][2]={
	{257,63},{257,145},{137,154},{137,63},{197,108},
	{257,154},{257,245},{137,245},{137,154},{197,199},
	{137,154},{137,245},{18,245},{18,54},{137,154},
	{137,63},{137,154},{18,54},{18,63},{77,108}
};
//像素坐标沿逆时针方向上
const int XS_POINT_ARRAY[20][2]={
	{267,291},{265,1038},{1274,1030},{1264,271},{765,644},
	{265,1038},{283,1795},{1284,1805},{1274,1030},{774,1413},
	{1274,1030},{1284,1805},{2281,1780},{2279,1016},{1786,1408},
	{1264,271},{1274,1030},{2279,1016},{2257,262},{1779,635}
};
int CContrlledStamper::STCalculatePosition(int stanpPointx,int stampPointy,char* points,int len)
{
	//象限位置判断
	int cur_idx =-1;
	for (int i=0;i<20;++i)
	{
		if((stanpPointx-XS_POINT_ARRAY[(i/5+1)*5-1][0])*(XS_POINT_ARRAY[i][0]-XS_POINT_ARRAY[(i/5+1)*5-1][0])>=0&&
			(stampPointy-XS_POINT_ARRAY[(i/5+1)*5-1][1])*(XS_POINT_ARRAY[i][1]-XS_POINT_ARRAY[(i/5+1)*5-1][1])>=0)
		{
			cur_idx = i;
			break;
		}
	}
	//所给的点不在象限内
	if (cur_idx<0 || cur_idx>3)
	{
		return -1;
	}
	//物理用印位置计算
	double d_x=0.0;
	double d_y=0.0;
	if(cur_idx==(cur_idx/5+1)*5-1)
	{
		d_x = WL_POINT_ARRAY[cur_idx][0];
		d_y = WL_POINT_ARRAY[cur_idx][1];
	}
	else
	{
		d_x = double((stanpPointx-XS_POINT_ARRAY[(cur_idx/5+1)*5-1][0])*(WL_POINT_ARRAY[cur_idx][0]-WL_POINT_ARRAY[(cur_idx/5+1)*5-1][0]))/(XS_POINT_ARRAY[cur_idx][0]-XS_POINT_ARRAY[(cur_idx/5+1)*5-1][0])+WL_POINT_ARRAY[(cur_idx/5+1)*5-1][0];
		d_y = double((stampPointy-XS_POINT_ARRAY[(cur_idx/5+1)*5-1][1])*(WL_POINT_ARRAY[cur_idx][1]-WL_POINT_ARRAY[(cur_idx/5+1)*5-1][1]))/(XS_POINT_ARRAY[cur_idx][1]-XS_POINT_ARRAY[(cur_idx/5+1)*5-1][1])+WL_POINT_ARRAY[(cur_idx/5+1)*5-1][1];

	}
	//sprintf(points,"%f,%f",d_x,d_y);
	//去整值
	unsigned short  posx1= static_cast<unsigned short>(algorithm::convert::round(d_x));
	unsigned short  posy1 = static_cast<unsigned short>(algorithm::convert::round(d_y));
	char * pOrigPoints = points;
	memset(points,0,len);
	BYTE szPoint[2];
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(posx1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(posy1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));

	points =  pOrigPoints;

	return 0 ;
}


int CContrlledStamper::CalculatePosition(int stamperPointX,int stanperPointY,char* points,int len)
{

	//物理坐标,测试使用
	spoint wtopLeft;
	spoint wleftButtom;
	spoint wtopRight;
	spoint wrightButtom;
	BYTE szPoint[2];
	wtopLeft._x    = SEAL_TABLE_X_MIN;
	wtopLeft._y    = SEAL_TABLE_Y_MAX ;
	wleftButtom._x = SEAL_TABLE_X_MIN;
	wleftButtom._y = SEAL_TABLE_Y_MIN;
	wtopRight._x  =   SEAL_TABLE_X_MAX;
	wtopRight._y  =   SEAL_TABLE_Y_MAX;
	wrightButtom._x = SEAL_TABLE_X_MAX;
	wrightButtom._y = SEAL_TABLE_Y_MIN;


	//测试像素坐标

	spoint pixtopLeft;
	spoint pixleftButtom;
	spoint pixtopRight;
	spoint pixrightButtom;
	spoint pixStampPoint;
	pixtopLeft._x =  2208;
	pixtopLeft._y =  1825;
	pixleftButtom._x= 2225;
	pixleftButtom._y= 253;
	pixtopRight._x= 164;
	pixtopRight._y= 1778;
	pixrightButtom._x= 180;
	pixrightButtom._y= 224;
	pixStampPoint._x =(double)stamperPointX;
	pixStampPoint._y =(double)stanperPointY;



	////求像素距离
	double dXFactorDis=  algorithm::math::TwoPointsDistance(pixtopLeft._x,pixtopLeft._y,pixleftButtom._x,pixleftButtom._y);
	double dYFactorDis=  algorithm::math::TwoPointsDistance(pixrightButtom._x,pixrightButtom._y,pixleftButtom._x,pixleftButtom._y);
	double dXDis =  algorithm::math::TwoPointsDistance(wtopLeft._x,wtopLeft._y,wleftButtom._x,wleftButtom._y);;
	double dYDis =   algorithm::math::TwoPointsDistance(wrightButtom._x,wrightButtom._y,wleftButtom._x,wleftButtom._y);;
	double d1 =  dXDis/dXFactorDis;
	double d2 =  dYDis/dYFactorDis;

	//像素距离 
	double dpixLength = algorithm::math::TwoPointsDistance(pixleftButtom._x,pixleftButtom._y,pixStampPoint._x,pixStampPoint._y);

	double dMLength = (d1+d2)/2 *dpixLength;


	//buttomtop向量
	svector svecbt;
	svecbt._x = pixStampPoint._x - pixleftButtom._x;
	svecbt._y = pixStampPoint._y - pixleftButtom._y;




	svector svecblr;
	svecblr._x =  pixtopLeft._x - pixleftButtom._x;
	svecblr._y =  pixtopLeft._y - pixleftButtom._y;

	double angle1 = algorithm::math::CalculateVectorAngle(svecbt,svecblr);

	svector svecbtl;

	svecbtl._x =  pixrightButtom._x - pixleftButtom._x;
	svecbtl._y =  pixrightButtom._y - pixleftButtom._y;

	double angle2 = algorithm::math::CalculateVectorAngle(svecbt,svecbtl);

	//平移向量
	svector sPYvector;
	sPYvector._x = 0 - wleftButtom._x ;
	sPYvector._y = 0- wleftButtom._y ;



	svector aVector;
	svector bVector;


	aVector._x = wtopLeft._x+ sPYvector._x;
	aVector._y = wtopLeft._y+ sPYvector._y;

	bVector._x =  wrightButtom._x+sPYvector._x;
	bVector._y  = wrightButtom._y + sPYvector._y;

	svector posv;
	algorithm::math::CalculatePosition(aVector,bVector,dMLength,angle1,angle2,posv);

	//平移
	posv._x = posv._x- sPYvector._x;
	posv._y = posv._y- sPYvector._y;

	//去整值
	unsigned short  posx1= static_cast<unsigned short>(algorithm::convert::round(posv._x));
	unsigned short  posy1 = static_cast<unsigned short>(algorithm::convert::round(posv._y));
	char * pOrigPoints = points;
	memset(points,0,len);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(posx1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(posy1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));

	points =  pOrigPoints;

	return  0 ;
}

int CContrlledStamper::CalculateStampePoints()
{
//	szStamper[0][]={};

	return  -1;

}

int CContrlledStamper::CalibrationMutiple(char * points,int len)
{


	vector<point>  vecStamper;

	//测试
	memset(points,0,len);
	BYTE szPoint[2];
	//获取印章ID
	BYTE str[256] = {0};
	BYTE szID[4] = {0};
	BYTE szSelectID[4] ={0};
	int posStart ; 
	int posEnd ;
	int  ret = -1;
	char data[8]={0};
	unsigned short _x1 =0 ;
	unsigned short _y1 =0 ;

	unsigned short _x2 =0 ;
	unsigned short _y2 =0 ;

	unsigned short _x3 =0 ;
	unsigned short _y3 =0 ;
	unsigned short _x4 =0 ;
	unsigned short _y4 =0 ;
	unsigned short _x5 =0 ;
	unsigned short _y5 =0 ;

	unsigned short _x6 =0 ;
	unsigned short _y6 =0 ;

	unsigned short _x7 =0 ;
	unsigned short _y7 =0 ;
	unsigned short _x8 =0 ;
	unsigned short _y8 =0 ;

	unsigned short _x9 =0 ;
	unsigned short _y9 =0 ;

	unsigned short _x10 =0 ;
	unsigned short _y10 =0 ;

	unsigned short _x11 =0 ;
	unsigned short _y11 =0 ;

	unsigned short _x12 =0 ;
	unsigned short _y12 =0 ;

	unsigned short _x13 =0 ;
	unsigned short _y13 =0 ;

	unsigned short _x14 =0 ;
	unsigned short _y14 =0 ;

	unsigned short _x15 =0 ;
	unsigned short _y15 =0 ;

	unsigned short _x16 =0 ;
	unsigned short _y16 =0 ;

	unsigned short _x17 =0 ;
	unsigned short _y17 =0 ;

	unsigned short _x18 =0 ;
	unsigned short _y18 =0 ;

	unsigned short _x19 =0 ;
	unsigned short _y19 =0 ;

	unsigned short _x20 =0 ;
	unsigned short _y20 =0 ;


	unsigned short _x21 =0 ;
	unsigned short _y21 =0 ;

	unsigned short _x22 =0 ;
	unsigned short _y22=0 ;

	unsigned short _x23 =0 ;
	unsigned short _y23 =0 ;

	unsigned short _x24 =0 ;
	unsigned short _y24 =0 ;

	
	unsigned short _x25=0 ;
	unsigned short _y25 =0 ;

	_x1 =(unsigned short)  SEAL_TABLE_X_MIN;
	_y1 =(unsigned short)  SEAL_TABLE_Y_MIN;
	_x2 =(unsigned short)  SEAL_TABLE_X_MAX;
	_y2 =(unsigned short)  SEAL_TABLE_Y_MAX;

	_x3 =(unsigned short)SEAL_TABLE_X_MIN ;
	_y3 =(unsigned short) SEAL_TABLE_Y_MAX;
	_x4 =(unsigned short) SEAL_TABLE_X_MAX;
	_y4 = (unsigned short) SEAL_TABLE_Y_MIN;

	_x5 =(unsigned short)( SEAL_TABLE_X_MAX+SEAL_TABLE_X_MIN)/2;
	_y5 = (unsigned short) (SEAL_TABLE_Y_MIN+SEAL_TABLE_Y_MAX)/2;




	_x6 =_x1 ;
	_y6 =_y1 ;

	_x7 =_x5 ;
	_y7 =_y5 ;
	_x8 = (_x3+ _x1)/2;
	_y8 = (_y3+ _y1)/2 ;

	_x9 =(_x1+ _x4)/2;
	_y9 =(_y1+ _y4)/2;

	_x10 =(_x1 +_x5)/2;
	_y10 =(_y1 +_y5)/2 ;

	_x11 =(_x4+ _x1)/2;
	_y11 =(_y4+ _y1)/2 ;

	_x12 =(_x4+ _x2)/2;
	_y12 =(_y4+ _y2)/2;

	_x13 =_x5 ;
	_y13 =_y5 ;

	_x14 =_x4 ;
	_y14 =_y4 ;

	_x15 =(_x4+ _x5)/2;;
	_y15 =(_y4+ _y5)/2;

	_x16 =_x5 ;
	_y16 =_y5 ;

	_x17 =_x2 ;
	_y17 =_y2 ;

	_x18 =(_x2+ _x3)/2 ;
	_y18 =(_y2+ _y3)/2;

	_x19 =(_x2+ _x3)/2 ;
	_y19 =(_y2+ _y3)/2 ;

	_x20 =(_x2+ _x5)/2 ; ;
	_y20 =(_y2+ _y5)/2 ; ;


	_x21 =(_x1+ _x3)/2 ;
	_y21 =(_y1+ _y3)/2;

	_x22 =(_x2+ _x3)/2 ;
	_y22=(_y2+ _y3)/2 ;

	_x23 =_x3 ;
	_y23 =_y3 ;

	_x24 =_x5 ;
	_y24 =_y5 ;


	_x25=(_x5+ _x3)/2 ;
	_y25 =(_y5+ _y3)/2 ;



	vecStamper.push_back(point(_x1,_y1));

	vecStamper.push_back(point(_x2,_y2));
	vecStamper.push_back(point(_x3,_y3));
	vecStamper.push_back(point(_x4,_y4));
	vecStamper.push_back(point(_x5,_y5));
	vecStamper.push_back(point(_x6,_y6));

	vecStamper.push_back(point(_x7,_y7));
	vecStamper.push_back(point(_x8,_y8));
	vecStamper.push_back(point(_x9,_y9));

	vecStamper.push_back(point(_x10,_y10));
	vecStamper.push_back(point(_x11,_y11));
	vecStamper.push_back(point(_x12,_y12));
	vecStamper.push_back(point(_x13,_y13));
	vecStamper.push_back(point(_x14,_y14));
	vecStamper.push_back(point(_x15,_y15));
	vecStamper.push_back(point(_x16,_y16));
	vecStamper.push_back(point(_x17,_y17));
	vecStamper.push_back(point(_x18,_y18));
	vecStamper.push_back(point(_x19,_y19));
	vecStamper.push_back(point(_x20,_y20));
	vecStamper.push_back(point(_x21,_y21));
	vecStamper.push_back(point(_x22,_y22));
	vecStamper.push_back(point(_x23,_y23));
	vecStamper.push_back(point(_x24,_y24));
	vecStamper.push_back(point(_x25,_y25));
	


	char * pOrigPoints = points;
	if(len>15)
	{
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);
	
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x3,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y3,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x4,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y4,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);
		ASSIGMENT(_x6,_y6)
		ASSIGMENT(_x7,_y7)
		ASSIGMENT(_x8,_y8)
		ASSIGMENT(_x9,_y9)
		ASSIGMENT(_x10,_y10)
		ASSIGMENT(_x11,_y11)
		ASSIGMENT(_x12,_y12)
		ASSIGMENT(_x13,_y13)
		ASSIGMENT(_x13,_y13)
		ASSIGMENT(_x14,_y14)
		ASSIGMENT(_x15,_y15)
		ASSIGMENT(_x16,_y16)
		ASSIGMENT(_x17,_y17)
		ASSIGMENT(_x18,_y18)
		ASSIGMENT(_x19,_y19)
		ASSIGMENT(_x20,_y20)
		ASSIGMENT(_x21,_y21)
		ASSIGMENT(_x22,_y22)
		ASSIGMENT(_x23,_y23)
		ASSIGMENT(_x24,_y24)
		ASSIGMENT(_x25,_y25)
	}
	points = pOrigPoints;
	ret =::FGetSealPresent((char*)str, sizeof(str));
	for(unsigned int i  = 0 ; i <6 ; i++)
	{
		posStart = 1;
		posEnd = 5;
		memset(szID,0,4);
		posStart += i*4 ;
		posEnd += i*4;
		memcpy(szID,(void*)&str[posStart],4);
		if(szID[0]&0x01||szID[1]&0x01||szID[2]&0x01||szID[3]&0x01)
		{
			memcpy(szSelectID,szID,sizeof(szSelectID));
			break;
		}

	}

	 //选择第一个被寻找到的印章
	FStartStamper(0, (char*)szID, 1, _x1,_y1,0,15);
	DWORD dwTimeOut = 15000;
	DWORD dTimeOut = 0;
	DWORD dStartTime = GetTickCount();
	size_t icount = 1;
	while(1)
	{
		Sleep(100);

		
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			if(icount<vecStamper.size())
			{
			FStartStamper(0, (char*)szID, 1, vecStamper.at(icount)._x, vecStamper.at(icount)._y,0,15);
			icount++;
			}
			else
			{
				break;
			}
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		
	}
	/*Sleep(15000);

	FStartStamper((char*)szID, 1, _x3, _y3,0,15);
	Sleep(15000);
	FStartStamper((char*)szID, 1, _x4, _y4,0,15);

	Sleep(15000);
	FStartStamper((char*)szID, 1, _x5, _y5,0,15);*/

	/*while(1)
	{
		Sleep(100);
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			FStartStamper((char*)szID, 1, _x3, _y3,0,15);
			break;
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		if (dTimeOut> dwTimeOut)
		{
			break;
		}
	}

	while(1)
	{
		Sleep(100);
		if(m_USB->GetStamperStauts()==2)
		{
			m_USB->SetStamperStauts(0);
			FStartStamper((char*)szID, 1, _x4, _y4,0,15);
			break;
		}
		DWORD	dEndTime = GetTickCount();
		DWORD  dTimeOut = dEndTime - dStartTime;


		if (dTimeOut> dwTimeOut)
		{
			break;
		}
	}*/

	return  0;
}

int CContrlledStamper::CalibrationEx(char * pStampid, char* points, int len)
{
	//保存印章ID
	char szStampID[4] = { 0 };
	memset(points, 0, len);
	if(pStampid != 0)
	{
		memcpy(szStampID,pStampid,sizeof(szStampID));
	}

	vector<point>  vecStampPoints;
	//测试
	BYTE szPoint[2];
	int  ret = -1;
	char data[8]={0};
	DWORD	dStartTime =20000;
	DWORD	dwTimeOut= 200000;
	unsigned short _x1 =0 ;
	unsigned short _y1 =0 ;
	unsigned short _x2 =0 ;
	unsigned short _y2 =0 ;

	unsigned short _x3 =0 ;
	unsigned short _y3 =0 ;
	unsigned short _x4 =0 ;
	unsigned short _y4 =0 ;
	unsigned short _x5 =0 ;
	unsigned short _y5 =0 ;

	_x1 =(unsigned short)SEAL_TABLE_X_MIN;
	_y1 =(unsigned short)SEAL_TABLE_Y_MIN;
	_x2 =(unsigned short)SEAL_TABLE_X_MAX;
	_y2 =(unsigned short)SEAL_TABLE_Y_MAX;

	_x3 =(unsigned short)SEAL_TABLE_X_MIN ;
	_y3 =(unsigned short)SEAL_TABLE_Y_MAX;
	_x4 =(unsigned short)SEAL_TABLE_X_MAX;
	_y4 = (unsigned short)SEAL_TABLE_Y_MIN;

	_x5 =(unsigned short)( SEAL_TABLE_X_MAX+SEAL_TABLE_X_MIN)/2;
	_y5 = (unsigned short) (SEAL_TABLE_Y_MIN+SEAL_TABLE_Y_MAX)/2;

	char * pOrigPoints = points;
	if(len>12)
	{
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);
	
	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y1,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y2,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_x5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	points +=sizeof(szPoint);

	memset(szPoint,0,sizeof(szPoint));
	algorithm::convert::U16ToLeBuf(_y5,szPoint,0);
	memcpy(points,szPoint,sizeof(szPoint));
	}
	points = pOrigPoints;

	//存放盖章点
	vecStampPoints.push_back(point(_x1,_y1));
	vecStampPoints.push_back(point(_x1,_y1));
	vecStampPoints.push_back(point(_x5,_y5));
	vecStampPoints.push_back(point(_x5,_y5));
	vecStampPoints.push_back(point(_x2,_y2));
	vecStampPoints.push_back(point(_x2,_y2));

    size_t iCount = 0;
    dStartTime = GetTickCount();
    while (1)
    {
        if (iCount > (vecStampPoints.size() - 1))
        {
            break;
        }
        if (iCount == 0)
        {
            FStartStamper(GetTickCount(), (char*)szStampID, 1, vecStampPoints.at(0)._x, vecStampPoints.at(0)._y, 0, 2);
            iCount++;
        }

        if (m_USB->GetStamperStauts() == 2)
        {
            m_USB->SetStamperStauts(0);
            dStartTime = GetTickCount();
            FStartStamper(GetTickCount(), (char*)szStampID, 1, vecStampPoints.at(iCount)._x, vecStampPoints.at(iCount)._y, 0, 2);
            iCount++;
        }
        DWORD	dEndTime = GetTickCount();
        DWORD  dTimeOut = dEndTime - dStartTime;

        if (dTimeOut > dwTimeOut)
        {
            break;
        }

        Sleep(100);
    }

    if (iCount == 6)
    {
        ret = 0;
    }
    return ret;
}

int CContrlledStamper::SetAlarm(char alarm,char operation)
{
	char recvdata = 0;	
	int ret = 0;
	unsigned char params[2] = {0};
	params[0] = alarm;
	params[1] = operation;
	ret = SendCommand(CMD_ALARMPD, sizeof(params), params, &recvdata, sizeof(recvdata));
	if (ret==STF_SUCCESS)
	{
		ret = recvdata;
	}

	return ret;	
}


  



int CContrlledStamper::SetStampMap(unsigned char* mapin,int len)
{
	char recvdata = 0;	
	int ret = 0;
// 	unsigned char params[24] = {0};
// 	if(len ==24)
// 	{
// 		memcpy(params,mapin,sizeof(24));
// 
// 	}

	ret = SendCommand(CMD_SETMAPPING, len, mapin, &recvdata, sizeof(recvdata));
    if (STF_SUCCESS != ret) {
        CLog::sharedLog()->WriteNormalLog("SetStampMap fails, err:%d", ret);
        return ret;
    }

	ret = recvdata;
	return ret;	
}

//获取印章映射表
int  CContrlledStamper::GetStampMap(char* mapout,int len)
{
	return SendCommand(CMD_READMAPPINT, 0, NULL, mapout, len);
}

//读取印章有效范围
int CContrlledStamper::GetPhsicalRange(char * phsicalOut,int len)
{
    return SendCommand(CMD_STAMPRANGE, 0, NULL, phsicalOut, len);
}


int CContrlledStamper::GetStorageCapacity(unsigned char& version, unsigned short& mem_size)
{
    char recvdata[3] = { 0 };
    int ret = SendCommand(
        CMD_GETSTORECPACITY,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    version = recvdata[0];
    mem_size = (unsigned short)recvdata[1] | (recvdata[2] << 8);
    return 0;
}

int CContrlledStamper::WriteIntoStamper(
    unsigned short offset, 
    const unsigned char* data, 
    unsigned short len)
{
    if (offset < 0 || offset + len > MAX_STAMPER_MEM)
        return STF_INVALID_PARAMETER;

    char recvdata = 0;
    unsigned char params[1024] = { 0 };
    params[0] = (unsigned char)offset;
    params[1] = (unsigned char)(offset >> 8);
    params[2] = (unsigned char)len;
    params[3] = (unsigned char)(len >> 8);
    memcpy(params + 4, data, len);

    int ret = SendCommand(
        CMD_STORE_DATA,
        len + 4,
        params,
        &recvdata,
        sizeof(recvdata));
    if (ret == STF_SUCCESS)
        ret = recvdata;
    else
        CLog::sharedLog()->WriteNormalLog("WriteIntoStamper fails->err:%d", ret);

    return ret;
}

int CContrlledStamper::ReadStamperMem(
    unsigned short offset, 
    unsigned short size,
    unsigned char* data, 
    unsigned char& len)
{
    if (NULL == data || offset + size > MAX_STAMPER_MEM)
        return STF_INVALID_PARAMETER;

    unsigned char recvdata[1024] = { 0 };
    unsigned char params[4] = { 0 };
    params[0] = (unsigned char)offset;
    params[1] = (unsigned char)(offset >> 8);
    params[2] = (unsigned char)size;
    params[3] = (unsigned char)(size >> 8);
    int ret = SendCommand(
        CMD_READ_DATA,
        sizeof(params),
        params,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("ReadStamperMem fails->err:%d", ret);
        return ret;
    }

    unsigned short returned_offset = (unsigned short)recvdata[0] | (recvdata[1] << 8);
    unsigned short returned_size = (unsigned short)recvdata[2] | (recvdata[3] << 8);
    if (0 == returned_size) //本包读取失败
        return STF_INVALID_PARAMETER;

    len = (unsigned char)returned_size;
    for (int i = 0; i < returned_size; ++i) {
        data[i] = recvdata[4 + i];
    }

    return 0;
}

int CContrlledStamper::SelectStamper(unsigned char stamper)
{
    char recvdata = 0;
    int ret = SendCommand(
        CMD_STAMPERNO,
        1,
        &stamper,
        &recvdata,
        sizeof(recvdata));
    if (ret == STF_SUCCESS)
        ret = recvdata;
    else
        CLog::sharedLog()->WriteNormalLog("SelectStamper fails->err:%d", ret);

    return ret;
}

int CContrlledStamper::GetStamperID(unsigned char stamper, unsigned int& rfid)
{
    if (stamper > 6)
        return STF_INVALID_PARAMETER;

    unsigned char recvdata[6] = { 0 };
    int ret = SendCommand(
        CMD_SPECIFYSTAMPERNO,
        1,
        &stamper,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    //校验请求和回应是否是同一个印章
    unsigned char returned_stamper = recvdata[1];
    if (stamper != returned_stamper)
        return STF_ERR_RECVFAIL;

    ret = recvdata[0];
    rfid = (unsigned int)recvdata[2] | (recvdata[3] << 8) | (recvdata[4] << 16) | (recvdata[5] << 24);
    return ret;
}

int CContrlledStamper::GetStamper(unsigned int rfid, unsigned char& stamper)
{
    std::map<unsigned int, int> stampers;
    unsigned int id = 0;
    for (int i = 0; i < 6; ++i) {
        if (0 == GetStamperID(i, id))
            stampers.insert(std::make_pair(id, i));
    }

    std::map<unsigned int, int>::iterator it = stampers.find(rfid);
    if (it == stampers.end())
        return -1;

    stamper = it->second + 1;
    return STF_SUCCESS;
}

int CContrlledStamper::EnterAuthMode()
{
    char recvdata = 0;
    int ret = SendCommand(
        CMD_ENTERAUTH,
        0,
        NULL,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::ExitAuthMode()
{
    char recvdata = 0;
    int ret = SendCommand(
        CMD_QUITAUTH,
        0,
        NULL,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::EnableFactory(unsigned char enable)
{
    char recvdata = 0;
    int ret = SendCommand(
        CMD_SET_FACTORY_TEST,
        1,
        &enable,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::VerifyKey(char key_type, const unsigned char* key, unsigned char len)
{
    unsigned char params[7] = { 0 };
    params[0] = key_type;
    memcpy(params + 1, key, len >= 6? 6: len);

    char recvdata = 0;
    int ret = SendCommand(
        CMD_VERIFYKEY,
        sizeof(params),
        params,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::OperateBlock(unsigned char block)
{
    char recvdata = 0;
    int ret = SendCommand(
        CMD_OPERATEBLOCK,
        1,
        &block,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::ReadBlock(unsigned char block, char* data, unsigned char len)
{
    char recvdata[17] = { 0 };
    int ret = SendCommand(
        CMD_READBLOCK,
        sizeof(block),
        &block,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata[0];
    //读取数据成功
    if (ret == 0)
        memcpy(data, &recvdata[1], len >= 16 ? 16 : len);

    return ret;
}

int CContrlledStamper::WriteBlock(unsigned char block, char* data, unsigned char len)
{
    unsigned char params[18] = { 0 };
    params[0] = block;
    memcpy(params + 1, data, len >= 17? 17: len);
    char recvdata = 0;
    int ret = SendCommand(
        CMD_WRITEBLOCK,
        sizeof(params),
        params,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS)
        return ret;

    ret = recvdata;
    return ret;
}

int CContrlledStamper::ReadAllRFID(unsigned int* rfids, unsigned char len, unsigned char* stampers)
{
    len = len >= 7 ? 7 : len;
    unsigned char recvdata[30] = { 0 };
    int ret = SendCommand(
        CMD_ALL_RFID,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ReadAllRFID->err:%d", ret);
        return ret;
    }

    ret = recvdata[0];
    *stampers = recvdata[1];
    for (int i = 0; i < len; ++i) {
        memcpy(&rfids[i], (recvdata + 2) + i * 4, 4);
    }

    return ret;
}

int CContrlledStamper::ReadAlarmVoltage(unsigned char* voltage)
{
    unsigned char recvdata[2] = { 0 };
    int ret = SendCommand(
        CMD_ALARM_VOLTAGE,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ReadAlarmVoltage->err:%d", ret);
        return ret;
    }

    ret = recvdata[0];
    if (0 == ret)
        *voltage = recvdata[1];

    return ret;
}

int CContrlledStamper::ClearStatistic()
{
    unsigned char recvdata = 0;;
    int ret = SendCommand(
        CMD_CLEAR_STATISTIC,
        0,
        NULL,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ClearStatistic->err:%d", ret);
        return ret;
    }
    
    ret = recvdata;
    return ret;
}

int CContrlledStamper::Reset()
{
    unsigned char recvdata = 0;;
    int ret = SendCommand(
        CMD_RESET,
        0,
        NULL,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::Reset->err:%d", ret);
        return ret;
    }

    ret = recvdata;
    return ret;
}

int CContrlledStamper::Confirm(
    unsigned int serial,
    unsigned int rfid,
    char isPadInk,
    short x_point,
    short y_point,
    short angle,
    short w_time,
    unsigned char type)
{
    char recvdata = 0;
    unsigned char params[18] = { 0 };
    memcpy(params, &serial, sizeof(serial));
    memcpy(params + 4, &rfid, sizeof(rfid));

    const unsigned char begin = 7;
    params[begin + 1] = isPadInk;
    params[begin + 2] = (unsigned char)x_point;
    params[begin + 3] = (unsigned char)(x_point >> 8);
    params[begin + 4] = (unsigned char)y_point;
    params[begin + 5] = (unsigned char)(y_point >> 8);
    params[begin + 6] = (unsigned char)angle;
    params[begin + 7] = (unsigned char)(angle >> 8);
    params[begin + 8] = (unsigned char)w_time;
    params[begin + 9] = w_time >> 8;
    params[begin + 10] = type;

    int ret = SendCommand(CMD_CONFIRM_STAMP, sizeof(params), params, &recvdata, sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::Confirm->err:%d", ret);
        return ret;
    }

    ret = recvdata;
    return ret;
}

int CContrlledStamper::ReadStamp(unsigned char* info, unsigned char len)
{
    char recvdata[47] = { 0 };
    int ret = SendCommand(
        CMD_READ_STAMPING,
        0,
        NULL,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ReadStamp->err:%d", ret);
        return ret;
    }

    ret = recvdata[0];
    return ret;
}

int CContrlledStamper::SetSideDoor(unsigned short keep_open, unsigned short timeout)
{
    unsigned char recvdata = 0;
    unsigned char params[4] = { 0 };
    memcpy(params, &keep_open, sizeof(keep_open));
    memcpy(params + 2, &timeout, sizeof(timeout));
    int ret = SendCommand(
        CMD_SET_SIDEDOOR_INFO,
        sizeof(params),
        params,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::SetSideDoor->err:%d", ret);
        return ret;
    }

    ret = recvdata;
    return ret;
}

int CContrlledStamper::SetPaperDoor(unsigned short timeout)
{
    unsigned char recvdata = 0;
    unsigned char params[2] = { 0 };
    memcpy(params, &timeout, sizeof(timeout));
    int ret = SendCommand(
        CMD_PAPER_DOOR_INFO,
        sizeof(params),
        params,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::SetPaperDoor->err:%d", ret);
        return ret;
    }

    ret = recvdata;
    return ret;
}

int CContrlledStamper::ReadAlarmStatus(char* door, char* vibration)
{
    char recvdata[3] = { 0 };
    int ret = SendCommand(
        CMD_ALARM_STATUS,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ReadAlarmStatus->err:%d", ret);
        return ret;
    }

    if ((ret = recvdata[0]) == 0) {
        *door = recvdata[1];
        *vibration = recvdata[2];
    }

    return ret;
}

int CContrlledStamper::GetHardwareVer(char* version, unsigned char len)
{
    if (NULL == version || 0 == len)
        return STF_INVALID_PARAMETER;

    char recvdata[50] = { 0 };
    int ret = SendCommand(
        CMD_HARDWARE_VERSION,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetHardwareVer->err:%d", ret);
        return ret;
    }

    if ((ret = recvdata[0]) == 0) {
        unsigned char real_len = recvdata[1];
        if (real_len > len) {
            CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetHardwareVer->下位机返回字符串"
                "长度大于传入参数");
            return STF_INVALID_PARAMETER;
        }

        memcpy(version, &recvdata[2], real_len);
    }

    return ret;
}

int CContrlledStamper::GetABCStamper(unsigned char stamper, char* id, unsigned char len)
{
    if (stamper < 1 || stamper > 6 || NULL == id || 0 == len)
        return STF_INVALID_PARAMETER;

    memset(id, 0, len);
    std::string abc_id = GetStamperABC(stamper);
    if (abc_id.empty())
        return 1;
    
    if (0 == strcmp(abc_id.c_str() + 4, "00000000"))
        return 2;

    memcpy(id, abc_id.c_str(), abc_id.length() > len? len: abc_id.length());
    return 0;
}

int CContrlledStamper::GetRFID(const std::string& abc_id, unsigned int& rfid)
{
    std::map<std::string, unsigned int>::iterator it = abc_rfid_.find(abc_id);
    if (it == abc_rfid_.end())
        return -1;

    rfid = it->second;
    return 0;
}

int CContrlledStamper::GetABCIndex(const std::string& abc_id, char& index)
{
    int ret = -1;
    std::map<int, std::string>::iterator it = stamper_abc_.begin();
    for (; it != stamper_abc_.end(); ++it) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetABCIndex->%s vs %s", it->second.c_str(),abc_id.c_str());
        if (it->second.compare(abc_id) == 0) {
            ret = 0;
            index = it->first;
        }
    }

    return ret;
}

std::string CContrlledStamper::GetStamperABC(unsigned char index)
{
    //1. 卡选择(SelectStamper),
    //2. 卡请求(GetStamperID),
    //3. 设置地址(OperateBlock),
    //4. 密码比较(VerifyKey),
    //5. 卡读写(WriteBlock, ReadBlock)
    int ret = SelectStamper(index - 1);
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->卡选择失败, err:%d", ret);
        return std::string();
    }

    unsigned int rfid = 0;
    ret = GetStamperID(index - 1, rfid);
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->卡请求失败, err:%d", ret);
        return std::string();
    }

    const unsigned char ABC_BLOCK = 1;
    ret = OperateBlock(ABC_BLOCK);
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->设置卡操作绝对地址失败, "
            "err:%d", ret);
        return std::string();
    }

    const unsigned char key[] = { 255, 255, 255, 255, 255, 255};
    ret = VerifyKey('A', key, 6);
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->卡密码校验失败, err:%d", ret);
        return std::string();
    }

    char block_data[4] = { 0 }; //读块前四字节
    ret = ReadBlock(ABC_BLOCK, block_data, sizeof(block_data));
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->读卡失败, err:%d", ret);
        return std::string();
    }
    
    std::map<int, std::string>::iterator it = stamper_abc_.find(index);
    if (it != stamper_abc_.end())
        stamper_abc_.erase(it);

    int sum = 0;//strtol(block_data, NULL, 16);
	memcpy((char*)&sum,block_data,sizeof(int));
    CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->读块成功, 值为: %d", sum);
    if (sum < 0)
        sum = ~sum;

    char buf[9] = { 0 };
    sprintf_s(buf, "%d", sum);
    char abc_des[] = { '0', '0', '0', '0', '0', '0', '0', '0', '0' };
    strcpy(abc_des + 8 - strlen(buf), buf);

    std::string abc_id = "STDZ" + std::string(abc_des);
    if (0 != sum) {
        //农行电子标签
        stamper_abc_.insert(std::make_pair(index, abc_id));
        abc_rfid_.insert(std::make_pair(abc_id, rfid));
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::GetStamperABC->成功获取农行电子标签"
            "(stamper, id) = (%d, %s)", index, abc_id.c_str());
    }

    return abc_id;
}

int CContrlledStamper::ReadBackupSN(unsigned char* sn, unsigned char len)
{
    if (NULL == sn || len < 48)
        return STF_INVALID_PARAMETER;

    unsigned char recvdata[51] = { 0 };
    int ret = SendCommand(
        CMD_READ_BACKUP_SN,
        0,
        NULL,
        recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::ReadBackupSN->err:%d", ret);
        return ret;
    }

    if ((ret = recvdata[0]) == 0)
        memcpy(sn, recvdata + 1, 48);

    return ret;
}

int CContrlledStamper::WriteBackupSN(const unsigned char* sn, unsigned char len)
{
    if (NULL == sn || len == 0 || len > 48)
        return STF_INVALID_PARAMETER;

    char params[49] = { 0 };
    memcpy(params, sn, 48);
    int sum = 0;
    for (int i = 0; i < 48; ++i)
        sum += sn[i];
    sum = ~sum;
    params[48] = sum;

    char recvdata = 0;
    int ret = SendCommand(
        CMD_WRITE_BACKUP_SN,
        sizeof(params),
        (unsigned char*)params,
        &recvdata,
        sizeof(recvdata));
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("CContrlledStamper::WriteBackupSN->err:%d", ret);
        return ret;
    }

    ret = recvdata;
    return ret;
}
