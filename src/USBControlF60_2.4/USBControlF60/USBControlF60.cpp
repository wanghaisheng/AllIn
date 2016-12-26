// USBControlF60.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "USBControlF60.h"
#include "CtrlUSB.h"
#include "ContrlledStamper.h"
#include "Log.h"
#include <string.h>
#include "WLock.h"

//EEPROM地址空间分配
const unsigned short STEP = 2;

//MAC地址
const unsigned short MAC1_OFFSET = 0;
const unsigned short MAC2_OFFSET = 20;
const unsigned short MAX_MAC = 17;

//KEY地址
const unsigned short KEY_OFFSET = MAC2_OFFSET + MAX_MAC + STEP;
const unsigned short KEY_LEN = 16;

//校准点数据地址
const unsigned short CALIBRATION_OFFSET = KEY_OFFSET + KEY_LEN + STEP;
const unsigned short CALIBRATION_LEN = 20;

//图像转换倍率地址
const unsigned short IMAGE_RATION_OFFSET = CALIBRATION_OFFSET + CALIBRATION_LEN + STEP;
const unsigned short IMAGE_RATION_LEN = 16;

//锁标记地址
const unsigned short LOCK_OFFSET = IMAGE_RATION_OFFSET + IMAGE_RATION_LEN + STEP;
const unsigned short LOCK_LEN = 1;

//序列号地址
const unsigned short SN_OFFSET = LOCK_OFFSET + LOCK_LEN + STEP;
const unsigned short SN_LEN = 14;

//印控仪编号地址
const unsigned short ID_OFFSET = SN_OFFSET + SN_LEN + STEP;
const unsigned short ID_LEN = 22;

//设备认证码地址
const unsigned short DEV_CODE_OFFSET = ID_OFFSET + ID_LEN + STEP;
const unsigned short DEV_CODE_LEN = 10;

#define  INTERFACE_LOG(inf_name)  CLog* plog = CLog::sharedLog();\
	SYSTEMTIME sys;   \
    ::GetLocalTime( &sys );\
	plog->WriteNormalLog(#inf_name) ;\


// 这是导出变量的一个示例
USBCONTROLF60_API int nUSBControlF60=0;

// 这是导出函数的一个示例。
USBCONTROLF60_API int fnUSBControlF60(void)
{
	return 42;
}


int F_RegisterDevCallBack(pDevConnectCallBack func)
{
	CCtrlUSB *p = CCtrlUSB::sharedCCtrlUSB();
	p->RegisterDevCallBack(func);
	return STF_SUCCESS;
}

void F_RecordLog(int level, const char* szLog)
{
	CLog::sharedLog()->WriteLog(level, szLog);
}

 
// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 USBControlF60.h
CUSBControlF60::CUSBControlF60()
{
 	/*usbctrl = new CContrlledStamper;	*/
	return;
}

CUSBControlF60::~CUSBControlF60(void)
{
// 	if (usbctrl)
// 	{
// 		usbctrl->CloseDev();
// 		delete usbctrl;
// 		usbctrl = NULL;
// 	}
// 
// 	return;
}





static CContrlledStamper gusbctrl;// = new CContrlledStamper;

int FRegisterDevCallBack(pfnDevMsgCallBack pfn)
{
	gusbctrl.RegisterDevCallBack(pfn);
	return STF_SUCCESS;
}
int FOpenDev(const char *file)//参数无效
{
#ifdef _LOG
	INTERFACE_LOG(FOpenDev);
#endif
	CCtrlUSB *p = CCtrlUSB::sharedCCtrlUSB();

	string devfile = p->GetDevPath();

	int ret = -1;
	//if (gusbctrl)
	{		
		ret = gusbctrl.OpenDev(devfile.c_str());
	}
	return ret;
}
int FCloseDev(void)
{
#ifdef _LOG
	INTERFACE_LOG(FCloseDev);
#endif

	int ret = -1;
	ret = gusbctrl.CloseDev();
	return ret;
}

int FQuitMaintainMode(void)
{

#ifdef _LOG
	INTERFACE_LOG(FReset);
#endif

	int ret = -1;
	ret = gusbctrl.QuitMaintainMode();
	return ret;
}
int FRestart(void)
{
#ifdef _LOG
	INTERFACE_LOG(FRestart);
#endif
	int ret = -1;
	ret = gusbctrl.Restart();
	return ret;
}

int FGetSystemMsg(char *pRtn, int nLen)
{
#ifdef _LOG
	INTERFACE_LOG(FGetSystemMsg);
#endif
	int ret = -1;
	ret = gusbctrl.GetSystemMsg(pRtn,nLen);
	return ret;
}
int FGetFirwareVer(OUT char* strVer, int len)
{
#ifdef _LOG
	INTERFACE_LOG(_LOG);
#endif
	int ret = -1;
	ret = gusbctrl.GetFirwareVer(strVer, len);
	return ret;
}

int FSetFirwareSN(char* strSN, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FSetFirwareSN);
#endif

    if (NULL == strSN)
        return STF_INVALID_PARAMETER;

    unsigned char* sn_addr = new (std::nothrow) unsigned char[SN_LEN + 1];
    if (NULL == sn_addr)
        return STF_ALLOCATE_MEM_FAIL;

    memset(sn_addr, 0, SN_LEN + 1);
    unsigned char key_len = len >= SN_LEN ? SN_LEN : len;
    memcpy(sn_addr, strSN, key_len);
    sn_addr[SN_LEN] = '\0';
    int ret = WriteIntoStamper(
        SN_OFFSET,
        sn_addr,
        key_len);
    delete[] sn_addr;
    
    return ret;
}

