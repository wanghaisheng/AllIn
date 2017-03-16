// PSBC.STDZ.Device.STAMP.Device.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Resource.h"
#include "SealLog.h"
#include "psbc_agent.h"
#include "PSBC.STDZ.Device.STAMP.Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern PSBCSTDZDeviceSTAMPDeviceApp theApp;

STMAP_DEVICE_PSBC_API int Execute(\
	const char* typeId,\
	const char* transcode,\
	const char* clientconfig,\
	const char* deviceconfig,\
	const char* instrut,\
	const void* input1,\
	const void* input2,\
	void* output1,\
	void* output2,\
	char* errormessage\
	)
{
	WriteSealLog(1,"PSBC");

	if(input1)
	{
		StampParm* parm = (StampParm*)(input1);
		if(parm)
		{
			printf("%s",parm->parm1);
		}
	}
	if(output1)
	{
		StampParm* parm = (StampParm*)(output1);
		if(parm)
		{
			printf("%s",parm->parm1);
		}
	}
	if(errormessage)
	{
		memcpy(errormessage,"hello,world!",13);
	}
	if(instrut)
	{
		if(strcmp(instrut, "0010SC01")==0)
		{
			WriteSealLog(1,instrut);
		}
	}
	return theApp.Test();
}

////////////////////////////// 用印机接口 //////////////////////////////////

// No. 1
STMAP_DEVICE_PSBC_API int initializationMachine(void)
{
	return theApp.InitializeMachine();
}

// No. 2
STMAP_DEVICE_PSBC_API char* querySealInfo(const char* machineNum)
{
	return theApp.querySealInfo(machineNum);
}

// No.3
STMAP_DEVICE_PSBC_API char* querySloatInfo(const char* machineNum)
{
	return theApp.querySloatInfo(machineNum);
}

// No.4
STMAP_DEVICE_PSBC_API int initStamp(const char* machineNum, const char* slotNumAndStampId)
{
	return theApp.initStamp(machineNum, slotNumAndStampId);
}

// No.5
STMAP_DEVICE_PSBC_API int setAcrossPageSeal(void)
{
    return theApp.setAcrossPageSeal();
}

// No.6
STMAP_DEVICE_PSBC_API int setMachineNum(const char* machineNum)
{
    return theApp.setMachineNum(machineNum);
}

// No.7
STMAP_DEVICE_PSBC_API int openPaperDoor(void)
{
    return theApp.openPaperDoor();
}

// No.8
STMAP_DEVICE_PSBC_API int checkManualPrintPara( int pointX, int pointY, int angle)
{
    return theApp.checkManualPrintPara(pointX, pointY, angle);
}

// No.9
STMAP_DEVICE_PSBC_API int manualPrintStart(int printNum, int pointX, int pointY, int angle)
{
    return theApp.manualPrintStart(printNum, pointX, pointY, angle);
}

// No.10
STMAP_DEVICE_PSBC_API int manualPrintStartByLight(int printNum, int pointX, int pointY, int angle)
{
    return theApp.manualPrintStartByLight(printNum, pointX, pointY, angle);
}

// No.11
STMAP_DEVICE_PSBC_API int autoPrintStart(int printNum, int pointX, int pointY, int angle)
{
    return theApp.autoPrintStart(printNum, pointX, pointY, angle);
}

// No.12
STMAP_DEVICE_PSBC_API int openMachineBackDoor(void)
{
    return theApp.openMachineBackDoor();
}

// No.13
STMAP_DEVICE_PSBC_API int openMachineBackDoorUnNormal(const char* openInfo)
{
    return theApp.openMachineBackDoorUnNormal(openInfo);
}

// No.14
STMAP_DEVICE_PSBC_API char* getMachineNum(void)
{
    return theApp.getMachineNum();
}

// No.15
STMAP_DEVICE_PSBC_API char* getMachineType(void)
{
	return theApp.getMachineType();
}

// No.16
STMAP_DEVICE_PSBC_API int checkPaperDoorState(void)
{
    return theApp.checkPaperDoorState();
}

// No.17
STMAP_DEVICE_PSBC_API int checkBackDoorState(void)
{
    return theApp.checkBackDoorState();
}

// No.18
STMAP_DEVICE_PSBC_API int lockPrinter(void)
{
    return theApp.lockPrinter();
}

// No.19
STMAP_DEVICE_PSBC_API int unLockPrinter(void)
{
    return theApp.unLockPrinter();
}

// No.20
STMAP_DEVICE_PSBC_API int checkLockState(void)
{
    return theApp.checkLockState();
}

// No.21
STMAP_DEVICE_PSBC_API int openVideoCapLight(void)
{
    return theApp.openVideoCapLight();
}

// No.22
STMAP_DEVICE_PSBC_API int closeVideoCapLight(void)
{
    return theApp.closeVideoCapLight();
}

// No.23
STMAP_DEVICE_PSBC_API char* geterrMsg(int errNo)
{
    return theApp.geterrMsg(errNo);
}

// No.24
STMAP_DEVICE_PSBC_API int connMachine(const char* seriaID)
{
    return theApp.connMachine(seriaID);
}

// No.25
STMAP_DEVICE_PSBC_API int disconnMachine(void)
{
    return theApp.disconnMachine();
}

// No.26
STMAP_DEVICE_PSBC_API int isConnMachine(void)
{
    return theApp.isConnMachine();
}

//===============CAMEAR API===============

// No.27
STMAP_DEVICE_PSBC_API int openVideoCap(void)
{
    return theApp.openVideoCap();
}

// No.28
STMAP_DEVICE_PSBC_API int setVedioProperties(
    int brightness, 
    int constrast, 
    int hue,
    int saturation,
    int sharpness,
    int whitebalance, 
    int gain)
{
    return theApp.setVedioProperties(
        brightness, 
        constrast, 
        hue, 
        saturation, 
        sharpness, 
        whitebalance, 
        gain);
}

// No.29
STMAP_DEVICE_PSBC_API int getImageFormat(const char* filePath, int type, int isEraseBorder)
{
    return theApp.getImageFormat(filePath, type, isEraseBorder);
}

// No.30
STMAP_DEVICE_PSBC_API int revolveImg(const char* filePath, const char*  targetPath, int angle)
{
    return theApp.revolveImg(filePath, targetPath, angle);
}

// No.31
STMAP_DEVICE_PSBC_API int closeVideoCap(void)
{
	return theApp.closeVideoCap();
}

// No.32
STMAP_DEVICE_PSBC_API int checkVideoState(void)
{
	return theApp.checkVideoState();
}

// No.33
STMAP_DEVICE_PSBC_API char* readOpenBackDoorExceptionInfo(void)
{
	return theApp.readOpenBackDoorExceptionInfo();
}

// No.34
STMAP_DEVICE_PSBC_API int delOpenBackDoorExceptionInfo(void)
{
    return theApp.delOpenBackDoorExceptionInfo();
}
