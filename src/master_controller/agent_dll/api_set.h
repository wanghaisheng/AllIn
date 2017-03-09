#ifndef MC_AGENT_API_SET_H_
#define MC_AGENT_API_SET_H_

#include <string>
#include <map>
#include <boost/thread/thread.hpp>
#include "agent_cmd.h"

#define CLEAN_FUNC_WAIT 30000   // ms, every 'CLEAN_FUNC_WAIT', the clean thread will free heap
#define LIFE_DURATION   25000   // ms, should be bigger than 'SYNC_IMAGE_WAIT'

class QueryMachineNT {
public:
    virtual ~QueryMachineNT() {}

    virtual void Notify(std::string sn, int ec) {}
};

class SetMachineNT {
public:
    virtual ~SetMachineNT() {}

    virtual void Notify(std::string sn, int ec) {}
};

class InitMachineNT {
public:
    virtual ~InitMachineNT() {}

    virtual void Notify(std::string key, int ec) {}
};

class BindMACNT {
public:
    virtual ~BindMACNT() {}

    virtual void Notify(std::string mac, int ec) {}
};

class UnbindMACNT {
public:
    virtual ~UnbindMACNT() {}

    virtual void Notify(std::string mac, int ec) {}
};

class PrepareStampNT {
public:
    virtual ~PrepareStampNT() {}

    virtual void Notify(int num, int timeout, std::string task_id, int ec) {}
};

class QueryPaperNT {
public:
    virtual ~QueryPaperNT() {}

    virtual void Notify(int status, int ec) {}
};

class SnapshotNT {
public:
    virtual ~SnapshotNT() {}

    virtual void Notify(int ori_dpi, int cut_dpi,
        std::string ori_path, std::string cut_path, int ec) {}
};

// 合成照片
class MergePhotoNT {
public:
    virtual ~MergePhotoNT() {}

    virtual void Notify(std::string p1, std::string p2, std::string merged, int ec) {}
};

// 原图用印点查找
class SearchStampPointNT {
public:
    virtual ~SearchStampPointNT() {}

    virtual void Notify(int x, int y, double angle, int ec) {}
};

// 坐标转换
class CvtCoordNT {
public:
    virtual ~CvtCoordNT() {}

    virtual void Notify(int x_dev, int y_dev, int ec) {}
};

class RecogModelPointNT {
public:
    virtual ~RecogModelPointNT() {}

    virtual void Notify(std::string model, double angle, int x, int y, int ec) {}
};

// 版面、验证码识别
class RecognizeNT {
public:
    virtual ~RecognizeNT() {}

    virtual void Notify(std::string path, std::string template_id, std::string trace, int ec) {}
};

// 要素识别
class IdentifyNT {
public:
    virtual ~IdentifyNT() {}
    virtual void Notify(std::string path, int x, int y, int width, int height,
        std::string result, int ec) {}
};

// 普通用印
class OrdinaryStampNT {
public:
    virtual ~OrdinaryStampNT() {}
    virtual void Notify(std::string task, std::string voucher_type, int stamp_num,
        int x, int y, int angle, int ec) {}
};

// 自动用印
class AutoStampNT {
public:
    virtual ~AutoStampNT() {}

    virtual void Notify(std::string task, std::string voucher_type, int stamp_num, int ec) {}
};

// 结束用印
class FinishStampNT {
public:
    virtual ~FinishStampNT() {}

    virtual void Notify(std::string task, int ec) {}
};

// 释放印控机
class ReleaseStampNT {
public:
    virtual ~ReleaseStampNT() {}

    virtual void Notify(std::string machine, int ec) {}
};

// 获取错误信息
class GetErrorNT {
public:
    virtual ~GetErrorNT() {}

    virtual void Notify(int er_code, std::string err_msg, std::string err_resolver, int ec) {}
};

// 校准印章
class CalibrationNT {
public:
    virtual ~CalibrationNT() {}

    virtual void Notify(int slot, int ec) {}
};

// 查询印章信息
class QueryStampersNT {
public:
    virtual ~QueryStampersNT() {}

    virtual void Notify(char* status, int ec) {}
};

// 安全门状态
class QuerySafeNT {
public:
    virtual ~QuerySafeNT() {}

    virtual void Notify(int status, int ec) {}
};

// 开关安全门
class CtrlSafeNT {
public:
    virtual ~CtrlSafeNT() {}