int FGetFirwareSN(OUT char* strSN, int key_len)
{
#ifdef _LOG
	INTERFACE_LOG(FGetFirwareSN);
#endif

    if (NULL == strSN)
        return STF_INVALID_PARAMETER;

    unsigned char len = key_len >= SN_LEN ? SN_LEN : key_len;
    unsigned char ret_len = 0;
    int ret = ReadStamperMem(SN_OFFSET, len, (unsigned char*)strSN, ret_len);
    if (STF_SUCCESS == ret)
        strSN[len] = '\0';

    return ret;
}

int FGetDevStatus(OUT unsigned char* strStatus, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FGetDevStatus);
#endif
    int ret = -1;
    ret = gusbctrl.GetDevStatus(strStatus, len);
    return ret;
}

int FMaintenanceMode(void)
{
#ifdef _LOG
	INTERFACE_LOG(FMaintenanceMode);
#endif
	int ret = -1;
	ret = gusbctrl.MaintenanceMode();
	return ret;
}
int FGetInfraRedStatus(OUT char* strInfraRed, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FGetInfraRedStatus);
#endif
	int ret = -1;
	ret = gusbctrl.GetInfraRedStatus(strInfraRed, len);
	return ret;
}
int FGetSealPresent(OUT char* strSealStatus, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FGetSealPresent);
#endif
	int ret = -1;
	ret = gusbctrl.GetSealPresent(strSealStatus, len);
	return ret;
}
int FGetDoorsPresent(OUT char* strDoorsStatus, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FGetDoorsPresent);
#endif
	int ret = -1;
	ret = gusbctrl.GetDoorsPresent(strDoorsStatus, len);
	return ret;
}
int FOpenDoorPaper()
{
#ifdef _LOG
	INTERFACE_LOG(FOpenDoorPaper);
#endif
	int ret = -1;
	ret = gusbctrl.OpenDoorPaper();
	return ret;
}


int FOpenDoorSafe(void)
{
#ifdef _LOG
	INTERFACE_LOG(FOpenDoorSafe);
#endif
	int ret = -1;
	ret = gusbctrl.OpenDoorSafe();
	return ret;
}
int FCloseDoorSafe(void)
{
#ifdef _LOG
	INTERFACE_LOG(FCloseDoorSafe);
#endif
	int ret = -1;
	ret = gusbctrl.CloseDoorSafe();
	return ret;
}
int FLightCtrl(char light, int op)
{
#ifdef _LOG
	INTERFACE_LOG(FLightCtrl);
#endif
	int ret = -1;
	ret = gusbctrl.LinghtCtrl(light, op);
	return ret;
}
int FMoveX(unsigned short x_point)
{
#ifdef _LOG
	INTERFACE_LOG(FMoveX);
#endif
	int ret = -1;
	ret = gusbctrl.MoveX(x_point);
	return ret;
}
int FMoveY(unsigned short y_point)
{
#ifdef _LOG
	INTERFACE_LOG(FMoveY);
#endif
	int ret = -1;
	ret = gusbctrl.MoveY(y_point);
	return ret;
}
int FTurnSeal(unsigned short angle)
{
#ifdef _LOG
	INTERFACE_LOG(FTurnSeal);
#endif
	int ret = -1;
	ret = gusbctrl.TurnSeal(angle);
	return ret;
}

