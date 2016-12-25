// PSBC_STAMP.h : main header file for the PSBC_STAMP DLL
//

#pragma once

#include <map>
#include <string>
#include "common.h"
#include "RZCamera.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

enum FontLib {
    image = 0,
    eng = 1
};

struct Point {
    Point() : x(0), y(0) {}

    Point(int _x, int _y) : x(_x), y(_y) {}

    int x;
    int y;
};

class PSBCSTDZDeviceSTAMPDeviceApp : public CWinApp {
public:
    PSBCSTDZDeviceSTAMPDeviceApp();

    int Test();
    // 重写

public:
    // No.1
    int InitializeMachine(void);

    // No.2
    char* querySealInfo(const char* machineNum);

    // No.3
    char* querySloatInfo(const char* machineNum);

    // No.4
    int initStamp(const char* machineNum, const char* slotNumAndStampId);

    // No.5
    int setAcrossPageSeal(void);

    // No.6
    int setMachineNum(const char* machineNum);

    // No.7
    int openPaperDoor(void);
    
    // No.8
    int checkManualPrintPara(int pointX, int pointY, int angle);

    int ManualStart(
        int printNum,
        int pointX,
        int pointY,
        int angle,
        int ink);
    // No.9
    int manualPrintStart(int printNum, int pointX, int pointY, int angle);

    // No.10
    int manualPrintStartByLight(int printNum, int pointX, int pointY, int angle);

    // No.11
    int autoPrintStart(int printNum, int pointX, int pointY, int angle);

    // No.12
    int openMachineBackDoor(void);

    // No.13
    int openMachineBackDoorUnNormal(const char* openInfo);

    // No.14
    char* getMachineNum(void);

    // No.15
    // 查询设备型号(ST-SCM-F60)
    char* getMachineType(void);

    // No.16
    int checkPaperDoorState(void);

    // No.17
    int checkBackDoorState(void);

    // No.18
    int lockPrinter(void);

    // No.19
    int unLockPrinter(void);

    // No.20
    int checkLockState(void);

    // No.21
    int openVideoCapLight(void);

    // No.22
    int closeVideoCapLight(void);

    // No.23
    char* geterrMsg(int errNo);

    // No.24
    int connMachine(const char* seriaID);

    // No.25
    int disconnMachine(void);

    // No.26
    int isConnMachine(void);

    char* readOpenBackDoorExceptionInfo(void);

    int delOpenBackDoorExceptionInfo(void);

    //////////////////////////////////////////////////////////////////////////

    int openVideoCap(void);

    int setVedioProperties(
        int brightness, 
        int constrast, 
        int hue, 
        int saturation, 
        int sharpness, 
        int whitebalance, 
        int gain);

    int getImageFormat(const char* filePath, int type, int isEraseBorder);

    int revolveImg(const char* filePath, const char*  targetPath, int angle);

    int closeVideoCap(void);

    int checkVideoState(void);

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    DECLARE_MESSAGE_MAP()

public:
    static int _stdcall DevConnCallBack(
        const char*     DevPath,
        unsigned int    uMsg);

    static int _stdcall DevMessageCallBack(
        unsigned int    uMsg,
        unsigned int    wParam,
        long            lParam,
        unsigned char*  data,
        unsigned char   len);

    static HANDLE notify_finish;

private:
    // slotNumAndStampId --- 0字符结尾
    bool ParseSlotAndStamperID(
        const char* slotNumAndStampId, 
        std::map<int, std::string>& slot_stamper);

    struct Point* GetSealCoord(int nX, int nY);

    ErrorCode CheckMachineNum(const char* machine);

	bool CopyFile(std::string strFrom,std::string strTo);

    // 由于邮储接口中没有关安全门接口, 
    // 暂按以下原则处理:
    //     如果开安全门后调用其它任何接口(除isConnMachine)则退出维护模式并关安全门
    void ExitIfInMaintain();

    // type     --- 0(BMP), 1(JPG), 2(RAW)
    enum IMAGEFORMAT CvtType(int type) {
        switch (type) {
        case 0:
            return FORMATBMP;
        case 1:
            return FORMATJPG;
        case 2:
        default:
            return FORMATBMP;
        }
    }

private:
    // 记录实际写入印章信息长度
    static const int SLOT_STAMPER_WRITTEN_OFFSET = 148;
    static const int SLOT_STAMPER_WRITTEN_SIZE = 1;
    static const int SLOT_STAMPER_OFFSET = 150;
    static const int MAX_SLOT_STAMPER_SIZE = 90; // 15 * 6

    static const int STEP = 2;
    // 记录异常开锁信息
    static const int MAX_EXCEPTION_OPEN_COUNT = 4;
    static const int OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET = 
        SLOT_STAMPER_OFFSET + 
        MAX_SLOT_STAMPER_SIZE + 
        STEP;
    static const int OPEN_SAFE_DOOR_EXCEPTION_MARK_SIZE = 1;

    static const int OPEN_SAFE_DOOR_EXCEPTION_OFFSET =
        OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET +
        OPEN_SAFE_DOOR_EXCEPTION_MARK_SIZE;
    static const int OPEN_SAFE_DOOR_EXCEPTION_SIZE = 16;

    static bool connected_;     // 印控机与PC连接状况标记

private:
    bool            across_page_seal_called_;   // 盖骑缝章标记

    char*           querySealInfo_str_;
    char*           querySloatInfo_str_;
    unsigned char*  getMachineNum_str_;
    unsigned char*  readOpenBackDoorExceptionInfo_str_;
    char*           getMachineType_str_;

    bool            init_;              // 标记是否自检
    bool            mainintain_mode_;   // 当前是否是维护模式

    bool            video_opened_;      // 凭证摄像头是否开启

    std::string     src_image_;         // 存放原图路径
};

void WriteLog(int level, const char * fmt, ...);

// 打开/关闭凭证摄像头
int PrepareCamera();

void DisableCamera();

//盖章参数
struct StampPara {
    static const int MAX_X = 275; //毫米
    static const int MAX_Y = 250; //毫米
    static const int MAX_ANGLE = 360; //度

    static const int DEFAULT_X = 100;
    static const int DEFAULT_Y = 100;
    static const int DEFAULT_ANGLE = 0;

    static const int DEFAULT_WAIT = 1;
    static const int DEFAULT_TIMES = 9999;

    unsigned int rfid;
    unsigned char stamp_idx; //印章号, 从1开始
    unsigned char ink;       //是否蘸印油, 1--是
    unsigned char wait;      //等待时间,秒
    unsigned short x;
    unsigned short y;
    unsigned short angle;

    StampPara() : ink(0), rfid(0) {}
};