    virtual void Notify(int ctrl, int ec) {}
};

// 开关蜂鸣器
class CtrlBeepNT {
public:
    virtual ~CtrlBeepNT() {}

    virtual void Notify(int ctrl, int ec) {}
};

class QuerySlotNT {
public:
    virtual ~QuerySlotNT() {}

    virtual void Notify(int num, int ec) {}
};

// 报警器状态查询
class QueryAlarmNT {
public:
    virtual ~QueryAlarmNT() {}

    virtual void Notify(int door, int vibration, int ec) {}
};

// 开关报警器
class CtrlAlarmNT {
public:
    virtual ~CtrlAlarmNT() {}

    virtual void Notify(int alarm, int ctrl, int ec) {}
};

// 查询mac信息
class QueryMACNT {
public:
    virtual ~QueryMACNT() {}

    virtual void Notify(std::string mac1, std::string mac2, int ec) {}
};

// 锁定
class LockNT {
public:
    virtual ~LockNT() {}

    virtual void Notify(int ec) {}
};

class UnlockNT {
public:
    virtual ~UnlockNT() {}

    virtual void Notify(int ec) {}
};

class QueryLockNT {
public:
    virtual ~QueryLockNT() {}

    virtual void Notify(int lock, int ec) {}
};

class OpenCnnNT {
public:
    virtual ~OpenCnnNT() {}

    virtual void Notify(int ec) {}
};

class CloseCnnNT {
public:
    virtual ~CloseCnnNT() {}

    virtual void Notify(int ec) {}
};

class QueryCnnNT {
public:
    virtual ~QueryCnnNT() {}

    virtual void Notify(int cnn, int ec) {}
};

class SideDoorAlarmNT {
public:
    virtual ~SideDoorAlarmNT() {}

    virtual void Notify(int ec) {}
};

class DevModelNT {
public:
    virtual ~DevModelNT() {}

    virtual void Notify(std::string model, int ec) {}
};

class OpenPaperNT {
public:
    virtual ~OpenPaperNT() {}

    virtual void Notify(int ec) {}
};

class CtrlLedNT {
public:
    virtual ~CtrlLedNT() {}

    virtual void Notify(int ec) {}
};

class CheckParamNT {
public:
    virtual ~CheckParamNT() {}

    virtual void Notify(int ec) {}
};

class OpenCameraNT {
public:
    virtual ~OpenCameraNT() {}

    virtual void Notify(int ec) {}
};

class CloseCameraNT {
public:
    virtual ~CloseCameraNT() {}

    virtual void Notify(int ec) {}
};

class QueryCameraNT {
public:
    virtual ~QueryCameraNT() {}

    virtual void Notify(int which, int status, int ec) {}
};

class SetResolutionNT {
public:
    virtual ~SetResolutionNT() {}

    virtual void Notify(int ec) {}
};

class SetDPIValueNT {
public:
    virtual ~SetDPIValueNT() {}

    virtual void Notify(int ec) {}
};

class SetPropertyNT {
public:
    virtual ~SetPropertyNT() {}

    virtual void Notify(int ec) {}
};

class RecordVideoNT {
public:
    virtual ~RecordVideoNT() {}

    virtual void Notify(int ec) {}
};

class StopRecordVideoNT {
public:
    virtual ~StopRecordVideoNT() {}

    virtual void Notify(int ec) {}
};

class GetRFIDNT {
public:
    virtual ~GetRFIDNT() {}

    virtual void Notify(int rfid, int ec) {}
};

class GetStatusNT {
public:
    virtual ~GetStatusNT() {}

    virtual void Notify(int code, int ec) {}
};

// 心跳
class HeartNT {
public:
    virtual void Notify() {}
};

class WriteRatioNT {
public:
    virtual ~WriteRatioNT() {}

    virtual void Notify(int ec) {}
};

class ReadRatioNT {
public:
    virtual ~ReadRatioNT() {}

    virtual void Notify(float x, float y, int ec) {}
};

class WriteCaliNT {
public:
    virtual ~WriteCaliNT() {}

    virtual void Notify(int ec) {}
};

class ReadCaliNT {
public:
    virtual ~ReadCaliNT() {}

    virtual void Notify(
        unsigned short pts0, unsigned short pts1,
        unsigned short pts2, unsigned short pts3,
        unsigned short pts4, unsigned short pts5,
        unsigned short pts6, unsigned short pts7,
        unsigned short pts8, unsigned short pts9,
        int ec) {}
};