int FStartStamperstrc(STAMPERPARAM *pstamperparam)
{
#ifdef _LOG
	INTERFACE_LOG(FStartStamperstrc);
#endif
	int ret = -1;
	ret = gusbctrl.StartStamper(pstamperparam);
	return ret;
}

int FStartStamper(
    unsigned int serial,
    char *seal, char isPadInk, 
    unsigned short x_point, unsigned short y_point, 
    unsigned short angle, unsigned short w_time,
    unsigned char type)
{
#ifdef _LOG
	INTERFACE_LOG(FStartStamper);
#endif

	STAMPERPARAM stamperparam;
	//RFID版修改
    stamperparam.serial_number = serial;
	memcpy(stamperparam.seal, seal, sizeof(stamperparam.seal));
	stamperparam.isPadInk = isPadInk;
	stamperparam.x_point = x_point;
	stamperparam.y_point = y_point;
	stamperparam.angle = angle;
	stamperparam.w_time = w_time;
    stamperparam.type = type;

	int ret = -1;
	ret = gusbctrl.StartStamper(&stamperparam);
	return ret;
}

int FStartStamperEx(unsigned int serial, char sealNo, char isPadInk,
    short x_point, short y_point, short angle, short w_time,
    unsigned char type)
{
    unsigned int rfid = 0;
    int ret = GetStamperID(sealNo - 1, rfid);
    if (ret != STF_SUCCESS)
        return ret;

    char seal[4] = { 0 };
    memcpy(seal, &rfid, sizeof(rfid));
    return FStartStamper(serial, seal, isPadInk, x_point, y_point, angle, w_time, type);
}

int FStartStamperABC(
    unsigned int serial,
    char* abc_id,
    char isPadInk,
    short x_point,
    short y_point,
    short angle,
    short w_time,
    unsigned char type)
{
    unsigned int rfid = 0;
    int ret = gusbctrl.GetRFID(abc_id, rfid);
    if (0 != ret) {
        CLog::sharedLog()->WriteNormalLog("FStartStamperABC->获取农行电子标签(%s)对应的RFID失败", abc_id);
        return STF_INVALID_STAMPER;
    }

    char seal[4] = { 0 };
    memcpy(seal, &rfid, sizeof(rfid));
    return FStartStamper(serial, seal, isPadInk, x_point, y_point, angle, w_time, type);
}

int FSelectStamper(unsigned int serial, unsigned int seal_id, char	isPadInk)
{
#ifdef _LOG
	INTERFACE_LOG(FSelectStamper);
#endif
	int ret = -1;
	ret = gusbctrl.SelectStamper(serial, seal_id, isPadInk);
	return ret;
}
int FCancleStamper(void)
{
#ifdef _LOG
	INTERFACE_LOG(FCancleStamper);
#endif
	int ret = -1;
	ret = gusbctrl.CancleStamper();
	return ret;
}
int FBeepCtrl(char beep, char interval)
{
#ifdef _LOG
	INTERFACE_LOG(FBeepCtrl);
#endif
	int ret = -1;
	ret = gusbctrl.BeepCtrl(beep, interval);
	return ret;
}
int FLightBrightness(char light, char brightness)
{
#ifdef _LOG
	INTERFACE_LOG(FLightBrightness);
#endif
	int ret = -1;
	ret = gusbctrl.LightBrightness(light, brightness);
	return ret;
}
int FSealBack(void)
{
#ifdef _LOG
	INTERFACE_LOG(FSealBack);
#endif
	int ret = -1;
	ret = gusbctrl.SealBack();
	return ret;
}
int FInTestMode(void)
{
#ifdef _LOG
	INTERFACE_LOG(FInTestMode);
#endif
	int ret = -1;
	ret = gusbctrl.InTestMode();
	return ret;
}
int FOutTestMode(void)
{
#ifdef _LOG
	INTERFACE_LOG(FOutTestMode);
#endif
	int ret = -1;
	ret = gusbctrl.OutTestMode();
	return ret;
}

