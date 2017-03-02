#ifndef MC_AGENT_API_SET_H_
#define MC_AGENT_API_SET_H_

#include <string>
#include <map>
#include "agent_cmd.h"

class QueryMachineNT {
public:
    virtual void Notify(std::string sn, int ec) = 0;
};

class SetMachineNT {
public:
    virtual void Notify(std::string sn, int ec) = 0;
};

class InitMachineNT {
public:
    virtual void Notify(std::string key, int ec) = 0;
};

class BindMACNT {
public:
    virtual void Notify(std::string mac, int ec) = 0;
};

class UnbindMACNT {
public:
    virtual void Notify(std::string mac, int ec) = 0;
};

class PrepareStampNT {
public:
    virtual void Notify(int num, int timeout, std::string task_id, int ec) = 0;
};

class QueryPaperNT {
public:
    virtual void Notify(int status, int ec) = 0;
};

class SnapshotNT {
public:
    virtual void Notify(int ori_dpi, int cut_dpi, 
        std::string ori_path, std::string cut_path, int ec) = 0;
};

// 合成照片
class MergePhotoNT {
public:
    virtual void Notify(std::string p1, std::string p2, std::string merged, int ec) = 0;
};

// 原图用印点查找
class SearchStampPointNT {
public:
    virtual void Notify(int x, int y, double angle, int ec) = 0;
};

// 坐标转换
class CvtCoordNT {
public:
    virtual void Notify(int x_dev, int y_dev, int ec) = 0;
};

class RecogModelPointNT {
public:
    virtual void Notify(std::string model, double angle, int x, int y, int ec) = 0;
};

// 版面、验证码识别
class RecognizeNT {
public:
    virtual void Notify(std::string path, std::string template_id, std::string trace, int ec) = 0;
};

// 要素识别
class IdentifyNT {
public:
    virtual void Notify(std::string path, int x, int y, int width, int height,
        std::string result, int ec) = 0;
};

// 普通用印
class OrdinaryStampNT {
public:
    virtual void Notify(std::string task, std::string voucher_type, int stamp_num, 
        int x, int y, int angle, int ec) = 0;
};

// 自动用印
class AutoStampNT {
public:
    virtual void Notify(std::string task, std::string voucher_type, int stamp_num, int ec) = 0;
};

// 结束用印
class FinishStampNT {
public:
    virtual void Notify(std::string task, int ec) = 0;
};

// 释放印控机
class ReleaseStampNT {
public:
    virtual void Notify(std::string machine, int ec) = 0;
};

// 获取错误信息
class GetErrorNT {
public:
    virtual void Notify(int er_code, std::string err_msg, std::string err_resolver, int ec) = 0;
};

// 校准印章
class CalibrationNT {
public:
    virtual void Notify(int slot, int ec) = 0;
};

// 查询印章信息
class QueryStampersNT {
public:
    virtual void Notify(char* status, int ec) = 0;
};

// 安全门状态
class QuerySafeNT {
public:
    virtual void Notify(int status, int ec) = 0;
};

// 开关安全门
class CtrlSafeNT {
public:
    virtual void Notify(int ctrl, int ec) = 0;
};

// 开关蜂鸣器
class CtrlBeepNT {
public:
    virtual void Notify(int ctrl, int ec) = 0;
};

class QuerySlotNT {
public:
    virtual void Notify(int num, int ec) = 0;
};

// 报警器状态查询
class QueryAlarmNT {
public:
    virtual void Notify(int door, int vibration, int ec) = 0;
};

// 开关报警器
class CtrlAlarmNT {
public:
    virtual void Notify(int alarm, int ctrl, int ec) = 0;
};

// 查询mac信息
class QueryMACNT {
public:
    virtual void Notify(std::string mac1, std::string mac2, int ec) = 0;
};

// 锁定
class LockNT {
public:
    virtual void Notify(int ec) = 0;
};

class UnlockNT {
public:
    virtual void Notify(int ec) = 0;
};

class QueryLockNT {
public:
    virtual void Notify(int lock, int ec) = 0;
};

class OpenCnnNT {
public:
    virtual void Notify(int ec) = 0;
};

class CloseCnnNT {
public:
    virtual void Notify(int ec) = 0;
};

class QueryCnnNT {
public:
    virtual void Notify(int cnn, int ec)  = 0;
};

class SideDoorAlarmNT {
public:
    virtual void Notify(int ec) = 0;
};

class DevModelNT {
public:
    virtual void Notify(std::string model, int ec) = 0;
};

class OpenPaperNT {
public:
    virtual void Notify(int ec) = 0;
};

class CtrlLedNT {
public:
    virtual void Notify(int ec)  = 0;
};

class CheckParamNT {
public:
    virtual void Notify(int ec) = 0;
};

class OpenCameraNT {
public:
    virtual void Notify(int ec) = 0;
};

class CloseCameraNT {
public:
    virtual void Notify(int ec) = 0;
};

class QueryCameraNT {
public:
    virtual void Notify(int which, int status, int ec) = 0;
};

class SetResolutionNT {
public:
    virtual void Notify(int ec) = 0;
};

