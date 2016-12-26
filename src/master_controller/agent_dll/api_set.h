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

// 照片合成通知
class MergePhotoNT {
public:
    virtual void Notify(std::string p1, std::string p2, std::string merged, int ec) = 0;
};

// 版面验证码识别通知
class RecognizeNT {
public:
    virtual void Notify(std::string path, std::string template_id, std::string trace, int ec) = 0;
};

// 要素识别通知
class IdentifyNT {
public:
    virtual void Notify(std::string path, int x, int y, int width, int height, int angle,
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

// 用印结束
class FinishStampNT {
public:
    virtual void Notify(std::string task, int ec) = 0;
};

// 释放用印机
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

// 印章状态查询
class QueryStampersNT {
public:
    virtual void Notify(int* status, int ec) = 0;
};

// 安全门状态查询
class QuerySafeNT {
public:
    virtual void Notify(int status, int ec) = 0;
};

// 开关安全门
class CtrlSafeNT {
public:
    virtual void Notify(int ctrl, int ec) = 0;
};

// 蜂鸣器控制
class CtrlBeepNT {
public:
    virtual void Notify(int ctrl, int ec) = 0;
};

// 卡槽数量查询
class QuerySlotNT {
public:
    virtual void Notify(int num, int ec) = 0;
};

// 报警器控制
class CtrlAlarmNT {
public:
    virtual void Notify(int alarm, int ctrl, int ec) = 0;
};

// 查询MAC地址
class QueryMACNT {
public:
    virtual void Notify(std::string mac1, std::string mac2, int ec) = 0;
};

// 心跳通知
class HeartNT {
public:
    virtual void Notify() {}
};

class AsynAPISet {

public:
    int AsynQueryMachine(const QueryMachineNT* const nt);

    int AsynSetMachine(const std::string& sn, SetMachineNT* nt);

    int AsynInitMachine(const std::string& key, InitMachineNT* nt);

    int AsynBindMAC(const std::string& mac, BindMACNT* nt);

    int AsynUnbindMAC(const std::string& mac, UnbindMACNT* nt);

    int AsynPrepareStamp(int num, int timeout, PrepareStampNT* nt);

    int AsynQueryPaper(QueryPaperNT* nt);

    int AsynSnapshot(
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

    int AsynRecognizeImage(const std::string& path, RecognizeNT* nt);

    int AsynIdentifyElement(const std::string& path, int x, int y, int width, int height,
        int angle, IdentifyNT* nt);

    int AsynOrdinaryStamp(const std::string& task, 
        const std::string& voucher, int num, int x, int y, int angle, OrdinaryStampNT* nt);

    int AsynAutoStamp(const std::string& task,
        const std::string& voucher, int num, AutoStampNT* nt);

    int AsynFinishStamp(const std::string& task, FinishStampNT* nt);

    int AsynReleaseStamp(const std::string& machine, ReleaseStampNT* nt);

    int AsynGetError(int err_code, GetErrorNT* nt);

    int AsynCalibrate(int slot, CalibrationNT* nt);

    int AsynQueryStampers(QueryStampersNT* nt);

    int AsynQuerySafe(QuerySafeNT* nt);

    int AsynSafeControl(int ctrl, CtrlSafeNT* nt);

    int AsynBeepControl(int ctrl, CtrlBeepNT* nt);

    int AsynQuerySlot(QuerySlotNT* nt);

    int AsynAlarmControl(int alarm, int ctrl, CtrlAlarmNT* nt);

    int AsynQueryMAC(QueryMACNT* nt);

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

    void HandleQueryMAC(char* chBuf);

private:
    std::map<std::string, void*> api_maps_;
};

#endif
