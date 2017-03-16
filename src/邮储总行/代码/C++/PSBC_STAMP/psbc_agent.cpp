// PSBC_STAMP.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <time.h>
#include <ctime>
#include "SealLog.h"
#include "USBControlF60.h"
#include "parse.h"
#include "psbc_agent.h"
#include "ImgProcAndReco.h" // locate after psbc_agent.h

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#pragma comment(lib, "ABC.STDZ.Device.STAMP.USBF60APID.lib")
#pragma comment(lib, "ABC.STDZ.Device.STAMP.SealLog.lib")
#pragma comment(lib, "ABC.STDZ.Device.STAMP.RZCamera.lib")
#pragma comment(lib, "ImageProcess.lib")
#else
#pragma comment(lib, "ABC.STDZ.Device.STAMP.USBF60API.lib")
#pragma comment(lib, "ABC.STDZ.Device.STAMP.SealLog.lib")
#pragma comment(lib, "ABC.STDZ.Device.STAMP.RZCamera.lib")
#pragma comment(lib, "ImageProcess.lib")
#endif

bool PSBCSTDZDeviceSTAMPDeviceApp::connected_ = false;

HANDLE PSBCSTDZDeviceSTAMPDeviceApp::notify_finish = CreateEvent(NULL, TRUE, FALSE, NULL);

BEGIN_MESSAGE_MAP(PSBCSTDZDeviceSTAMPDeviceApp, CWinApp)

END_MESSAGE_MAP()