class SetDPIValueNT {
public:
    virtual void Notify(int ec) = 0;
};

class SetPropertyNT {
public:
    virtual void Notify(int ec) = 0;
};

class RecordVideoNT {
public:
    virtual void Notify(int ec) = 0;
};

class StopRecordVideoNT {
public:
    virtual void Notify(int ec) = 0;
};

class GetRFIDNT {
public:
    virtual void Notify(int rfid, int ec) = 0;
};

class GetStatusNT {
public:
    virtual void Notify(int code, int ec) = 0;
};

// 心跳
class HeartNT {
public:
    virtual void Notify() {}
};

class WriteRatioNT {
public:
    virtual void Notify(int ec) = 0;
};

class ReadRatioNT {
public:
    virtual void Notify(float x, float y, int ec) = 0;
};

class WriteCaliNT {
public:
    virtual void Notify(int ec) = 0;
};

class ReadCaliNT {
public:
    virtual void Notify(unsigned short* pts, unsigned char len, int ec) = 0;
};

class QueryTopNT {
public:
    virtual void Notify(int status, int ec) = 0;
};

class EnterMaintainNT {
public:
    virtual void Notify(int ec) = 0;
};

class ExitMaintainNT {
public:
    virtual void Notify(int ec) = 0;
};

class StartPreviewNT {
public:
    virtual void Notify(int ec) = 0;
};

class StopPreviewNT {
public:
    virtual void Notify(int ec) = 0;
};

class CtrlFactoryNT {
public:
    virtual void Notify(int ec) = 0;
};

class ResetNT {
public:
    virtual void Notify(int ec) = 0;
};

class RestartNT {
public:
    virtual void Notify(int ec) = 0;
};

class AsynAPISet {
public:
    void DeleteNotify(void* nt);

public:
    int AsynQueryMachine(const QueryMachineNT* const nt);

    int AsynSetMachine(const std::string& sn, SetMachineNT* nt);

    int AsynInitMachine(const std::string& key, InitMachineNT* nt);

    int AsynBindMAC(const std::string& mac, BindMACNT* nt);

    int AsynUnbindMAC(const std::string& mac, UnbindMACNT* nt);

    int AsynPrepareStamp(int num, int timeout, PrepareStampNT* nt);

    int AsynQueryPaper(QueryPaperNT* nt);

    int AsynSnapshot(
        int which,
        int ori_dpi, 
        int cut_dpi, 
        const std::string& ori_path, 
        const std::string& cut_path,
        SnapshotNT* nt);

    int AsynMergePhoto(
        const std::string& p1, 
        const std::string& p2,
        const std::string& merged,
        MergePhotoNT* nt);

    int AsynSearchStampPoint(
        const std::string& img,
        int x,
        int y,
        double angle,
        SearchStampPointNT* nt);

    int AsynRecogModelPoint(
        const std::string& path,
        RecogModelPointNT* nt);

    int AsynRecognizeImage(
        const std::string& path, 
        RecognizeNT* nt);

    int AsynIdentifyElement(
        const std::string& path,
        int x,
        int y, 
        int width, 
        int height,
        IdentifyNT* nt);

    int AsynGetSealCoord(
        int x_img,
        int y_img,
        CvtCoordNT* nt);

    int AsynOrdinaryStamp(
        const std::string& task, 
        const std::string& voucher, 
        int num, 
        int ink,
        int x, 
        int y, 
        int angle, 
        int type,
        OrdinaryStampNT* nt);

    int AsynAutoStamp(const std::string& task,
        const std::string& voucher, int num, AutoStampNT* nt);

    int AsynFinishStamp(const std::string& task, FinishStampNT* nt);

    int AsynReleaseStamp(const std::string& machine, ReleaseStampNT* nt);

    int AsynGetError(int err_code, GetErrorNT* nt);

    int AsynCalibrate(int slot, CalibrationNT* nt);

    int AsynQueryStampers(QueryStampersNT* nt);

    int AsynQuerySafe(QuerySafeNT* nt);

    int AsynSafeControl(int ctrl, CtrlSafeNT* nt);

    int AsynBeepControl(int ctrl, int type, int interval, CtrlBeepNT* nt);

    int AsynQuerySlot(QuerySlotNT* nt);

    int AsynAlarmControl(int alarm, int ctrl, CtrlAlarmNT* nt);

    int AsynQueryAlarm(QueryAlarmNT* nt);

    int AsynQueryMAC(QueryMACNT* nt);

    int AsynLock(LockNT* nt);

    int AsynUnlock(UnlockNT* nt);

    int AsynQueryLock(QueryLockNT* nt);

    int AsynOpen(OpenCnnNT* nt);

    int AsynClose(CloseCnnNT* nt);

    int AsynQueryCnn(QueryCnnNT* nt);

    int AsynSetSideAlarm(int keep, int timeout, SideDoorAlarmNT* nt);

    int AsynQueryModel(DevModelNT* nt);

    int AsynOpenPaper(int timeout, OpenPaperNT* nt);

    int AsynCtrlLed(int which, int ctrl, int value, CtrlLedNT* nt);