int GetMacAdress(char num, OUT char* strmac, int len)
{
#ifdef _LOG
	INTERFACE_LOG(GetMacAdress);
#endif

    int ret = 0;
    if (1 == num) {
        ret = ReadMAC((unsigned char*)strmac, NULL, len);
    } else if (2 == num) {
        ret = ReadMAC(NULL, (unsigned char*)strmac, len);
    } else {
        ret = STF_INVALID_PARAMETER;
    }

    return ret;

	/*unsigned char str[6] = {0xFC, 0xAA,0x14, 0x90 ,0x69, 0x7B};*/
	unsigned char str[6] = {0};
	
	ret = gusbctrl.GetMacAdress(num, str, sizeof(str));
	char str1[18] = {0};

	char	ddl,ddh;
	int i;

	for (i=0; i<6; i++)
	{
		ddh = 48 + str[i] / 16;
		ddl = 48 + str[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		str1[i*3] = ddh;
		str1[i*3+1] = ddl;
		str1[i*3+1+1] = ':';
	}

	str1[17] = 0x00;

	unsigned char* pData = static_cast<unsigned char*>((void*)(strmac));

	memcpy(strmac, str1, len>18?18:len);	
	return ret;
}


//左移 4*(len -1 -i) 位
#define LEFT(len, i) (4 * ((len) - 1- (i)))
//将字符串str转换成对应的16进制数
bool StringToSixteen(string str, char &value)
{
	value = 0;
	int len = str.length();
	if(len < 1 || len > 8)
		return false;

	char a;
	for(int i=0; i<len; i++)
	{
		a = str[i];
		if(a >= '0' && a <= '9')
			value += atoi(&a) << LEFT(len, i);
		else
		{
			switch(a)
			{
			case 'a':
			case 'A':
				value += 10 << LEFT(len, i);
				break;
			case 'b':
			case 'B':
				value += 11 << LEFT(len, i);
				break;
			case 'c':
			case 'C':
				value += 12 << LEFT(len, i);
				break;
			case 'd':
			case 'D':
				value += 13 << LEFT(len, i);
				break;
			case 'e':
			case 'E':
				value += 14 << LEFT(len, i);
				break;
			case 'f':
			case 'F':
				value += 15 << LEFT(len, i);
				break;
			default:
				value = 0;
				return false;
			}
		}		
	}

	return true;
}

int FBindMac(char op, char num, char* strmac, int len)
{
    int ret = 0;
    if (1 == op) { //绑定
        if (1 == num)
            ret = WriteMAC((unsigned char*)strmac, NULL, len, len);
        else if (2 == num)
            ret = WriteMAC(NULL, (unsigned char*)strmac, len, len);
        else
            ret = STF_INVALID_PARAMETER;
    } else if (0 == op) { //解绑
        if (1 == num) {
            ret = WriteMAC((unsigned char*)strmac, NULL, len, 0);
        } else if (2 == num) {
            ret = WriteMAC(NULL, (unsigned char*)strmac, 0, len);
        } else
            ret = STF_INVALID_PARAMETER;
    } else {
        ret = STF_INVALID_PARAMETER;
    }

    return ret;

	if (len==17)
	{			
		char str[6] = {0};
		int i = 0;
		char* token = strtok( strmac, ":");
		while( token != NULL && i<6 )
		{		
			StringToSixteen(token, str[i]);	
			token = strtok( NULL, ":");
			i++;
		}

// 		CLog* p = CLog::sharedLog();
// 		p->WriteDebugLog("%2x,%2x,%2x,%2x,%2x,%2x;%d",str[0],str[1],str[2],str[3],str[4],str[5], len);

		ret = gusbctrl.BindMac(op, num, str, sizeof(str));
	}
	return ret;
}

int SetDevCode(char* code, int len)
{
#ifdef _LOG
	INTERFACE_LOG(SetDevCode);
#endif

    return WriteIntoStamper(
        DEV_CODE_OFFSET, 
        (unsigned char*)code, 
        len >= DEV_CODE_LEN? DEV_CODE_LEN: len);
}

int GetDevCode(OUT char* code, int len)
{
#ifdef _LOG
	INTERFACE_LOG(GetDevCode);
#endif

    unsigned char ret_len = 0;
    return ReadStamperMem(DEV_CODE_OFFSET, DEV_CODE_LEN, (unsigned char*)code, ret_len);
}

int FfirewareUpdate(unsigned char cmd, unsigned char* data, int len)
{
#ifdef _LOG
	INTERFACE_LOG(FfirewareUpdate);
#endif
	int ret = -1;
	ret = gusbctrl.firewareUpdate(cmd, data, len);
	return ret;
}

int FDebugLogSwitch(char op)
{
#ifdef _LOG
	INTERFACE_LOG(FDebugLogSwitch);
#endif
	int ret = -1;
	ret = gusbctrl.DebugLogSwitch(op);
	return ret;
}

 int Calibration( char* points,int len)
 {
#ifdef _LOG
	INTERFACE_LOG(Calibration);
#endif
	 int ret = -1;
	 ret = gusbctrl.Calibration(points,len);
	 return ret;
 }

  int  CalculatePos(double* x1, double* y1, double* x2, double* y2, double* scalex,double*scaley)
 {
	 int ret = -1;
	ret = gusbctrl.CalculatePos(x1,y1,x2,y2,scalex,scaley);
	return ret;

 }

   int  CalculatePosition(int stamperPointX,int stanperPointY,char* points,int len)
   {
	    int ret = -1;
	ret = gusbctrl.STCalculatePosition(stamperPointX,stanperPointY,points,len);
	return ret;
   }

   int  CalibrationMutiple(char * points,int len)
   {

	   int ret = -1;
	   ret = gusbctrl.CalibrationMutiple(points,len);
	   return ret;
   }


int CalibrationEx(char * pStampid,char *points,int len)
{
    int ret = -1;
    ret = gusbctrl.CalibrationEx(pStampid, points, len);
    return  ret;
}

int SetAlarm(char alarm, char operation)
{
    int ret = -1;
    ret = gusbctrl.SetAlarm(alarm, operation);
    return  ret;
}

struct StamperInfo {

    StamperInfo()
    {
        memset(data, 0, sizeof(data));
    }

    void SetStamperID(const unsigned char* id, unsigned short len = 4) 
    {
        memcpy(data, id, len < 4? len: 4);
    }

    void SetReserved(const unsigned char* extra, unsigned short len = 4)
    {
        memcpy(data + 4, extra, len < 4? len: 4);
    }

    unsigned char data[8];
};

int SetStampMap()
{
    unsigned int rfids[7] = { 0 };
    unsigned char cur_stampers = 0;
    int ret = ReadAllRFID(rfids, 7, &cur_stampers);
    if (ret != STF_SUCCESS) {
        CLog::sharedLog()->WriteNormalLog("获取章映射失败, err:%d", ret);
        return ret;
    }

    //获取印章信息并存入映射关系
    unsigned char str[256] = { 0 };
    memcpy(str, rfids, 6 * sizeof(unsigned int));

    struct StamperInfo stampers[6];
    for (int i = 0; i < 6; ++i)
        stampers[i].SetStamperID(&str[4 * i], 4);

    unsigned char stamper_table[52] = { 0 };
    memcpy(stamper_table, &stampers, sizeof(stampers));
    unsigned int sum = 0;
    for (int i = 0; i < 48; ++i) {
        sum += stamper_table[i];
    }

    sum = ~sum;
    memcpy(stamper_table + 48, &sum, sizeof(sum));
    ret = gusbctrl.SetStampMap(stamper_table, sizeof(stamper_table));
	return  ret;
}

 int   GetStampMap(char* mapout,int len)
 {
     int ret = -1;
     ret = gusbctrl.GetStampMap(mapout, len);
     return  ret;
 }

 int GetPhsicalRange(char * phsicalOut,int len)
 {
     int ret = -1;
	 ret =   gusbctrl.GetPhsicalRange(phsicalOut,len);
     return  ret;
 }

 int GetStorageCapacity(unsigned char& version, unsigned short& mem_size)
 {
     return gusbctrl.GetStorageCapacity(version, mem_size);
 }

 // data    --- 待写入数据, C字符串
 // len     --- 写入长度, 通过指定len可以表示是否需要写入0结尾字符
 // 如: data = "case"
 //     len = 4, 表示不写入0结尾字符
 //     len  5, 表示要写入0结尾字符
 int WriteIntoStamper(unsigned short offset, const unsigned char* data, unsigned char len)
 {
     if (NULL == data || 0 == len)
         return 0;

     return gusbctrl.WriteIntoStamper(offset, data, len);
 }

 int ReadStamperMem(unsigned short offset, unsigned short size,
     unsigned char* data, unsigned char& len)
 {
     return gusbctrl.ReadStamperMem(offset, size, data, len);
 }

 int SelectStamper(unsigned char stamper)
 {
     return gusbctrl.SelectStamper(stamper);
 }

 int GetStamperID(unsigned char stamper, unsigned int& rfid)
 {
     return gusbctrl.GetStamperID(stamper, rfid);
 }

 int GetStamper(unsigned int rfid, unsigned char& stamper)
 {
     return gusbctrl.GetStamper(rfid, stamper);
 }

int EnterAuthMode()
{
    return gusbctrl.EnterAuthMode();
}

int ExitAuthMode()
{
     return gusbctrl.ExitAuthMode();
}

int EnableFactoryMode(unsigned char enable)
{
    return gusbctrl.EnableFactory(enable);
}

int VerifyKey(char key_type, const unsigned char* key, unsigned char len)
{
    return gusbctrl.VerifyKey(key_type, key, len);
}

int OperateBlock(unsigned char block)
{
    return gusbctrl.OperateBlock(block);
}

int ReadBlock(unsigned char block, char* data, unsigned char len)
{
    return gusbctrl.ReadBlock(block, data, len);
}

int WriteBlock(unsigned char block, char* data, unsigned char len)
{
    return gusbctrl.WriteBlock(block, data, len);
}

int WriteMAC(
    const unsigned char* mac1, 
    const unsigned char* mac2, 
    unsigned char max_mac1_len,
    unsigned char max_mac2_len)
{
//     unsigned char* mac_addr = new (std::nothrow) unsigned char[MAX_MAC + 1];
//     if (NULL == mac_addr)
//         return STF_ALLOCATE_MEM_FAIL;

    int ret1 = 0;
    if (NULL != mac1 && /*0 != strlen((char*)mac1) &&*/ 0 != max_mac1_len) {
//         memset(mac_addr, 0, MAX_MAC + 1);
//         unsigned char len = max_mac1_len >= MAX_MAC ? MAX_MAC : max_mac1_len;
//         memcpy(mac_addr, mac1, len);
//         mac_addr[MAX_MAC] = '\0';
        ret1 = WriteIntoStamper(
            MAC1_OFFSET,
            mac1,
            max_mac1_len);
    }
    
    int ret2 = 0;
    if (NULL != mac2 /*&& 0 != strlen((char*)mac2)*/ && 0 != max_mac2_len) {
//         memset(mac_addr, 0, MAX_MAC + 1);
//         unsigned char len = max_mac2_len >= MAX_MAC ? MAX_MAC : max_mac2_len;
//         memcpy(mac_addr, mac2, len);
//         mac_addr[MAX_MAC] = '\0';
        ret2 = WriteIntoStamper(
            MAC2_OFFSET,
            mac2,
            max_mac2_len);
    }
//     
//     delete[] mac_addr;

    if (STF_SUCCESS == ret1 && STF_SUCCESS == ret2)
        return 0;
    else
        return STF_WRITE_MAC_FAIL;
}

int ReadMAC(
    unsigned char* mac1, 
    unsigned char* mac2, 
    unsigned char mac_len)
{
    unsigned char len = mac_len >= MAX_MAC ? MAX_MAC : mac_len;
    unsigned char ret_len = 0;
    int ret1 = 0;
    int ret2 = 0;
    if (NULL != mac1) {
        ret1 = ReadStamperMem(MAC1_OFFSET, len, mac1, ret_len);
        if (STF_SUCCESS == ret1)
            mac1[len] = '\0';
    }

    if (NULL != mac2) {
        ret2 = ReadStamperMem(MAC2_OFFSET, len, mac2, ret_len);
        if (STF_SUCCESS == ret1)
            mac2[len] = '\0';
    }

    if (STF_SUCCESS == ret1 && STF_SUCCESS == ret2)
        return 0;
    else
        return STF_READ_MAC_FAIL;
}

int WriteKey(const unsigned char* key, unsigned char key_len)
{
    if (NULL == key)
        return STF_INVALID_PARAMETER;

    unsigned char* key_addr = new (std::nothrow) unsigned char[KEY_LEN + 1];
    if (NULL == key_addr)
        return STF_ALLOCATE_MEM_FAIL;

    memset(key_addr, 0, KEY_LEN + 1);
    unsigned char len = key_len >= KEY_LEN ? KEY_LEN : key_len;
    memcpy(key_addr, key, len);
    key_addr[KEY_LEN] = '\0';
    int ret = WriteIntoStamper(
        KEY_OFFSET,
        key_addr,
        len);
    delete[] key_addr;

    return ret;
}

int ReadKey(unsigned char* key, unsigned char key_len)
{
    if (NULL == key)
        return STF_INVALID_PARAMETER;

    unsigned char len = key_len >= KEY_LEN ? KEY_LEN : key_len;
    unsigned char ret_len = 0;
    int ret = ReadStamperMem(KEY_OFFSET, len, key, ret_len);
    if (STF_SUCCESS == ret)
        key[len] = '\0';

    return ret;
}

int WriteCalibrationPoint(unsigned short* points, unsigned char len)
{
    unsigned char total = len * sizeof(unsigned short);
    unsigned char* data = new (std::nothrow) unsigned char[total];
    if (NULL == data)
        return STF_ALLOCATE_MEM_FAIL;

    memcpy(data, points, total);
    int ret = WriteIntoStamper(
        CALIBRATION_OFFSET,
        data,
        total);
    delete[] data;

    return ret;
}

int CalibrationPoint(unsigned short* points, unsigned char len)
{
    unsigned char total = len * sizeof(unsigned short);
    unsigned char* data = new (std::nothrow) unsigned char[total];
    if (NULL == data)
        return STF_ALLOCATE_MEM_FAIL;

    unsigned char ret_len = 0;
    int ret = ReadStamperMem(
        CALIBRATION_OFFSET, 
        total,
        data, 
        ret_len);
    if (STF_SUCCESS != ret)
        return ret;

    memcpy(points, data, total);
    delete[] data;

    return STF_SUCCESS;
}

int WriteImageConversionRatio(const unsigned char* ratio, unsigned char len)
{
    return WriteIntoStamper(
        IMAGE_RATION_OFFSET,
        ratio,
        len > IMAGE_RATION_LEN? IMAGE_RATION_LEN: len);
}

int ReadImageConversionRatio(unsigned char* ratio, unsigned char len)
{
    unsigned char ret_len = 0;
    return ReadStamperMem(IMAGE_RATION_OFFSET, len > IMAGE_RATION_LEN? IMAGE_RATION_LEN: len, 
        ratio, ret_len);
}

int WriteImageConvRatio(float* x, float* y)
{
    if (NULL == x || NULL == y)
        return STF_INVALID_PARAMETER;

    unsigned char xy[16] = { 0 };
    memcpy(xy, x, sizeof(float));
    memcpy(xy + sizeof(float), y, sizeof(float));
    return WriteImageConversionRatio(xy, sizeof(xy));
}

int ReadImageConvRatio(float* x, float* y)
{
    unsigned char xy[16] = { 0 };
    unsigned char ret_len = 0;
    int ret = ReadStamperMem(IMAGE_RATION_OFFSET, sizeof(xy), xy, ret_len);
    if (ret != 0)
        return ret;

    memcpy(x, xy, sizeof(float));
    memcpy(y, xy + sizeof(float), sizeof(float));
    return 0;
}

int CheckStampers()
{
    char stamper_mapped[52] = { 0 };
    int ret = GetStampMap(stamper_mapped, sizeof(stamper_mapped));
    if (STF_SUCCESS != ret)
        return ret;

    std::map<unsigned int, int> stored_stampers; //印章id和印章号(从0开始)的映射关系
    unsigned int rfid = 0;
    //解析存储的印章id和印章号的映射关系
    for (int i = 0; i < 6; i++) {
        memcpy(&rfid, &stamper_mapped[i * 8], 4);
        std::pair<std::map<unsigned int, int>::iterator, bool> ret
            = stored_stampers.insert(std::make_pair(rfid, i));
        if (!ret.second && 0 != rfid)
            return STF_DUPLICATE_STAMPER;
    }

    //当前印章id和印章号间的映射关系
    std::map<unsigned int, int> current_stampers;
    for (int i = 0; i < 6; ++i) {
        int ret = GetStamperID(i, rfid);
        if (STF_SUCCESS != ret)
            continue;

        current_stampers.insert(std::make_pair(rfid, i));
    }

    std::map<unsigned int, int>::iterator it_curr;
    std::map<unsigned int, int>::iterator it_stored;
    for (it_curr = current_stampers.begin(); it_curr != current_stampers.end(); ++it_curr) {
        it_stored = stored_stampers.find((*it_curr).first);
        if (stored_stampers.end() == it_stored) {
            return STF_INVALID_STAMPER;
        } else {
            if (it_curr->second != it_stored->second)
                ret = STF_STAMPER_SEQ_CHANGE;
        }
    }

    for (it_stored = stored_stampers.begin(); it_stored != stored_stampers.end(); ++it_stored) {
        if (current_stampers.end() == current_stampers.find((*it_stored).first))
            return STF_STAMPER_LOSS;
    }

    return ret;
}

int ReadAllRFID(unsigned int* rfids, unsigned char len, unsigned char* stampers)
{
    return gusbctrl.ReadAllRFID(rfids, len, stampers);
}

static const Mutex mutex;

int Lock()
{
    if (IsLocked())
        return 0;

    const unsigned char lockbit[] = { 1, 0x0 };
    CLock lk(mutex);
    return WriteIntoStamper(LOCK_OFFSET, lockbit, strlen((char*)lockbit));
}

int Unlock()
{
    const unsigned char unlockbit[] = { 2, 0x0 };
    CLock lk(mutex);
    return WriteIntoStamper(LOCK_OFFSET, unlockbit, strlen((char*)unlockbit));
}

bool IsLocked()
{
    unsigned char lockbit[2] = { 0 };
    unsigned char ret_len = 0;
    CLock lk(mutex);
    int ret = ReadStamperMem(LOCK_OFFSET, 1, lockbit, ret_len);
    if (STF_SUCCESS != ret)
        return false;

    return lockbit[0] == 1;
}

// len      --- 不包括0结尾字符
int WriteStamperIdentifier(const unsigned char* id, unsigned char len)
{
    return WriteIntoStamper(ID_OFFSET, id, len + 1 >= ID_LEN ? ID_LEN : len + 1);
}

int ReadStamperIdentifier(unsigned char* id, unsigned char len)
{
    unsigned char ret_len = 0;
    return ReadStamperMem(ID_OFFSET, ID_LEN, id, ret_len);
}

int ReadAlarmVoltage(unsigned char* voltage)
{
    return gusbctrl.ReadAlarmVoltage(voltage);
}

int Reset()
{
    return gusbctrl.Reset();
}

int Confirm(
    unsigned int serial,
    unsigned int rfid,
    char isPadInk,
    short x_point,
    short y_point,
    short angle,
    short w_time,
    unsigned char type)
{
    return gusbctrl.Confirm(
        serial,
        rfid,
        isPadInk,
        x_point,
        y_point,
        w_time,
        angle,
        type);
}

int ReadStamp(unsigned char* info, unsigned char len)
{
    return gusbctrl.ReadStamp(info, len);
}

int ClearStatistic()
{
    return gusbctrl.ClearStatistic();
}

int ReadBackupSN(unsigned char* sn, unsigned char len)
{
    return gusbctrl.ReadBackupSN(sn, len);
}

int WriteBackupSN(const unsigned char* sn, unsigned char len)
{
    return gusbctrl.WriteBackupSN(sn, len);
}

int SetSideDoor(unsigned short keep_open, unsigned short timeout)
{
    return gusbctrl.SetSideDoor(keep_open, timeout);
}

int SetPaperDoor(unsigned short timeout)
{
    return gusbctrl.SetPaperDoor(timeout);
}

int ReadAlarmStatus(char* door, char* vibration)
{
    return gusbctrl.ReadAlarmStatus(door, vibration);
}

int GetHardwareVer(char* version, unsigned char len)
{
    return gusbctrl.GetHardwareVer(version, len);
}

int GetABCStamper(unsigned char stamper, char* id, unsigned char len)
{
    return gusbctrl.GetABCStamper(stamper, id, len);
}

int GetABCStamperIndex(const char* abc_id, char* index)
{
    return gusbctrl.GetABCIndex(abc_id, *index);
}
