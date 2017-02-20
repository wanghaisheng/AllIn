﻿#include <windows.h>
#define _XP

#ifndef _XP
#include <boost/thread/condition_variable.hpp>
#endif

#include "api_set.h"
#include "log.h"
#include "api.h"


AsynAPISet api_agent;

const int WAIT_TIME = 5000; // 等待异步通知回调超时时间(毫秒)

// 同步接口, 异步改同步接口, 同步阻塞等异步通知

///////////////////////////// 获取印控机编号 ////////////////////////////////

class QueryMachNT : public QueryMachineNT {
public:
#ifdef _XP
    QueryMachNT() {
        cv_ = CreateEvent(
            NULL, 
            TRUE,
            FALSE,
            NULL);
    }

    ~QueryMachNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string sn, int ec)
    {
        Log::WriteLog(LL_DEBUG, "QueryMachNT::Notify->获取印控仪编号, ec: %d, sn: %s",
            ec,
            sn.c_str());
        sn_ = sn;
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
        Log::WriteLog(LL_MSG, "QueryMachNT::Notify->event signaled");
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string sn_;
    int er_;
};

int ST_QueryMachine(std::string& sn)
{
    QueryMachineNT* nt = new (std::nothrow) QueryMachNT;
    api_agent.AsynQueryMachine(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QueryMachNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QueryMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QueryMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    sn = ((QueryMachNT*)nt)->sn_;
    int ret = ((QueryMachNT*)nt)->er_;
//    delete nt;
    return ret;
}

/////////////////////////////// 设置印控机编号 ////////////////////////////////

class SetMachNT : public SetMachineNT {
public:
#ifdef _XP
    SetMachNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SetMachNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string sn, int ec) {
        Log::WriteLog(LL_DEBUG, "SetMachNT::Notify->设置印控机编号, ec: %d, sn: %s",
            ec,
            sn.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_SetMachine(const std::string& sn)
{
    SetMachineNT* nt = new (std::nothrow) SetMachNT;
    api_agent.AsynSetMachine(sn, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((SetMachNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((SetMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((SetMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((SetMachNT*)nt)->er_;
    delete nt;
    return ret;
}

//////////////////////////// 初始化印控机 ///////////////////////////////////

class InitMaNT: public InitMachineNT {
public:
#ifdef _XP
    InitMaNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~InitMaNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string key, int ec)
    {
        Log::WriteLog(LL_DEBUG, "InitMaNT::Notify->初始化印控机, ec: %d, key: %s", 
            ec, 
            key.c_str());
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_InitMachine(const std::string& key)
{
    InitMachineNT* nt = new (std::nothrow) InitMaNT;
    api_agent.AsynInitMachine(key, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((InitMaNT*)nt)->cv_, WAIT_TIME))
        ((InitMaNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((InitMaNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((InitMaNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((InitMaNT*)nt)->er_;
    //delete nt;
    return ret;
}

///////////////////////////////// 绑定MAC地址 ///////////////////////////

class BindNT : public BindMACNT {
public:
#ifdef _XP
    BindNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~BindNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string mac, int ec)
    {
        Log::WriteLog(LL_DEBUG, "BindNT::Notify->绑定MAC地址, ec: %d, mac: %s", 
            ec, 
            mac.c_str());
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_BindMAC(const std::string& mac)
{
    BindMACNT* nt = new BindNT;
    api_agent.AsynBindMAC(mac, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((BindNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((BindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((BindNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((BindNT*)nt)->er_;
    delete nt;
    return ret;
}

//////////////////////// 解绑MAC地址 /////////////////////////////////

class UnbindNT : public UnbindMACNT {
public:
#ifdef _XP
    UnbindNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~UnbindNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string mac, int ec)
    {
        Log::WriteLog(LL_DEBUG, "UnbindNT::Notify->解绑MAC地址, ec: %d, mac: %s", 
            ec,
            mac.c_str());
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_UnbindMAC(const std::string& mac)
{
    UnbindMACNT* nt = new UnbindNT;
    api_agent.AsynUnbindMAC(mac, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((UnbindNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((UnbindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((UnbindNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((UnbindNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////// 准备用印 //////////////////////////////

class PrepareNT : public PrepareStampNT {
public:
#ifdef _XP
    PrepareNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~PrepareNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int num, int timeout, std::string task_id, int ec)
    {
        Log::WriteLog(LL_DEBUG, "PrepareNT::Notify->准备用印, ec: %d, 章卡槽号: %d, "
            "超时时间: %d, 任务ID: %s", 
            ec,
            num,
            timeout,
            task_id.c_str());

        task_id_ = task_id;
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string task_id_;
    int er_;
};

int ST_PrepareStamp(char stamp_num, int timeout, std::string& task_id)
{
    PrepareStampNT* nt = new PrepareNT;
    api_agent.AsynPrepareStamp(stamp_num, timeout, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((PrepareNT*)nt)->cv_, timeout * 1000))
#else
    if (!((PrepareNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(timeout * 1000)))
#endif
        ((PrepareNT*)nt)->er_ = MC::EC_TIMEOUT;

    task_id = ((PrepareNT*)nt)->task_id_;
    int ret = ((PrepareNT*)nt)->er_;
    delete nt;
    return ret;
}

////////////////////////// 查进进纸门状态 //////////////////////////////////

class PaperNT : public QueryPaperNT {
public:
#ifdef _XP
    PaperNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~PaperNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int status, int ec)
    {
        Log::WriteLog(LL_DEBUG, "PaperNT::Notify->查进纸门状态, ec: %d, 进纸门状态: %d",
            ec,
            status);

        status_ = status;
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int status_;
    int er_;
};

int ST_QueryPaper(int& status)
{
    QueryPaperNT* nt = new PaperNT;
    api_agent.AsynQueryPaper(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((PaperNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((PaperNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((PaperNT*)nt)->er_ = MC::EC_TIMEOUT;

    status = ((PaperNT*)nt)->status_;
    int ret = ((PaperNT*)nt)->er_;
    delete nt;
    return ret;
}

////////////////////////// 拍照 ////////////////////////////////////

class SnapNT : public SnapshotNT {
public:
#ifdef _XP
    SnapNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SnapNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ori_dpi, int cut_dpi,
                        std::string ori_path, std::string cut_path, int ec) {
        Log::WriteLog(LL_DEBUG, "SnapNT::Notify->拍照, ec: %d, ori_dpi: %d, cut_dpi: %d, "
            "ori_path: %s, cut_path: %s",
            ec,
            ori_dpi,
            cut_dpi,
            ori_path.c_str(),
            cut_path.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif

    int er_;
};

int ST_Snapshot(
    int which,
    const std::string& ori_path, 
    const std::string& cut_path)
{
    SnapshotNT* nt = new SnapNT;
    int ori_dpi = 0;
    int cut_dpi = 0;
    api_agent.AsynSnapshot(which, ori_dpi, cut_dpi, ori_path, cut_path, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((SnapNT*)nt)->cv_, WAIT_TIME))
        ((SnapNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((SnapNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((SnapNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((SnapNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////// 照片合成 /////////////////////////////////////

class MergeNT : public MergePhotoNT {
public:
#ifdef _XP
    MergeNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~MergeNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string p1, std::string p2, std::string merged, int ec) {
        Log::WriteLog(LL_DEBUG, "MergeNT::Notify->合成照片, ec: %d, "
        "图片1: %s, 图片2: %s, 合成图片: %s",
            ec,
            p1.c_str(),
            p2.c_str(),
            merged.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_MergePhoto(
    const std::string& p1, 
    const std::string& p2, 
    const std::string& merged)
{
    MergePhotoNT* nt = new MergeNT;
    api_agent.AsynMergePhoto(p1, p2, merged, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((MergeNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((MergeNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((MergeNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((MergeNT*)nt)->er_;
    delete nt;
    return ret;
}

////////////////////////// 原图用印点查找 //////////////////////////////////

class SearchStampNT : public SearchStampPointNT {
public:
#ifdef _XP
    SearchStampNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SearchStampNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int x, int y, double angle, int ec) {
        Log::WriteLog(LL_DEBUG, "SearchStampNT::Notify->原图用印点查找, ec: %d",
            ec);

        er_ = ec;
        x_ = x;
        y_ = y;
        angle_ = angle;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int x_;
    int y_;
    double angle_;
    int er_;
};

int ST_SearchSrcImageStampPoint(
    const char*     src_img_name,
    int             in_x,
    int             in_y,
    double          in_angle,
    int             &out_x,
    int             &out_y,
    double          &out_angle)
{
    SearchStampPointNT* nt = new SearchStampNT;
    api_agent.AsynSearchStampPoint(
        src_img_name, 
        in_x,
        in_y,
        in_angle,
        nt);

    SearchStampNT* derive_nt = (SearchStampNT*)nt;
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
    if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
        derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    out_x = derive_nt->x_;
    out_y = derive_nt->y_;
    out_angle = derive_nt->angle_;
    delete nt;
    return ret;
}

////////////////////////// 版面验证码识别 //////////////////////////////////

class RecogNT : public RecognizeNT {
public:
#ifdef _XP
    RecogNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~RecogNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string path, std::string template_id, std::string trace, int ec) {
        Log::WriteLog(LL_DEBUG, "RecogNT::Notify->版面验证码识别, ec: %d, 模板ID: %s, 追溯码: %s",
            ec,
            template_id.c_str(),
            trace.c_str());

        er_ = ec;
        template_id_ = template_id;
        trace_num_ = trace;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string template_id_;
    std::string trace_num_;
    int er_;
};

int ST_RecognizeImage(const std::string& path,
    std::string& template_id, std::string& trace_num)
{
    RecognizeNT* nt = new RecogNT;
    api_agent.AsynRecognizeImage(path, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((RecogNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((RecogNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((RecogNT*)nt)->er_ = MC::EC_TIMEOUT;

    template_id = ((RecogNT*)nt)->template_id_;
    trace_num = ((RecogNT*)nt)->trace_num_;
    int ret = ((RecogNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////// 要素识别 ////////////////////////////////

class IdentiNT: public IdentifyNT {
public:
#ifdef _XP
    IdentiNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~IdentiNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string path, int x, int y, int width, int height, int angle,
        std::string result, int ec) {
        Log::WriteLog(LL_DEBUG, "IdentiNT::Notify->要素识别, ec: %d, 识别结果: %s",
            ec,
            result.c_str());

        er_ = ec;
        re_ = result;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string re_;
    int er_;
};

int ST_IdentifyElement(
    const std::string& path,
    int x,
    int y,
    int width,
    int height,
    int angle,
    std::string& result)
{
    IdentifyNT* nt = new IdentiNT;
    api_agent.AsynIdentifyElement(path, x, y, width, height, angle, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((IdentiNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((IdentiNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((IdentiNT*)nt)->er_ = MC::EC_TIMEOUT;

    result = ((IdentiNT*)nt)->re_;
    int ret = ((IdentiNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////// 普通用印 /////////////////////////////////

class OridinaryNT: public OrdinaryStampNT {
public:
#ifdef _XP
    OridinaryNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~OridinaryNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string task, std::string voucher_type, int stamp_num,
        int x, int y, int angle, int ec) {
        Log::WriteLog(LL_DEBUG, "OridinaryNT::Notify->普通用印, ec: %d, 任务ID: %s", 
            ec,
            task.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_OrdinaryStamp(
    const std::string& task,
    int num, 
    int ink,
    int x, 
    int y, 
    int angle,
    int type)
{
    std::string voucher;
    OrdinaryStampNT* nt = new OridinaryNT;
    api_agent.AsynOrdinaryStamp(task, voucher, num, ink, x, y, angle, type, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((OridinaryNT*)nt)->cv_, STAMPING_WAIT_TIME))
#else
    if (!((OridinaryNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(STAMPING_WAIT_TIME)))
#endif
        ((OridinaryNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((OridinaryNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////// 自动用印 ////////////////////////////////////

class AutoNT: public AutoStampNT {
public:
#ifdef _XP
    AutoNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~AutoNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string task, std::string voucher_type, int stamp_num, int ec) {
        Log::WriteLog(LL_DEBUG, "AutoNT::Notify->自动用印, ec: %d, 任务ID: %s", 
            ec,
            task.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_AutoStamp(const std::string& task,
    const std::string& voucher, int num)
{
    AutoStampNT* nt = new AutoNT;
    api_agent.AsynAutoStamp(task, voucher, num, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((AutoNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((AutoNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((AutoNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((AutoNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////// 用印结束 //////////////////////////////////////

class FinishNT: public FinishStampNT {
public:
#ifdef _XP
    FinishNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~FinishNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string task, int ec) {
        Log::WriteLog(LL_DEBUG, "FinishNT::Notify->用印结束, ec: %d, 任务号: %s",
            ec, 
            task.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_FinishStamp(const std::string& task)
{
    FinishStampNT* nt = new FinishNT;
    api_agent.AsynFinishStamp(task, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((FinishNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((FinishNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((FinishNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((FinishNT*)nt)->er_;
    delete nt;
    return ret;
}

//////////////////////////// 释放印控机 //////////////////////////////////////

class ReleaseNT: public ReleaseStampNT {
public:
#ifdef _XP
    ReleaseNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~ReleaseNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string machine, int ec) {
        Log::WriteLog(LL_DEBUG, "ReleaseNT::Notify->释放印控机, ec: %d, 机器唯一编号: %s",
            ec, 
            machine.c_str());

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_ReleaseStamp(const std::string& machine)
{
    ReleaseStampNT* nt = new ReleaseNT;
    api_agent.AsynReleaseStamp(machine, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((ReleaseNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((ReleaseNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((ReleaseNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((ReleaseNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////// 获取错误信息 ////////////////////////////////

class GetErrNT: public GetErrorNT {
public:
#ifdef _XP
    GetErrNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~GetErrNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int er_code, std::string err_msg, std::string err_resolver, int ec) {
        Log::WriteLog(LL_DEBUG, "GetErrNT::Notify->获取错误信息, 错误码: %d, 错误信息: %s, 解决方案: %s",
            er_code,
            err_msg.c_str(),
            err_resolver.c_str());

        er_ = ec;
        msg_ = err_msg;
        resolver_ = err_resolver;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string msg_;
    std::string resolver_;
    int er_;
};

int ST_GetError(int err_code, std::string& err_msg, std::string& err_resolver)
{
    GetErrorNT* nt = new GetErrNT;
    api_agent.AsynGetError(err_code, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((GetErrNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((GetErrNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((GetErrNT*)nt)->er_ = MC::EC_TIMEOUT;

    err_msg = ((GetErrNT*)nt)->msg_;
    err_resolver = ((GetErrNT*)nt)->resolver_;
    int ret = ((GetErrNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////// 校准印章 ///////////////////////////////

class CaliNT: public CalibrationNT {
public:
#ifdef _XP
    CaliNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~CaliNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int slot, int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_Calibrate(int slot)
{
    CalibrationNT* nt = new CaliNT;
    api_agent.AsynCalibrate(slot, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((CaliNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((CaliNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((CaliNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((CaliNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////// 印章状态查询 ////////////////////////////////////

class QueryStamNT: public QueryStampersNT {
public:
    QueryStamNT(int* sta) {
        stat_ = sta;
#ifdef _XP
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
#endif
    }

#ifdef _XP
    ~QueryStamNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int* status, int ec) {
        int i = 0;
        while (status[i] != 0x0) {
            stat_[i] = status[i];
            ++i;
        }

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int* stat_;
    int er_;
};

int ST_QueryStampers(int* staus)
{
    QueryStampersNT* nt = new QueryStamNT(staus);
    api_agent.AsynQueryStampers(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QueryStamNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QueryStamNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QueryStamNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((QueryStamNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////// 安全门状态查询 ////////////////////////////////////

class QuerySafeDoorNT: public QuerySafeNT {
public:
#ifdef _XP
    QuerySafeDoorNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QuerySafeDoorNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int status, int ec) {
        status_ = status;
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int status_;
    int er_;
};

int ST_QuerySafe(int& status)
{
    QuerySafeNT* nt = new QuerySafeDoorNT;
    api_agent.AsynQuerySafe(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QuerySafeDoorNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QuerySafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QuerySafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    status = ((QuerySafeDoorNT*)nt)->status_;
    int ret = ((QuerySafeDoorNT*)nt)->er_;
    delete nt;
    return ret;
}

//////////////////////////// 安全门控制 //////////////////////////////////

class CtrLSafeDoorNT: public CtrlSafeNT {
public:
#ifdef _XP
    CtrLSafeDoorNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~CtrLSafeDoorNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ctrl, int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_ControlSafe(int ctrl)
{
    CtrlSafeNT* nt = new CtrLSafeDoorNT;
    api_agent.AsynSafeControl(ctrl, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((CtrLSafeDoorNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((CtrLSafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((CtrLSafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((CtrLSafeDoorNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////// 蜂鸣器控制 //////////////////////////////////

class BeepCtrlNT: public CtrlBeepNT {
public:
#ifdef _XP
    BeepCtrlNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~BeepCtrlNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ctrl, int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_ControlBeep(int ctrl, int type, int interval)
{
    CtrlBeepNT* nt = new BeepCtrlNT;
    api_agent.AsynBeepControl(ctrl, type, interval, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((BeepCtrlNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((BeepCtrlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((BeepCtrlNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((BeepCtrlNT*)nt)->er_;
    delete nt;
    return ret;
}

////////////////////////////// 卡槽数量查询 ////////////////////////////////

class QuerySlNT : public QuerySlotNT {
public:
#ifdef _XP
    QuerySlNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QuerySlNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int num, int ec) {
        Log::WriteLog(LL_DEBUG, "QuerySlNT::Notify->卡槽数量: %d", num);

        er_ = ec;
        num_ = num;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int num_;
    int er_;
};

int ST_QuerySlots(int &num)
{
    QuerySlotNT* nt = new QuerySlNT;
    api_agent.AsynQuerySlot(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QuerySlNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QuerySlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QuerySlNT*)nt)->er_ = MC::EC_TIMEOUT;

    num = ((QuerySlNT*)nt)->num_;
    int ret = ((QuerySlNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////////// 报警器开关 ////////////////////////////

class AlarmNT : public CtrlAlarmNT {
public:
#ifdef _XP
    AlarmNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~AlarmNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int alarm, int ctrl, int ec) {
        Log::WriteLog(LL_DEBUG, "AlarmNT::Notify->报警器类型: %d, 开关: %d", 
            alarm,
            ctrl);

        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_ControlAlarm(int alarm, int switches)
{
    CtrlAlarmNT* nt = new AlarmNT;
    api_agent.AsynAlarmControl(alarm, switches, nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((AlarmNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((AlarmNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((AlarmNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((AlarmNT*)nt)->er_;
    delete nt;
    return ret;
}

///////////////////////////////// 报警器状态查询 ////////////////////////////

class QryAlarmNT : public QueryAlarmNT {
public:
#ifdef _XP
    QryAlarmNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QryAlarmNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int door, int vibration, int ec) {
        Log::WriteLog(LL_DEBUG, "QryAlarmNT::Notify->报警器状态查询: %d");

        er_ = ec;
        door_ = door;
        vibration_ = vibration;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif

    int door_;
    int vibration_;
    int er_;
};

int ST_ReadAlarm(int& door, int& vibration)
{
    QueryAlarmNT* nt = new QryAlarmNT;
    api_agent.AsynQueryAlarm(nt);

    QryAlarmNT* derive_nt = (QryAlarmNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
        derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    door = derive_nt->door_;
    vibration = derive_nt->vibration_;
    delete nt;
    return ret;
}

/////////////////////////////// 查询已绑定MAC地址 /////////////////////////////

class QryMACNT: public QueryMACNT {
public:
#ifdef _XP
    QryMACNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QryMACNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string mac1, std::string mac2, int ec) {
        er_ = ec;
        mac1_ = mac1;
        mac2_ = mac2;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string mac1_;
    std::string mac2_;
    int er_;
};

int ST_QueryMAC(std::string& mac1, std::string& mac2)
{
    QueryMACNT* nt = new QryMACNT;
    api_agent.AsynQueryMAC(nt);

#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QryMACNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QryMACNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QryMACNT*)nt)->er_ = MC::EC_TIMEOUT;

    mac1 = ((QryMACNT*)nt)->mac1_;
    mac2 = ((QryMACNT*)nt)->mac2_;
    int ret = ((QryMACNT*)nt)->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 锁定印控仪 /////////////////////////////

class LockMachineNT: public LockNT {
public:
#ifdef _XP
    LockMachineNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~LockMachineNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;

#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif

    int er_;
};

int ST_Lock()
{
    LockNT* nt = new LockMachineNT;
    api_agent.AsynLock(nt);

    LockMachineNT* derive_nt = (LockMachineNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 解锁印控仪 /////////////////////////////

class UnlockMachineNT: public UnlockNT {
public:
#ifdef _XP
    UnlockMachineNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~UnlockMachineNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_Unlock()
{
    UnlockNT* nt = new UnlockMachineNT;
    api_agent.AsynUnlock(nt);

    UnlockMachineNT* derive_nt = (UnlockMachineNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 印控仪锁定状态查询 /////////////////////////////

class QryLockNT: public QueryLockNT {
public:
#ifdef _XP
    QryLockNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QryLockNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int lk, int ec) {
        er_ = ec;
        lock_ = lk;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int lock_;
    int er_;
};

int ST_QueryLock(int& lock)
{
    QueryLockNT* nt = new QryLockNT;
    api_agent.AsynQueryLock(nt);

    QryLockNT* derive_nt = (QryLockNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    lock = derive_nt->lock_;
    delete nt;
    return ret;
}

/////////////////////////////// 打开设备连接 /////////////////////////////

class OpenNT: public OpenCnnNT {
public:
#ifdef _XP
    OpenNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~OpenNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_Open()
{
    OpenCnnNT* nt = new OpenNT;
    api_agent.AsynOpen(nt);

    OpenNT* derive_nt = (OpenNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 关闭设备连接 /////////////////////////////

class CloseNT: public CloseCnnNT {
public:
#ifdef _XP
    CloseNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~CloseNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_Close()
{
    CloseCnnNT* nt = new CloseNT;
    api_agent.AsynClose(nt);

    CloseNT* derive_nt = (CloseNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 设备连接状态查询 /////////////////////////////

class QryCnnNT: public QueryCnnNT {
public:
#ifdef _XP
    QryCnnNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QryCnnNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int cnn, int ec) {
        er_ = ec;
        cnn_ = cnn;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int cnn_;
    int er_;
};

int ST_QueryCnn(int& cnn)
{
    QueryCnnNT* nt = new QryCnnNT;
    api_agent.AsynQueryCnn(nt);

    QryCnnNT* derive_nt = (QryCnnNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    cnn = derive_nt->cnn_;
    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 安全门报警器设置 /////////////////////////////

class SetSideAlarmNT: public SideDoorAlarmNT {
public:
#ifdef _XP
    SetSideAlarmNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SetSideAlarmNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_SetSideDoor(int keep, int timeout)
{
    SideDoorAlarmNT* nt = new SetSideAlarmNT;
    api_agent.AsynSetSideAlarm(keep, timeout, nt);

    SetSideAlarmNT* derive_nt = (SetSideAlarmNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 获取设备型号 /////////////////////////////

class GetModelNT: public DevModelNT {
public:
#ifdef _XP
    GetModelNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~GetModelNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(std::string model, int ec) {
        er_ = ec;
        model_ = model;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    std::string model_;
    int er_;
};

int ST_GetDevModel(std::string& model)
{
    DevModelNT* nt = new GetModelNT;
    api_agent.AsynQueryModel(nt);

    GetModelNT* derive_nt = (GetModelNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    model = derive_nt->model_;
    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 打开进纸门 /////////////////////////////

class OpenPaperDoorNT: public OpenPaperNT {
public:
#ifdef _XP
    OpenPaperDoorNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~OpenPaperDoorNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_OpenPaper(int timeout)
{
    OpenPaperNT* nt = new OpenPaperDoorNT;
    api_agent.AsynOpenPaper(timeout, nt);

    OpenPaperDoorNT* derive_nt = (OpenPaperDoorNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// LED补光灯控制 /////////////////////////////

class ControlLEDNT: public CtrlLedNT {
public:
#ifdef _XP
    ControlLEDNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~ControlLEDNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_ControlLed(int which, int ctrl, int value)
{
    CtrlLedNT* nt = new ControlLEDNT;
    api_agent.AsynCtrlLed(which, ctrl, value, nt);

    ControlLEDNT* derive_nt = (ControlLEDNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 用印坐标参数检查 /////////////////////////////

class ChkParaNT: public CheckParamNT {
public:
#ifdef _XP
    ChkParaNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~ChkParaNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_CheckParam(int x, int y, int angle)
{
    CheckParamNT* nt = new ChkParaNT;
    api_agent.AsynCheckParam(x, y, angle, nt);

    ChkParaNT* derive_nt = (ChkParaNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 打开摄像头 /////////////////////////////

class OpenCaNT: public OpenCameraNT {
public:
#ifdef _XP
    OpenCaNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~OpenCaNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_OpenCamera(int which)
{
    OpenCameraNT* nt = new OpenCaNT;
    api_agent.AsynOpenCamera(which, nt);

    OpenCaNT* derive_nt = (OpenCaNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 关闭摄像头 /////////////////////////////

class CloseCaNT: public CloseCameraNT {
public:
#ifdef _XP
    CloseCaNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~CloseCaNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_CloseCamera(int which)
{
    CloseCameraNT* nt = new CloseCaNT;
    api_agent.AsynCloseCamera(which, nt);

    CloseCaNT* derive_nt = (CloseCaNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 摄像头状态查询 /////////////////////////////

class QryCameraNT: public QueryCameraNT {
public:
#ifdef _XP
    QryCameraNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~QryCameraNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int which, int status, int ec) {
        er_ = ec;
        which_ = which;
        status_ = status;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int which_;
    int status_;
    int er_;
};

int ST_QueryCamera(int which, int& status)
{
    QueryCameraNT* nt = new QryCameraNT;
    api_agent.AsynQueryCamera(which, nt);

    QryCameraNT* derive_nt = (QryCameraNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    status = derive_nt->status_;
    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 设置分辨率 /////////////////////////////

class SetResoNT: public SetResolutionNT {
public:
#ifdef _XP
    SetResoNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SetResoNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_SetResolution(int which, int x, int y)
{
    SetResolutionNT* nt = new SetResoNT;
    api_agent.AsynSetResolution(which, x, y, nt);

    SetResoNT* derive_nt = (SetResoNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 设置DPI /////////////////////////////

class SetdpiNT: public SetDPIValueNT {
public:
#ifdef _XP
    SetdpiNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SetdpiNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_SetDPIValue(int which, int x, int y)
{
    SetDPIValueNT* nt = new SetdpiNT;
    api_agent.AsynSetDPI(which, x, y, nt);

    SetdpiNT* derive_nt = (SetdpiNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 设置摄像头属性 /////////////////////////////

class SetProNT: public SetPropertyNT {
public:
#ifdef _XP
    SetProNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~SetProNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_SetProperty(int which)
{
    SetPropertyNT* nt = new SetProNT;
    api_agent.AsynSetProperty(which, nt);

    SetProNT* derive_nt = (SetProNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 录像 /////////////////////////////

class RecordNT: public RecordVideoNT {
public:
#ifdef _XP
    RecordNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~RecordNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_StartRecordVideo(int which, const std::string& path)
{
    RecordVideoNT* nt = new RecordNT;
    api_agent.AsynStartRecordVideo(which, path, nt);

    RecordNT* derive_nt = (RecordNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 停止录像 /////////////////////////////

class StopRecordNT: public StopRecordVideoNT {
public:
#ifdef _XP
    StopRecordNT() {
        cv_ = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    }

    ~StopRecordNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int ec) {
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int er_;
};

int ST_StopRecordVideo(int which, const std::string& path)
{
    StopRecordVideoNT* nt = new StopRecordNT;
    api_agent.AsynStopRecordVideo(which, path, nt);

    StopRecordNT* derive_nt = (StopRecordNT*)nt;
#ifdef _XP
        if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
            derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    delete nt;
    return ret;
}

/////////////////////////////// 获取RFID /////////////////////////////

class GetrfidNT: public GetRFIDNT {
public:
#ifdef _XP
    GetrfidNT() {
        cv_ = CreateEvent(
                NULL,
                TRUE,
                FALSE,
                NULL);
    }

    ~GetrfidNT() {
        CloseHandle(cv_);
    }
#endif

    virtual void Notify(int rfid, int ec) {
        rfid_ = rfid;
        er_ = ec;
#ifdef _XP
        SetEvent(cv_);
#else
        cv_.notify_one();
#endif
    }

public:
#ifdef _XP
    HANDLE cv_;
#else
    boost::condition_variable cv_;
#endif
    int rfid_;
    int er_;
};

int ST_GetRFID(int slot, int& rfid)
{
    GetRFIDNT* nt = new GetrfidNT;
    api_agent.AsynGetRFID(slot, nt);

    GetrfidNT* derive_nt = (GetrfidNT*)nt;
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(derive_nt->cv_, WAIT_TIME))
#else
        if (!(derive_nt->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME))))
#endif
        derive_nt->er_ = MC::EC_TIMEOUT;

    int ret = derive_nt->er_;
    rfid = derive_nt->rfid_;
    delete nt;
    return ret;
}