    int AsynCheckParam(int x, int y, int angle, CheckParamNT* nt);

    int AsynOpenCamera(int which, OpenCameraNT* nt);

    int AsynCloseCamera(int which, CloseCameraNT* nt);

    int AsynQueryCamera(int which, QueryCameraNT* nt);

    int AsynSetResolution(int which, int x, int y, SetResolutionNT* nt);

    int AsynSetDPI(int which, int x, int y, SetDPIValueNT* nt);

    int AsynSetProperty(int which, SetPropertyNT* nt);

    int AsynStartRecordVideo(int which, const std::string& path, RecordVideoNT* nt);

    int AsynStopRecordVideo(int which, const std::string& path, StopRecordVideoNT* nt);

    int AsynGetRFID(int slot, GetRFIDNT* nt);

    int AsynGetStatus(GetStatusNT* nt);

    int AsynWriteRatio(float x, float y, WriteRatioNT* nt);

    int AsynReadRatio(ReadRatioNT* nt);

    int AsynWriteCali(unsigned short* pts, unsigned short len, WriteCaliNT* nt);

    int AsynReadCali(ReadCaliNT* nt);

    int AsynQueryTop(QueryTopNT* nt);

    int AsynEnterMain(EnterMaintainNT* nt);

    int AsynExitMain(ExitMaintainNT* nt);

    int AsynStartPreview(
        int which,
        int width,
        int height,
        int hwnd,
        StartPreviewNT* nt);

    int AsynStopPreview(
        int which,
        StopPreviewNT* nt);

    int AsynCtrlFactory(
        int ctrl,
        CtrlFactoryNT* nt);

    int AsynReset(ResetNT* nt);

    int AsynRestart(RestartNT* nt);

private:
    void AsynErrorNotify(BaseCmd* cmd, enum MC::ErrorCode ec);
    
    void ParseCmd(BaseCmd* cmd, char* chBuf);

    void* LookupSendTime(const std::string& send_time);

    void InsertNotify(const std::string& st, const void* const nt);

public:
    void HandleQueryMachine(char* chBuf);

    void HandleSetMachine(char* chBuf);

    void HandleInitMachine(char* chBuf);

    void HandleBindMac(char* chBuf);

    void HandleUnbindMAC(char* chBuf);

    void HandlePrepareStamp(char* chBuf);

    void HandleQueryPaper(char* chBuf);

    void HandleSnapshot(char* chBuf);

    void HandleMergePhoto(char* chBuf);

    void HandleSearchStamp(char* chBuf);

    void HandleRecogModelPoint(char* chBuf);

    void HandleRecognition(char* chBuf);

    void HandleIdentify(char* chBuf);

    void HandleOrdinary(char* chBuf);

    void HandleAuto(char* chBuf);

    void HandleFinish(char* chBuf);

    void HandleRelease(char* chBuf);

    void HandleGetError(char* chBuf);

    void HandleCalibrate(char* chBuf);

    void HandleQueryStampers(char* chBuf);

    void HandleQuerySafe(char* chBuf);

    void HandleSafeControl(char* chBuf);

    void HandleBeepControl(char* chBuf);

    void HandleQuerySlot(char* chBuf);

    void HandleAlarmControl(char* chBuf);

    void HandleQueryAlarm(char* chBuf);

    void HandleQueryMAC(char* chBuf);

    void HandleLock(char* chBuf);

    void HandleUnlock(char* chBuf);

    void HandleQueryLock(char* chBuf);

    void HandleOpenCnn(char* chBuf);

    void HandleCloseCnn(char* chBuf);

    void HandleQueryCnn(char* chBuf);

    void HandleSetSideAlarm(char* chBuf);

    void HandleGetModel(char* chBuf);

    void HandleOpenPaper(char* chBuf);

    void HandleCtrlLed(char* chBuf);

    void HandleCheckParam(char* chBuf);

    void HandleOpenCamera(char* chBuf);

    void HandleCloseCamera(char* chBuf);

    void HandleQueryCamera(char* chBuf);

    void HandleSetResolution(char* chBuf);

    void HandleSetDPI(char* chBuf);

    void HandleSetProperty(char* chBuf);

    void HandleRecordVideo(char* chBuf);

    void HandleStopRecordVideo(char* chBuf);

    void HandleGetRFID(char* chBuf);

    void HandleGetStatus(char* chBuf);

    void HandleCvtCoord(char* chBuf);

    void HandleWriteRatio(char* chBuf);

    void HandleReadRatio(char* chBuf);

    void HandleWriteCali(char* chBuf);

    void HandleReadCali(char* chBuf);

    void HandleQueryTop(char* chBuf);

    void HandleEnterMain(char* chBuf);

    void HandleExitMain(char* chBuf);

    void HandleStartPreview(char* chBuf);

    void HandleStopPreview(char* chBuf);

    void HandleFactoryCtrl(char* chBuf);

    void HandleReset(char* chBuf);

    void HandleRestart(char* chBuf);

private:
    std::map<std::string, void*> api_maps_;
    std::map<void*, std::string> nt_maps_;
};

#endif