PSBCSTDZDeviceSTAMPDeviceApp::PSBCSTDZDeviceSTAMPDeviceApp()
{
    // TODO: 在此处添加构造代码，
    // 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CABCSTDZDeviceSTAMPDeviceApp 对象
PSBCSTDZDeviceSTAMPDeviceApp theApp;


// CABCSTDZDeviceSTAMPDeviceApp 初始化

BOOL PSBCSTDZDeviceSTAMPDeviceApp::InitInstance()
{
    CWinApp::InitInstance();
    PSBCConfig::GetInst()->Parse();

    F_RegisterDevCallBack(PSBCSTDZDeviceSTAMPDeviceApp::DevConnCallBack);
    FRegisterDevCallBack(PSBCSTDZDeviceSTAMPDeviceApp::DevMessageCallBack);
    FOpenDev(NULL); // 防止动态加载库(延迟加载)而导致的错误

    init_                               = false;
    across_page_seal_called_            = false;
    video_opened_                       = false;
    querySealInfo_str_                  = NULL;
    querySloatInfo_str_                 = NULL;
    getMachineNum_str_                  = NULL;
    readOpenBackDoorExceptionInfo_str_  = NULL;
    getMachineType_str_                 = NULL;

    return TRUE;
}

int PSBCSTDZDeviceSTAMPDeviceApp::ExitInstance()
{
    CWinApp::ExitInstance();

    delete[] querySealInfo_str_;
    delete[] querySloatInfo_str_;
    delete[] getMachineNum_str_;
    delete[] readOpenBackDoorExceptionInfo_str_;
    delete[] getMachineType_str_;
    DisableCamera();

    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::Test()
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////

int PrepareCamera()
{
    FOpenDev(NULL);
    int ret = FLightCtrl(2, 1);
    if (0 != ret)
        WriteLog(3, "PrepareCamera->打开凭证补光灯失败, er: %d", ret);

    ret = FLightBrightness(2, PSBCConfig::GetInst()->brightness_);
    if (0 != ret)
        WriteLog(3, "PrepareCamera->凭证补光灯亮度调节失败, er: %d", ret);

    ret = OpenCamera(PAPERCAMERA);
    if (0 != ret)
        WriteLog(3, "PrepareCamera->打开凭证摄像头失败, er: %d", ret);

    ret = SetResolution(
        PAPERCAMERA,
        PSBCConfig::GetInst()->resolution_width_,
        PSBCConfig::GetInst()->resolution_height_);
    if (0 != ret)
        WriteLog(3, "PrepareCamera->设置凭证摄像头分辨率失败, er: %d", ret);

    WriteLog(4, "PrepareCamera->凭证摄像头已准备好, ret: %d", ret);
    return ret;
}

void DisableCamera()
{
    //CloseCamera(PAPERCAMERA);
}

int PSBCSTDZDeviceSTAMPDeviceApp::DevConnCallBack(
    const char*     DevPath,
    unsigned int    uMsg)
{
    // uMsg: 1 连接, 0 断开
    switch (uMsg) {
    case 0: {
        ::FCloseDev();
        connected_ = false;
        DisableCamera();

        WriteLog(4, "DevConnCallBack->设备断开并关闭设备, %s", 
            connected_ ? "连接" : "断开");
    }
        break;
    case 1: {
        ::FOpenDev(DevPath);
        connected_ = true;
/*        PrepareCamera();*/

        WriteLog(4, "DevConnCallBack->设备重连并打开设备, %s",
            connected_ ? "连接" : "断开");
    }
        break;
    default:
        break;
    }

    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::DevMessageCallBack(
    unsigned int    uMsg,
    unsigned int    wParam,
    long            lParam,
    unsigned char*  data,
    unsigned char   len)
{
    switch (uMsg) {
    case STAMPER_DOWN: {            // 盖章下压通知
        WriteLog(4, "DevMessageCallBack->章下压通知");
    }
        break;
    case STAMPER_COMPLETE: {        // 盖章完成通知
        WriteLog(4, "DevMessageCallBack->盖章完成通知, 结果: %s",
            wParam == 0 ? "成功": "失败");
        SetEvent(notify_finish);
    }
        break;
    case STAMPER_SEALERR: {         // 印章掉落通知
        WriteLog(4, "DevMessageCallBack->印章掉落通知");
    }
        break;
    case 0xA8:
    case STAMPER_SIDEDOOR_CLOSE: {
        if (0 == wParam) {
             WriteLog(4, "DevMessageCallBack->机械锁关闭通知");
             SetStampMap();
             FCloseDoorSafe();   // 关电子锁
             FQuitMaintainMode();
       }
    }
        break;
    default:
        break;
    }

    return 0;
}

void WriteLog(int level, const char * fmt, ...)
{
#define LOG_SIZE 2048

    char buf[LOG_SIZE] = { 0 };
    va_list val;
    va_start(val, fmt);
    _vsnprintf_s(buf, LOG_SIZE, fmt, val);
    va_end(val);

    WriteSealLog(level, buf);
}

//////////////////////// 印控机-邮储接口实现 /////////////////////////////////////

int PSBCSTDZDeviceSTAMPDeviceApp::InitializeMachine(void)
{
    WriteLog(4, "InitializeMachine->IN");

    // 功能:	用印机自检
    // 说明 : 检查机器状态是否正常，例如：检测印章
    // 设备第一次使用或更换印章时调用

    ExitIfInMaintain();

    // 首先打开设备
    int ret = FOpenDev(NULL);
    if (0 != ret) {
        WriteLog(3, "InitializeMachine->设备打开失败, er: %d", ret);
        return EC_OPEN_FAIL;
    }

    init_ = true;

    // 更新章映射 
    ret = SetStampMap();
    if (0 != ret) {
        WriteLog(3, "InitializeMachine->更新章映射失败, er: %d", ret);
        return EC_QUERY_STAMP_FAIL;
    }

    // 复位印控机
    Reset();

    // 检测系统状态
    unsigned char status[24] = { 0 };
    ret = FGetDevStatus(status, 24);
    if (0 != ret) {
        WriteLog(3, "InitializeMachine->检测章系统失败, er: %d", ret);
        return EC_QUERY_DEVICE_FAIL;
    }

    WriteLog(4, "InitializeMachine->当前系统状态: %d", status[1]);
    WriteLog(4, "InitializeMachine->OUT");

    switch (status[1]) {
    case 0:
        return EC_NOT_INIT;
    case 1:
        return EC_STARTUP_EXAM;
    case 3:
        return EC_SUC;
    case 4:
        return EC_TEST;
    case 5:
        return EC_BREAKDOWN;
    case 6:
        return EC_STAMPING;
    case 7:
        return EC_MAINTAIN;
    default:
        return EC_FAIL;
    }
}

char* PSBCSTDZDeviceSTAMPDeviceApp::querySealInfo(const char* machineNum)
{
    if (querySealInfo_str_ == NULL)
        querySealInfo_str_ = new char[MAX_SLOT_STAMPER_SIZE + 1];

    memset(querySealInfo_str_, 0x0, MAX_SLOT_STAMPER_SIZE + 1);

    if (NULL == machineNum) {
        WriteLog(3, "PSBCSTDZDeviceSTAMPDeviceApp::querySealInfo->传入参数为空");
        return querySealInfo_str_;
    }

    WriteLog(4, "querySealInfo->IN, (machineNum) = (%s)",
        machineNum);

    ExitIfInMaintain();

    // 格式: 槽位号 : 印章ID; 槽位号:印章ID(0 表示无章)...
    // 例(1:00000000001a; 2:00000000002b; 3:0...)

    // 先校验设备编号
    ErrorCode ec = CheckMachineNum(machineNum);
    if (EC_SUC != ec)
        return querySealInfo_str_;

    // 设备编号正确, 读槽位号与印章ID
    // 分两次读出
    int ret1, ret2;
    unsigned char len = 0;
    unsigned char data[] = { 0 };
    ret1 = ReadStamperMem(
        SLOT_STAMPER_WRITTEN_OFFSET,
        SLOT_STAMPER_WRITTEN_SIZE,
        data,
        len);

    int read_expected = (ret1 == 0 ? data[0] : 90);
    char* str = querySealInfo_str_;
    ret1 = ReadStamperMem(
        SLOT_STAMPER_OFFSET,
        read_expected > 45 ? 45 : read_expected,
        (unsigned char*)str,
        len);

    if (read_expected > 45) {
        ret2 = ReadStamperMem(
            SLOT_STAMPER_OFFSET + 46,
            read_expected - 45,
            (unsigned char*)(str + 45),
            len);
    }

    WriteLog(4, "querySealInfo->OUT, sealInfo: %s",
        str);
    return str;
}

char* PSBCSTDZDeviceSTAMPDeviceApp::querySloatInfo(const char* machineNum)
{
    if (querySloatInfo_str_ == NULL)
        querySloatInfo_str_ = new char[25];

    memset(querySloatInfo_str_, 0x0, 25);

    if (NULL == machineNum) {
        WriteLog(3, "querySloatInfo->参数为空");
        return querySloatInfo_str_;
    }

    WriteLog(4, "querySloatInfo->IN, (machineNum) = (%s)",
        machineNum);

    ExitIfInMaintain();

    // 格式: 槽位号:印章是否存在;槽位号:印章是否存在 (0 表示无章,1 表示有章)...
    // 例(1:0; 2:1; 3:0...)
    
    // 校验设备编号
    ErrorCode ec = CheckMachineNum(machineNum);
    if (EC_SUC != ec)
        return querySloatInfo_str_;

    // 若设备编号匹配
    char* seal_info = querySealInfo(machineNum);
    if (strlen(seal_info) == 0) {
        WriteLog(3, "querySloatInfo->获取印章信息失败");
        return querySloatInfo_str_;
    }

    std::map<int, std::string> slot_stamper;
    if (!ParseSlotAndStamperID(seal_info, slot_stamper)) {
        WriteLog(3, "querySloatInfo->有重复的章槽号");
        return querySloatInfo_str_;
    }

    char* str = querySloatInfo_str_;
    for (int i = 0; i < 6; ++i) {
        std::map<int, std::string>::iterator it = slot_stamper.find(i + 1);
        sprintf(str + i * 4, "%d:%d;", i + 1, atoi(it->second.c_str()) == 0 ? 0 : 1);
    }

    WriteLog(4, "querySloatInfo->OUT, sloatInfo: %s",
        str);
    return str;
}

int PSBCSTDZDeviceSTAMPDeviceApp::initStamp(
    const char* machineNum, 
    const char* slotNumAndStampId)
{
    // 参数检查
    if (NULL == machineNum || NULL == slotNumAndStampId) {
        WriteLog(3, "initStamp->参数为空");
        return EC_INVALID_PARAMETER;
    }

    WriteLog(4, "initStamp->IN, (machineNum, slotNumAndStampId) = (%s, %s)",
        machineNum,
        slotNumAndStampId);

    // 槽位号与印章ID循环串
    // 例(1:00000000001a; 2:00000000002b; 3:0...), 长度不大于15*卡槽数量

    std::map<int, std::string> slot_stamper;
    if (!ParseSlotAndStamperID(slotNumAndStampId, slot_stamper)) {
        WriteLog(3, "initStamp->有重复的章槽号");
        return EC_DUP_SLOT_NUM;
    }

    std::map<int, std::string>::iterator it = slot_stamper.begin();
    for (; it != slot_stamper.end(); ++it) {
        // 判断章槽号是否合法
        if (it->first < 1 || it->first > 6) {
            WriteLog(3, "initStamp->有非法章槽号: %d", it->first);
            return EC_INVALID_PARAMETER;
        }

        // 判断每个章槽号对应的印章信息是否合法
        if (it->second.length() > 12) {
            WriteLog(3, "initStamp->有非法章信息(%d, %s)", 
                it->first,
                it->second.c_str());
            return EC_INVALID_PARAMETER;
        }
    }

    // 参数合法
    ExitIfInMaintain();

    // 校验设备编号
    ErrorCode ec = CheckMachineNum(machineNum);
    if (EC_SUC != ec)
        return ec;

    // 设备编号匹配, 则将数据写到印控机存储区
    int ret1, ret2;
    // 数据过长, 分两次写入
    if (strlen(slotNumAndStampId) > 45) {
        ret1 = WriteIntoStamper(
            SLOT_STAMPER_OFFSET,
            (unsigned char*)slotNumAndStampId,
            45);

        ret2 = WriteIntoStamper(
            SLOT_STAMPER_OFFSET + 46,
            (unsigned char*)(slotNumAndStampId + 45),
            strlen(slotNumAndStampId) - 45 + 1);
        if (EC_SUC == ret1 && EC_SUC == ret2) {
            unsigned char len = strlen(slotNumAndStampId);
            WriteIntoStamper(
                SLOT_STAMPER_WRITTEN_OFFSET,
                &len,
                SLOT_STAMPER_WRITTEN_SIZE);
            return EC_SUC;
        } else {
            WriteLog(3, "initStamp->写章信息失败");
            return EC_FAIL;
        }
    }

    // 不超过45字节可一次性写入
    ret1 = WriteIntoStamper(
        SLOT_STAMPER_OFFSET,
        (unsigned char*)slotNumAndStampId,
        strlen(slotNumAndStampId) + 1);
    if (0 != ret1)
        return EC_API_FAIL;

    unsigned char len = strlen(slotNumAndStampId);
    WriteIntoStamper(
        SLOT_STAMPER_WRITTEN_OFFSET,
        &len,
        SLOT_STAMPER_WRITTEN_SIZE);

    WriteLog(4, "initStamp->OUT, (machineNum, slotNumAndStampId) = (%s, %s)",
        machineNum,
        slotNumAndStampId);
    return EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::setAcrossPageSeal(void)
{
    WriteLog(4, "setAcrossPageSeal->IN");

    ExitIfInMaintain();

    across_page_seal_called_ = true;
    WriteLog(4, "setAcrossPageSeal->OUT");
    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::setMachineNum(const char* machineNum)
{
    if (NULL == machineNum) {
        WriteLog(3, "setMachineNum->参数为空");
        return EC_INVALID_PARAMETER;
    }

    WriteLog(4, "setMachineNum->IN, (machineNum) = (%s)", machineNum);

    ExitIfInMaintain();

    // 参数检查, 最多20位字符串
    if (strlen(machineNum) > 20) {
        WriteLog(3, "setMachineNum->参数超过指定长度(%d)", 20);
        return EC_INVALID_PARAMETER;
    }

    int ret = WriteStamperIdentifier(
        (unsigned char*)machineNum, 
        strlen(machineNum) + 1); // 把0结尾字符写下去
    if (0 != ret) {
        WriteLog(3, "setMachineNum->调用驱动接口失败, er: %d", ret);
        return EC_API_FAIL;
    }

    WriteLog(4, "setMachineNum->OUT, (machineNum) = (%s)", machineNum);
    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::openPaperDoor(void)
{
    WriteLog(4, "openPaperDoor->IN");

    ExitIfInMaintain();

    unsigned char status[24] = { 0 };
    while (true) {
        int ret = FGetDevStatus(status, 24);
        if (0 != ret) {
            WriteLog(3, "openPaperDoor->检测章系统失败, er: %d", ret);
            break;
        }

        // 不是盖章过程中, 退出
        if (status[1] != 6)
            break;
    }
    
    // 关闭报警器
    SetAlarm(0, 0);
    SetAlarm(1, 0);
    
    int ret = FOpenDoorPaper();
    if (0 != ret) {
        WriteLog(3, "openPaperDoor->调用驱动接口失败, er: %d", ret);
        return EC_API_FAIL;
    }

    WriteLog(4, "openPaperDoor->OUT");
    return 0;
}

char* PSBCSTDZDeviceSTAMPDeviceApp::geterrMsg(int errNo)
{
    WriteLog(4, "geterrMsg->IN, (errNo) = (%d)", errNo);

    ExitIfInMaintain();

    if (errNo >= EC_MAX || errNo < 0)
        return "未定义的错误码";
    
    WriteLog(4, "geterrMsg->OUT, (errNo) = (%d)", errNo);
    return (char*)ec_des[errNo].c_str();
}

// 切图像素坐标, pointX, pointY
int PSBCSTDZDeviceSTAMPDeviceApp::checkManualPrintPara(int pointX, int pointY, int angle)
{
    WriteLog(4, "checkManualPrintPara->IN, (pointX, pointY, angle)"
        " = (%d, %d, %d)",
        pointX,
        pointY,
        angle);

    ExitIfInMaintain();

    // 坐标转换
    int ori_x = -1;
    int ori_y = -1;
    int ori_angle = 0;
    int ret = SearchImgStampPointForPSBC(
        src_image_.c_str(),      // 原图文件
        pointX,                  // 切图用印点
        pointY,
        angle,
        ori_x,
        ori_y,
        ori_angle);
    if (0 != ret) {
        WriteLog(3, "checkManualPrintPara->切图转原图坐标失败");
        return EC_CUT_TO_ORI_FAIL;
    }

    ret = FOpenDev(NULL);
    if (0 != ret)
        WriteLog(3, "checkManualPrintPara->打开设备失败");

    WriteLog(4, "checkManualPrintPara->切图转原图坐标(%d, %d)像素成功",
        ori_x,
        ori_y);

    // 将原图像素坐标转换到物理坐标
    struct Point* pt = GetSealCoord(ori_x, ori_y);
    WriteLog(3, "checkManualPrintPara->原图:(%d, %d)转物理坐标:(%d, %d)",
        ori_x,
        ori_y,
        pt->x,
        pt->y);

    WriteLog(4, "checkManualPrintPara->OUT, (pointX, pointY, angle)"
        " = (%d, %d, %d)",
        pointX,
        pointY,
        angle);
    delete pt;
    return EC_SUC;
}

// 盖章(是否蘸印油)
// pointX, pointY   --- 用印切图像素坐标
int PSBCSTDZDeviceSTAMPDeviceApp::ManualStart(
    int printNum,
    int pointX,
    int pointY,
    int angle,
    int ink)        // 是否蘸印油
{
    WriteLog(4, "ManualStart->IN");

    // 用印参数检查
    if (printNum <= 0 || printNum > 6 || angle < 0 || angle > 360 || pointX < 0 || pointY < 0) {
        WriteLog(3, "ManualStart->参数非法");
        return EC_INVALID_PARAMETER;
    }

    ExitIfInMaintain();

    // 进纸门状态
    if (1 == checkPaperDoorState()) {
        WriteLog(4, "ManualStart->进纸门未关闭");
        return EC_PAPER_OPEN;
    }

    // 安全门状态
    if (1 == checkBackDoorState()) {
        WriteLog(4, "ManualStart->安全门未关闭");
        return EC_SAFE_OPEN;
    }

    // 章槽号是否有效
    unsigned int rfids[7] = { 0 };
    unsigned char stampers = 0;
    if (0 == ReadAllRFID(rfids, 7, &stampers)) {
        if (rfids[printNum - 1] == 0) {
            WriteLog(3, "ManualStart->%d号章槽无章", printNum);
            return EC_NO_SEAL;
        }
    }

    // 打印原图路径
    WriteLog(4, "ManualStart->凭证原图路径: %s, 寻找原图坐标",
        src_image_.c_str());

    // 坐标转换
    int ori_x = -1;
    int ori_y = -1;
    int ori_angle = 0;
    int ret = SearchImgStampPointForPSBC(
        src_image_.c_str(),      // 原图文件
        pointX,                  // 切图用印点
        pointY,
        angle,
        ori_x,
        ori_y,
        ori_angle);
    if (0 != ret) {
        WriteLog(3, "ManualStart->切图转原图坐标失败");
        return EC_CUT_TO_ORI_FAIL;
    }

    ret = FOpenDev(NULL);
    if (0 != ret)
        WriteLog(3, "ManualStart->打开设备失败");

    WriteLog(4, "ManualStart->切图(%d, %d)转为原图坐标(%d, %d)像素成功",
        pointX,
        pointY,
        ori_x,
        ori_y);

    struct Point* pt = GetSealCoord(ori_x, ori_y);
    WriteLog(4, "ManualStart->原图转物理坐标:(%d, %d)毫米成功, 方式: %s",
        pt->x,
        pt->y,
        across_page_seal_called_ ? "盖骑缝章" : "盖普通章");

    StampPara para;
    para.stamp_idx = printNum;
    ret = GetStamperID(para.stamp_idx - 1, para.rfid);
    if (0 != ret) {
        WriteLog(3, "ManualStart->获取印章(%d)的rfid失败", para.stamp_idx);
        return EC_GET_RFID_FAIL;
    }

    STAMPERPARAM pa;
    memcpy(&pa.seal, &para.rfid, 4);
    std::srand((unsigned int)std::time(0));
    unsigned int serial = std::rand();
    pa.serial_number = serial;
    pa.isPadInk = ink;
    pa.x_point = pt->x;
    pa.y_point = pt->y;
    para.wait = PSBCConfig::GetInst()->wait_time_;
    pa.w_time = para.wait;
    pa.type = across_page_seal_called_ ? 1 : 0;
    across_page_seal_called_ = false;

    // 先关闭震动报警
    ret = SetAlarm(1, 0);
    if (0 != ret)
        WriteLog(3, "ManualStart->关震动报警失败");

    // 蜂鸣一声提示盖章
    FBeepCtrl(2, 1);
    FBeepCtrl(0, 0);
    Sleep(500);
    ret = FStartStamperstrc(&pa);
    if (0 != ret) {
        WriteLog(3, "ManualStart->发起盖章失败, er: %d", ret);
        delete pt;
        return EC_FAIL;
    }

    // WAIT_STAMP_COMPLETION 秒内盖章完成
    DWORD finish = WaitForSingleObject(notify_finish, WAIT_STAMP_COMPLETION * 1000);
    switch (finish) {
    case WAIT_OBJECT_0: {
       ResetEvent(notify_finish);
       ret = 0;
       WriteLog(4, "ManualStart->等待盖章完成成功");
    }
        break;
    case WAIT_TIMEOUT: {
       ret = 0;
       WriteLog(4, "ManualStart->等待盖章完成超时");
    }
        break;
    default: {
       ret = 0;
       WriteLog(4, "ManualStart->等待盖章完成错误");
    }
        break;
    }

    WriteLog(4, "ManualStart->OUT");
    delete pt;
    return ret;
}

// printNum         --- 章槽号(1~6)
// pointX, pointY   --- 像素坐标, 基于图片左上角
int PSBCSTDZDeviceSTAMPDeviceApp::manualPrintStart(
    int printNum, 
    int pointX, 
    int pointY, 
    int angle)
{
    WriteLog(4, "manualPrintStart->IN, (printNum, pointX, pointY, "
        "angle) = (%d, %d, %d, %d)",
        printNum,
        pointX,
        pointY,
        angle);

    return ManualStart(printNum, pointX, pointY, angle, 1);
}

// 光敏章用印(不蘸印油)
// pointX, pointY   --- 像素坐标
int PSBCSTDZDeviceSTAMPDeviceApp::manualPrintStartByLight(
    int printNum, 
    int pointX, 
    int pointY, 
    int angle)
{
    WriteLog(4, "manualPrintStartByLight->IN, (printNum, pointX,"
        " pointY, angle) = (%d, %d, %d, %d)",
        printNum,
        pointX,
        pointY,
        angle);

    return ManualStart(printNum, pointX, pointY, angle, 0);
}

// 自动用印
// pointX, pointY   --- 用印物理坐标, 单位毫米
int PSBCSTDZDeviceSTAMPDeviceApp::autoPrintStart(
    int printNum, 
    int pointX, 
    int pointY, 
    int angle)
{
    WriteLog(4, "autoPrintStart->IN, (printNum, pointX,"
        " pointY, angle) = (%d, %d, %d, %d)",
        printNum,
        pointX,
        pointY,
        angle);

    // 毫米转像素
    int x_in_pixel = 0;
    int ret = ConvertMM2Dpi(
        pointX,
        PSBCConfig::GetInst()->dpi_,
        x_in_pixel);
    if (0 != ret) {
        WriteLog(3, "autoPrintStart->x物理坐标(%d)转像素坐标失败, er: %d",
            pointX,
            ret);
        return EC_FAIL;
    }

    int y_in_pixel = 0;
    ret = ConvertMM2Dpi(
        pointY,
        PSBCConfig::GetInst()->dpi_,
        y_in_pixel);
    if (0 != ret) {
        WriteLog(3, "autoPrintStart->y物理坐标(%d)转像素坐标失败, er: %d",
            pointY,
            ret);
        return EC_FAIL;
    }

    WriteLog(4, "autoPrintStart->物理坐标(%d, %d)转像素坐标值(%d, %d)成功",
        pointX,
        pointY,
        x_in_pixel,
        y_in_pixel);

    ret = ManualStart(printNum, x_in_pixel, y_in_pixel, angle, 1);
    WriteLog(4, "autoPrintStart->OUT, (printNum, pointX,"
        " pointY, angle) = (%d, %d, %d, %d)",
        printNum,
        pointX,
        pointY,
        angle);
    return ret;
}

int PSBCSTDZDeviceSTAMPDeviceApp::openMachineBackDoor(void)
{
    WriteLog(4, "openMachineBackDoor->IN");

    ExitIfInMaintain();

    // 首先关闭震动报警
    SetAlarm(0, 0);
    SetAlarm(1, 0);

    // 进入维护模式
    int ret = FMaintenanceMode();
    if (0 != ret) {
        WriteLog(3, "openMachineBackDoor->进入维护模式失败, er: %d", ret);
        return EC_INTO_MAINTAIN_FAIL;
    }

    // 当前是“维护模式”
    ret = FOpenDoorSafe();  // 打开电子锁
    if (0 != ret) {
        WriteLog(3, "openMachineBackDoor->打开电子锁失败, er: %d", ret);
        return EC_QUIT_MAINTAIN_FAIL;
    }

    WriteLog(4, "openMachineBackDoor->OUT");
    return EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::openMachineBackDoorUnNormal(const char* openInfo)
{
    if (NULL == openInfo) {
        WriteLog(3, "openMachineBackDoorUnNormal->参数为空");
        return EC_INVALID_PARAMETER;
    }

    WriteLog(4, "openMachineBackDoorUnNormal->IN, (openInfo) = (%s)",
        openInfo);

    // 异常开锁信息最多15个字符, 不包括0结尾字符
    if (strlen(openInfo) > 15) {
        WriteLog(3, "openMachineBackDoorUnNormal->参数长度超过规定的最大长度%d", 15);
        return EC_INVALID_PARAMETER;
    }

    unsigned char data[2] = { 0 };
    unsigned char len = 0;
    // 已记录异常开锁信息条数
    int ret = ReadStamperMem(
        OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET,
        OPEN_SAFE_DOOR_EXCEPTION_MARK_SIZE,
        data,
        len);
    if (0 != ret)
        return EC_API_FAIL;

    // 判断是否已经写最大异常开锁记录
    int recorded = strtol((char*)&data[0], NULL, 16);
    if (recorded > MAX_EXCEPTION_OPEN_COUNT - 1) {
        WriteLog(3, "openMachineBackDoorUnNormal->已经写最大异常开锁记录");
        return EC_UPTO_MAX_EXCEPTION;
    }

    // 参数正确并可以写入
    ExitIfInMaintain();

    ret = openMachineBackDoor();
    if (EC_SUC != ret) {
        WriteLog(3, "openMachineBackDoorUnNormal->打开安全门失败, er: %d",
            ret);
        return ret;
    }
    
    char open_info[17] = { 0 };
    strcpy(open_info, openInfo);
    strcat(open_info, ";");
    ret = WriteIntoStamper(
        OPEN_SAFE_DOOR_EXCEPTION_OFFSET + recorded * OPEN_SAFE_DOOR_EXCEPTION_SIZE,
        (unsigned char*)open_info,
        strlen(open_info));
    if (0 != ret)
        return ret;

    // 写异常开锁信息成功, 更新记录异常开锁信息条目
    data[0] += 1;
    ret = WriteIntoStamper(
        OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET,
        data,
        1);

    WriteLog(4, "openMachineBackDoorUnNormal->OUT, (openInfo) = (%s)",
        openInfo);
    return ret;
}

char* PSBCSTDZDeviceSTAMPDeviceApp::getMachineNum(void)
{
    WriteLog(4, "getMachineNum->IN");

    ExitIfInMaintain();

    if (getMachineNum_str_ == NULL) {
        getMachineNum_str_ = new unsigned char[21];
    }

    memset(getMachineNum_str_, 0x0, 21);

    int ret = ReadStamperIdentifier(getMachineNum_str_, 21);
    if (0 != ret) {
        WriteLog(3, "getMachineNum->ReadStamperIdentifier失败, er: %d",
            ret);
        return (char*)getMachineNum_str_;
    }

    WriteLog(4, "getMachineNum->OUT");
    return (char*)getMachineNum_str_;
}

char* PSBCSTDZDeviceSTAMPDeviceApp::getMachineType(void)
{
    WriteLog(4, "getMachineType->IN");

    ExitIfInMaintain();

    if (getMachineType_str_ == NULL) {
        getMachineType_str_ = new char[512];
    }

    memset(getMachineType_str_, 0x0, 512);

    unsigned char sn[49] = { 0 };
    int ret = ReadBackupSN(sn, 48);
    if (0 != ret) {
        WriteLog(3, "getMachineType->ReadBackupSN失败, er: %d", ret);
        sprintf(getMachineType_str_, "-@#%d", ret);
        return getMachineType_str_;
    }

    strcpy(getMachineType_str_, (char*)sn);
    return getMachineType_str_;
}

int PSBCSTDZDeviceSTAMPDeviceApp::checkPaperDoorState(void)
{
    WriteLog(4, "checkPaperDoorState->IN");

    ExitIfInMaintain();

    // 获取门状态(len = 4)
    // P1:   推纸门状态  0 关闭，1 开启， 2检测错误
    // P2:   电子锁状态，同上
    // P3:   机械锁状态，同上
    // P4:   顶盖状态，同上

    char doors[4] = { 0 };
    int ret = FGetDoorsPresent(doors, sizeof(doors));
    if (0 != ret)
        return EC_API_FAIL;

    return doors[0] == 0 ? 2 : 1;
}

int PSBCSTDZDeviceSTAMPDeviceApp::checkBackDoorState(void)
{
    WriteLog(4, "checkBackDoorState->IN");

    // 该接口不能调用ExitIfInMaintain(如果先调用开安全门接口, 则会被自动关闭, 那么判断安全门状态会出错).

    char doors[4] = { 0 };
    int ret = FGetDoorsPresent(doors, sizeof(doors));
    if (0 != ret)
        return EC_API_FAIL;

    // 以电子锁为判断依据
    return doors[1] == 0 ? 2 : 1;
}

int PSBCSTDZDeviceSTAMPDeviceApp::lockPrinter(void)
{
    WriteLog(4, "lockPrinter->IN");

    ExitIfInMaintain();

    return Lock();
}

int PSBCSTDZDeviceSTAMPDeviceApp::unLockPrinter(void)
{
    WriteLog(4, "unLockPrinter->IN");
    ExitIfInMaintain();

    return Unlock();
}

int PSBCSTDZDeviceSTAMPDeviceApp::checkLockState(void)
{
    WriteLog(4, "checkLockState->IN");
    ExitIfInMaintain();

    return IsLocked() ? 0 : -1;
}

// 打开摄像头照明灯
int PSBCSTDZDeviceSTAMPDeviceApp::openVideoCapLight(void)
{
    WriteLog(4, "openVideoCapLight->IN");

    ExitIfInMaintain();

    // 补光灯控制
    // light     --- 补光灯类型
    //              1 -- 安全门旁边的补光灯; 
    //              2 -- 凭证摄像头旁边的补光灯
    // op        --- 操作(0 -- 关; 1 -- 开)

    // 补光灯亮度调节
    // light         --- 补光灯类型
    //                  1 安全门旁边的补光灯
    //                  2 凭证摄像头旁边的补光灯
    // brightness    --- 亮度值(建议范围 1-100, 1为最弱, 100为最亮)
    // int FLightBrightness(char light, char brightness);
    
    int ret = FLightCtrl(2, 1);
    if (0 != ret) {
        WriteLog(3, "openVideoCapLight->补光灯打开失败, er: %d", ret);
        return EC_OPEN_CAMERA_LED_FAIL;
    }

    // 打开摄像头后需要设置亮度
    return 0 != FLightBrightness(2, PSBCConfig::GetInst()->brightness_) ? EC_ADJUST_LED_FAIL : EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::closeVideoCapLight(void)
{
    WriteLog(4, "closeVideoCapLight->IN");

    ExitIfInMaintain();
    return 0 != FLightCtrl(2, 0) ? EC_CLOSE_CAMERA_LED_FAIL : EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::connMachine(const char* seriaID)
{
    if (NULL == seriaID) {
        WriteLog(3, "connMachine->参数为空");
        return EC_INVALID_PARAMETER;
    }

    WriteLog(4, "connMachine->IN, (seriaID) = (%s)", seriaID);

    ExitIfInMaintain();
    int ret = FOpenDev(NULL);
    if (0 != ret) {
        WriteLog(3, "connMachine->打开设备失败, er: %d", ret);
        return EC_OPEN_FAIL;
    }

    connected_ = true;

//     // 将序列号写入印控机
//     ret = WriteBackupSN((unsigned char*)seriaID, strlen(seriaID));
//     if (0 != ret) {
//         WriteLog(3, "PSBCSTDZDeviceSTAMPDeviceApp::connMachine->写序列号失败, er: %d", ret);
//         return EC_API_FAIL;
//     }

    return EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::disconnMachine(void)
{
    WriteLog(4, "disconnMachine->IN");

    ExitIfInMaintain();
    connected_ = false;
    return FCloseDev();
}

int PSBCSTDZDeviceSTAMPDeviceApp::isConnMachine(void)
{
    WriteLog(4, "isConnMachine->IN");

    ExitIfInMaintain();
    return connected_ ? 0 : -1;
}

char* PSBCSTDZDeviceSTAMPDeviceApp::readOpenBackDoorExceptionInfo(void)
{
    WriteLog(4, "readOpenBackDoorExceptionInfo->IN");

    ExitIfInMaintain();
    if (NULL == readOpenBackDoorExceptionInfo_str_)
        readOpenBackDoorExceptionInfo_str_ =
            new unsigned char[OPEN_SAFE_DOOR_EXCEPTION_SIZE * MAX_EXCEPTION_OPEN_COUNT + 1];

    memset(
        readOpenBackDoorExceptionInfo_str_,
        0x0,
        OPEN_SAFE_DOOR_EXCEPTION_SIZE * MAX_EXCEPTION_OPEN_COUNT + 1);

    // 读已记录条数
    unsigned char len = 0;
    unsigned char mark[2] = { 0 };
    int ret = ReadStamperMem(
        OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET,
        OPEN_SAFE_DOOR_EXCEPTION_MARK_SIZE,
        mark,
        len);
    if (0 != ret)
        return (char*)readOpenBackDoorExceptionInfo_str_;

    // 进制转换
    int recorded = (int)mark[0];

    // 如果异常记录已经清空
    if (0 == recorded) {
        WriteLog(3, "readOpenBackDoorExceptionInfo->异常记录信息已被清空");
        return (char*)readOpenBackDoorExceptionInfo_str_;
    }

    unsigned char* data = readOpenBackDoorExceptionInfo_str_;
    for (int i = 0; i < recorded; ++i) {
        unsigned char tmp[17] = { 0 };
        ret = ReadStamperMem(
            OPEN_SAFE_DOOR_EXCEPTION_OFFSET + i * OPEN_SAFE_DOOR_EXCEPTION_SIZE,
            OPEN_SAFE_DOOR_EXCEPTION_SIZE,
            tmp,
            len);
        if (0 == ret) {
            strcat((char*)data, (char*)tmp);
        }
    }

    return (char*)data;
}

int PSBCSTDZDeviceSTAMPDeviceApp::delOpenBackDoorExceptionInfo(void)
{
    WriteLog(4, "delOpenBackDoorExceptionInfo->IN");

    ExitIfInMaintain();
    char data[] = { 0x30 };
    int ret = WriteIntoStamper(
        OPEN_SAFE_DOOR_EXCEPTION_MARK_OFFSET,
        (unsigned char*)data,
        1);

    unsigned char buf[30] = { 0 };
    ret = WriteIntoStamper(
        OPEN_SAFE_DOOR_EXCEPTION_OFFSET,
        buf,
        24);

    return ret;
}

//////////////////////////// 摄像头API //////////////////////////////////

// 打开摄像头
int PSBCSTDZDeviceSTAMPDeviceApp::openVideoCap(void)
{
    WriteLog(4, "openVideoCap->IN");

    if (video_opened_)
        return 0;

    int ret1 = OpenCamera(PAPERCAMERA);
    if (0 != ret1)
        WriteLog(3, "openVideoCap->打开凭证摄像头失败, er: %d", 
            ret1);

    int ret2 = SetResolution(
        PAPERCAMERA,
        PSBCConfig::GetInst()->resolution_width_,
        PSBCConfig::GetInst()->resolution_height_);
    if (0 != ret2)
        WriteLog(3, "openVideoCap->设置凭证摄像头分辨率失败, er: %d",
            ret2);

    if (0 == ret1 && 0 == ret2)
        video_opened_ = true;

    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::setVedioProperties(
    int _bright,
    int _contrast,
    int _hue,
    int _saturation,
    int _sharpness,
    int _whitebalance,
    int _gain)
{
    WriteLog(4, "setVedioProperties->IN");

    int set_bright = SetParameter(
        PAPERCAMERA, 
        brightness,
        _bright);

    int set_con = SetParameter(
        PAPERCAMERA,
        contrast,
        _contrast);

    int set_hue = SetParameter(
        PAPERCAMERA,
        hue,
        _hue);

    int set_saturation = SetParameter(
        PAPERCAMERA,
        saturation,
        _saturation);

    int set_sharpness = SetParameter(
        PAPERCAMERA,
        sharpness,
        _sharpness);

    int set_whilebalance = SetParameter(
        PAPERCAMERA,
        whitebalance,
        _whitebalance);

    int set_exposure = SetParameter(
        PAPERCAMERA,
        expouse,
        _gain);

    return 0 == (set_bright | set_con | set_hue | set_saturation | set_sharpness | 
        set_whilebalance | set_exposure) ? EC_SUC : EC_SET_CAMERA_PARAM_FAIL;
}

int getSystemTime(int& year, int& month, int& date)
{
    time_t timer;
    time(&timer);
    tm* t_tm = localtime(&timer);

    year = t_tm->tm_year + 1900;
    month = t_tm->tm_mon + 1;
    date = t_tm->tm_mday;
    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::getImageFormat(
    const char* filePath,   // 切图路径
    int type,
    int isEraseBorder)
{
    if (NULL == filePath) {
        WriteLog(3, "getImageFormat->路径为空");
        return EC_INVALID_PARAMETER;
    }

    if (type != 0 && type != 1 && type != 2) {
        WriteLog(3, "getImageFormat->图片类型参数(%d)非法", type);
        return EC_INVALID_PARAMETER;
    }

    if (isEraseBorder != 0 && isEraseBorder != 1) {
        WriteLog(3, "getImageFormat->图片处理参数(%d)非法", isEraseBorder);
        return EC_INVALID_PARAMETER;
    }

    WriteLog(4, "getImageFormat->IN, (filePaht, type, isEraseBorder)"
        " = (%s, %d, %d)",
        filePath,
        type,
        isEraseBorder);

    // 参数正确, 打开摄像头并打开摄像头补光灯
    openVideoCap();
    openVideoCapLight();

    // 根据参数type构造文件扩展名
    char extension[10] = { 0 };
    if (0 == type)
        sprintf_s(extension, ".%s", "bmp");
    else
        sprintf_s(extension, ".%s", "jpg");

    // 构造以时间为文件名
    int year = 0;
    int month = 0;
    int date = 0;
    getSystemTime(year, month, date);
    char file_name[32] = { 0 };
    sprintf_s(file_name, "%d%d%d", year, month, date);

    std::string working_path;
    GetMoudulePath(working_path);
    char file_path[MAX_PATH] = { 0 };
    sprintf_s(file_path, "%stmp\\%s%s", working_path.c_str(), file_name, extension);

	std::string src_path(file_path); // 源图路径名
    src_image_ = src_path;

	int ret = CapturePhoto(
		PAPERCAMERA, 
		CvtType(type), 
		(char*)src_path.c_str());
    if (0 != ret) {
        WriteLog(3, "getImageFormat->原图路径: %s, 拍照失败, er: %d", 
            src_path.c_str(),
            ret);
        return EC_CAPTURE_FAIL;
    }

    char dst_path[MAX_PATH] = { 0 };
    memcpy(dst_path, filePath, strlen(filePath));
    if (0 == type) {
        std::string dst(dst_path);
        std::size_t last_dot = dst.find_last_of(".");
        if (last_dot != std::string::npos) {
            std::string sub_dst = dst.substr(0, last_dot + 1); // 包括"."字符
            char jpeg_copy_bmp[MAX_PATH] = { 0 };
            sprintf_s(jpeg_copy_bmp, "%s%s", sub_dst.c_str(), "jpg");
            WriteLog(4, "getImageFormat->bmp图片: %s ,的同名jpg图片: %s", jpeg_copy_bmp);

            CutImgEdgeEx(src_path.c_str(), jpeg_copy_bmp);
        }
    }

    // 图片不进行处理, 只是复制图片
    if (isEraseBorder == 0) {
        CopyFile(src_path, dst_path);
    } else {
        // 纠偏去黑边
        ret = CutImgEdgeEx(src_path.c_str(), dst_path);
        if (0 != ret) {
            WriteLog(3, "getImageFormat->纠偏去黑边失败, er: %d", ret);
            return EC_IMG_PROCESS_FAIL;
        }
    }

	return EC_SUC;
}

int PSBCSTDZDeviceSTAMPDeviceApp::revolveImg(
    const char*     filePath, 
    const char*     targetPath, 
    int             angle)
{
    WriteLog(4, "revolveImg->IN");

    return RotateImg(filePath, angle, targetPath);
}

// 关闭凭证摄像头
int PSBCSTDZDeviceSTAMPDeviceApp::closeVideoCap(void)
{
    WriteLog(4, "closeVideoCap->IN");

    video_opened_ = false;
    return 0;
}

int PSBCSTDZDeviceSTAMPDeviceApp::checkVideoState(void)
{
    WriteLog(4, "checkVideoState->IN");

    return video_opened_ ? 1 : 2;
}

//////////////////////////////////////////////////////////////////////////

// slotNumAndStampId    --- 形如: (1:00000000001a;2:00000000002b;3:0...)
bool PSBCSTDZDeviceSTAMPDeviceApp::ParseSlotAndStamperID(
    const char* slotNumAndStampId,
    std::map<int, std::string>& slot_stamper)
{
    char buf[128] = { 0 };
    strcpy(buf, slotNumAndStampId);
    char *token = std::strtok(buf, ";"); // strtok会改变buf
    while (token != NULL) {
        std::string str = token;
        size_t comma = str.find_first_of(":");
        if (comma != std::string::npos) {
            int slot = atoi(str.substr(0, comma + 1).c_str());
            std::string stamper = str.substr(comma + 1, std::string::npos);
            std::pair<std::map<int, std::string>::iterator, bool> ret = 
                slot_stamper.insert(std::make_pair(slot, stamper));
            if (!ret.second)
                return false;
        }

        token = std::strtok(NULL, ";");
    }

    return true;
}

// 将原图用印像素坐标转换为设备(印控机)用印坐标(毫米)
Point* PSBCSTDZDeviceSTAMPDeviceApp::GetSealCoord(int nX, int nY)
{
    int dminx = 3;          //2//6
    int dminy = 60;         //56//60
    int dmaxx = 260;        //270//270
    int dmaxy = 230;        // 250//239
    int w = dmaxx - dminx;  //--印章校准位置最大X-印章校准位置最小X 物理=239
    int h = dmaxy - dminy;  //--印章校准位置最大Y-印章校准位置最小Y 物理=182
    int x0 = nX;            //--原件盖章X坐标
    int y0 = nY;            //--原件盖章Y坐标

    const PSBCConfig* config = PSBCConfig::GetInst();
    int x1 = config->check_pt1_.x/*1929*/;              //--A4像素最小X坐标 像素 右上角坐标
    int y1 = config->check_pt1_.y/*139*/;              //--A4像素最小Y坐标 像素

    int x2 = config->check_pt3_.x/*296*/;              //--[===[A4像素最大X坐标 像素]===] 左下角坐标
    int y2 = config->check_pt3_.y/*1221*/;              //--[===[A4像素最大Y坐标 像素]===]
    WriteLog(4, "GetSealCoord->右上角(%d, %d), 左下角(%d, %d)",
        x1, y1,
        x2, y2);

    // 读取校准点数据
    //         std::vector<struct Point*> VerifyPoists;
    //         unsigned short buffer[10] = { 0 };
    //         int ret = CalibrationPoint(buffer, 10);
    //         if (ret == 0) {
    //             for (int i = 0; i < 5; ++i) {
    //                 struct Point* pt = new Point;
    //                 pt->x = buffer[i];
    //                 pt->y = buffer[i + 1];
    //                 VerifyPoists.push_back(pt);
    //             }
    //         } else {
    //             Log::WriteLog(LL_DEBUG, "未读取到校准点数据,使用系统默认值");
    //         }
    // 
    //         if (VerifyPoists.size() > 4 && VerifyPoists.at(1)->x != 65535 &&
    //             VerifyPoists.at(1)->y != 65535) {
    //             // 左上, 右上(1), 右下, 左下(3), 中间
    //             x1 = VerifyPoists[1]->x;
    //             y1 = VerifyPoists[1]->y;
    //             x2 = VerifyPoists[3]->x;
    //             y2 = VerifyPoists[3]->y;
    // 
    //             for (int i = 0; i < VerifyPoists.size(); ++i) {
    //                 delete VerifyPoists.at(i);
    //             }
    //         } else {
    //             x1 = MC::SvrConfig::GetInst()->check_pt2_.x;
    //             y1 = MC::SvrConfig::GetInst()->check_pt2_.y;
    //             x2 = MC::SvrConfig::GetInst()->check_pt4_.x;;
    //             y2 = MC::SvrConfig::GetInst()->check_pt4_.y;
    //         }

    // 比例计算
    double dRateX = (double)(fabs(float(x2 - x1)) * 1000) / (double)(fabs(float(dmaxx - dminx)) * 1000); // 7.85
    double dRateY = (double)(fabs(float(y2 - y1)) * 1000) / (double)(fabs(float(dmaxy - dminy)) * 1000); // 7.88
    double devX = (double)(fabs(float(x1 - nX)) * 1000) / (double)(dRateX * 1000) + dminx;
    double devY = (double)(fabs(float(y1 - nY)) * 1000) / (double)(dRateY * 1000) + dminy;
    int x = (int)ceil(devX);
    int y = (int)ceil(devY);

    bool bFix = false;
    if (x < 1) {
        x = 3;
        bFix = true;
    }
    if (x > 260) {
        x = 260;
        bFix = true;
    }
    if (y < 56) {
        y = 60;
        bFix = true;
    }
    if (y > 230) {
        y = 230;
        bFix = true;
    }

    if (bFix) {
        WriteLog(4, "修正设备用印位置, Device({2},{3})", nX, nY, x, y);
    }

    return new Point(x, y);
}

ErrorCode PSBCSTDZDeviceSTAMPDeviceApp::CheckMachineNum(const char* machine)
{
    unsigned char id[22] = { 0 };
    int ret = ReadStamperIdentifier(id, 21);
    if (0 != ret)
        return EC_API_FAIL;

    if (0 != strcmp((char*)id, machine)) {
        WriteLog(3, "CheckMachineNum->设备编号不匹配, 固有: %s, 传入: %s",
            id,
            machine);
        return EC_MACHINE_MISMATCH;
    }

    return EC_SUC;
}

void PSBCSTDZDeviceSTAMPDeviceApp::ExitIfInMaintain()
{
    if (!init_)
        FOpenDev(NULL);

    unsigned char status[24] = { 0 };
    int ret = FGetDevStatus(status, 24);
    if (0 != ret)
        return;

    // 如果是维护模式
    if (status[1] == 7) {
        SetStampMap();
        FCloseDoorSafe();   //  关电子锁
        FQuitMaintainMode();
    }
}

bool PSBCSTDZDeviceSTAMPDeviceApp::CopyFile(std::string strFrom,std::string strTo)
{
	int nFromLen = strFrom.length();
    int nToLen = strTo.length();
    char* pFrom = new char[nFromLen+2];
    char* pTo = new char[nToLen+2];
    strcpy(pFrom,strFrom.c_str());
    strcpy(pTo,strTo.c_str());
    pFrom[nFromLen+1]='\0';
    pTo[nToLen+1]='\0';

    SHFILEOPSTRUCT FileOp={0};
    FileOp.fFlags = FOF_NOCONFIRMATION|   //不出现确认对话框
        FOF_NOCONFIRMMKDIR ; //需要时直接创建一个文件夹,不需用户确定
    FileOp.pFrom = pFrom;
    FileOp.pTo = pTo;
    FileOp.wFunc = FO_COPY;
    bool bResult = (SHFileOperation(&FileOp) == 0);
    delete []pFrom;
    delete []pTo;
    return bResult;
}