class QueryTopNT {
public:
    virtual ~QueryTopNT() {}

    virtual void Notify(int status, int ec) {}
};

class EnterMaintainNT {
public:
    virtual ~EnterMaintainNT() {}

    virtual void Notify(int ec) {}
};

class ExitMaintainNT {
public:
    virtual ~ExitMaintainNT() {}

    virtual void Notify(int ec) {}
};

class StartPreviewNT {
public:
    virtual ~StartPreviewNT() {}

    virtual void Notify(int ec) {}
};

class StopPreviewNT {
public:
    virtual ~StopPreviewNT() {}

    virtual void Notify(int ec) {}
};

class CtrlFactoryNT {
public:
    virtual ~CtrlFactoryNT() {}

    virtual void Notify(int ec) {}
};

class ResetNT {
public:
    virtual ~ResetNT() {}

    virtual void Notify(int ec) {}
};

class RestartNT {
public:
    virtual ~RestartNT() {}

    virtual void Notify(int ec) {}
};

class GetSystemNT {
public:
    virtual ~GetSystemNT() {}

    virtual void Notify(int status, int ec) {}
};

class WriteMainSpareNT {
public:
    virtual ~WriteMainSpareNT() {}

    virtual void Notify(std::string sn, int ec) {}
};

class ReadMainSpareNT {
public:
    virtual ~ReadMainSpareNT() {}

    virtual void Notify(std::string sn, int ec) {}
};

class RecogQRNT {
public:
    virtual ~RecogQRNT() {}

    virtual void Notify(std::string qr, int ec) {}
};

class CalcRatioNT {
public:
    virtual ~CalcRatioNT() {}

    virtual void Notify(double ratio_x, double ratio_y, int ec) {}
};

class Find2CirclesNT {
public:
    virtual ~Find2CirclesNT() {}

    virtual void Notify(int x1, int y1, int r1, int x2, int y2, int r2, int ec) {}
};

class Find4CirclesNT {
public:
    virtual ~Find4CirclesNT() {}

    virtual void Notify(
        int x1, int y1, int r1, int x2, int y2, int r2,
        int x3, int y3, int r3, int x4, int y4, int r4,
        int ec) = 0;
};

class AsynAPISet {
public:
    AsynAPISet() {
        running_ = true;
        clean_thread_ =
            new (std::nothrow) boost::thread(boost::bind(&AsynAPISet::CleanFunc, this));
    }

    ~AsynAPISet() {
//         running_ = false;
//         clean_thread_->join();
    }

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

    int AsynGetSystem(GetSystemNT* nt);

    int AsynReadMainSpare(
        ReadMainSpareNT* nt);

    int AsynWriteMainSpare(
        const std::string& sn,
        WriteMainSpareNT* nt);

    int AsynRecogQR(
        const std::string& file,
        int left, 
        int top,
        int right,
        int bottom,
        RecogQRNT* nt);

    int AsynCalcRatio(
        const std::string& file,
        const int dpi,
        CalcRatioNT* nt);

    int AsynFind2Circles(
        const std::string& file,
        Find2CirclesNT* nt);

    int AsynFind4Circles(
        const std::string& file,
        Find4CirclesNT* nt);

private:
    void AsynErrorNotify(BaseCmd* cmd, enum MC::ErrorCode ec);
    
    void ParseCmd(BaseCmd* cmd, char* chBuf);

    void* LookupSendTime(const std::string& send_time);

    void InsertNotify(const BaseCmd* cmd, const void* const nt);

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

    void HandleGetSystem(char* chBuf);

    void HandleReadMainSpare(char* chBuf);

    void HandleWriteMainSpare(char* chBuf);

    void HandleRecogQR(char* chBuf);

    void HandleCalcRatio(char* chBuf);

    void HandleFind2Circles(char* chBuf);

    void HandleFind4Circles(char* chBuf);

public:
    struct NotifySet {
        NotifySet() : nt(NULL), used(false) {

        }

        NotifySet(void* n, long begin) : nt(n), life_start(begin), used(false) {

        }

        void* nt;
        long life_start;

        bool used;
    };

    void CleanFunc();

private:
    std::map<std::string, NotifySet*> api_maps_;

    bool running_;
    boost::thread*          clean_thread_;
};

#endif
