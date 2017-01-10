#include <windows.h>
#include "api_set.h"
#include "log.h"
#include "api.h"

bool vista_better = false;

// #if (!vista_better)
// #define _XP
// #endif

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

int QueryMachine(std::string& sn)
{
    QueryMachineNT* nt = new (std::nothrow) QueryMachNT;
    api_agent.AsynQueryMachine(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QueryMachNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QueryMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QueryMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    sn = ((QueryMachNT*)nt)->sn_;
    int ret = ((QueryMachNT*)nt)->er_;
    //delete nt;
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

int SetMachine(const std::string& sn)
{
    SetMachineNT* nt = new (std::nothrow) SetMachNT;
    api_agent.AsynSetMachine(sn, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((SetMachNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((SetMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((SetMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((SetMachNT*)nt)->er_;
    //delete nt;
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

int InitMachine(const std::string& key)
{
    InitMachineNT* nt = new (std::nothrow) InitMaNT;
    api_agent.AsynInitMachine(key, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
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

int BindMAC(const std::string& mac)
{
    BindMACNT* nt = new BindNT;
    api_agent.AsynBindMAC(mac, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((BindNT*)nt)->cv_, WAIT_TIME))
        ((BindNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((BindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((BindNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((BindNT*)nt)->er_;
    //delete nt;
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

int UnbindMAC(const std::string& mac)
{
    UnbindMACNT* nt = new UnbindNT;
    api_agent.AsynUnbindMAC(mac, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((UnbindNT*)nt)->cv_, WAIT_TIME))
        ((UnbindNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((UnbindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((UnbindNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((UnbindNT*)nt)->er_;
    //delete nt;
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

int PrepareStamp(char stamp_num, int timeout, std::string& task_id)
{
    PrepareStampNT* nt = new PrepareNT;
    api_agent.AsynPrepareStamp(stamp_num, timeout, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((PrepareNT*)nt)->cv_, WAIT_TIME))
        ((PrepareNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((PrepareNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(timeout * 1000)))
        ((PrepareNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    task_id = ((PrepareNT*)nt)->task_id_;
    int ret = ((PrepareNT*)nt)->er_;
    //delete nt;
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

int QueryPaper(int& status)
{
    QueryPaperNT* nt = new PaperNT;
    api_agent.AsynQueryPaper(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((PaperNT*)nt)->cv_, WAIT_TIME))
        ((PaperNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((PaperNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((PaperNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    status = ((PaperNT*)nt)->status_;
    int ret = ((PaperNT*)nt)->er_;
    //delete nt;
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

    virtual void Notify(int ori_dpi, int cut_dpi, std::string ori_path, std::string cut_path, int ec) {
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

int Snapshot(
    int ori_dpi, 
    int cut_dpi,
    const std::string& ori_path, 
    const std::string& cut_path)
{
    SnapshotNT* nt = new SnapNT;
    api_agent.AsynSnapshot(ori_dpi, cut_dpi, ori_path, cut_path, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((SnapNT*)nt)->cv_, WAIT_TIME))
        ((SnapNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((SnapNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((SnapNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((SnapNT*)nt)->er_;
    //delete nt;
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

int MergePhoto(
    const std::string& p1, 
    const std::string& p2, 
    const std::string& merged)
{
    MergePhotoNT* nt = new MergeNT;
    api_agent.AsynMergePhoto(p1, p2, merged, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((MergeNT*)nt)->cv_, WAIT_TIME))
        ((MergeNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((MergeNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((MergeNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((MergeNT*)nt)->er_;
    //delete nt;
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

int RecognizeImage(const std::string& path,
    std::string& template_id, std::string& trace_num)
{
    RecognizeNT* nt = new RecogNT;
    api_agent.AsynRecognizeImage(path, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((RecogNT*)nt)->cv_, WAIT_TIME))
        ((RecogNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((RecogNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((RecogNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    template_id = ((RecogNT*)nt)->template_id_;
    trace_num = ((RecogNT*)nt)->trace_num_;
    int ret = ((RecogNT*)nt)->er_;
    //delete nt;
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

int IdentifyElement(
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

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((IdentiNT*)nt)->cv_, WAIT_TIME))
        ((IdentiNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((IdentiNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((IdentiNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    result = ((IdentiNT*)nt)->re_;
    int ret = ((IdentiNT*)nt)->er_;
    //delete nt;
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

int OrdinaryStamp(
    const std::string& task,
    const std::string& voucher, 
    int num, 
    int ink,
    int x, 
    int y, 
    int angle)
{
    OrdinaryStampNT* nt = new OridinaryNT;
    api_agent.AsynOrdinaryStamp(task, voucher, num, ink, x, y, angle, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((OridinaryNT*)nt)->cv_, WAIT_TIME))
        ((OridinaryNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((OridinaryNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((OridinaryNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((OridinaryNT*)nt)->er_;
    //delete nt;
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

int AutoStamp(const std::string& task,
    const std::string& voucher, int num)
{
    AutoStampNT* nt = new AutoNT;
    api_agent.AsynAutoStamp(task, voucher, num, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((AutoNT*)nt)->cv_, WAIT_TIME))
        ((AutoNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((AutoNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((AutoNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((AutoNT*)nt)->er_;
    //delete nt;
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

int FinishStamp(const std::string& task)
{
    FinishStampNT* nt = new FinishNT;
    api_agent.AsynFinishStamp(task, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((FinishNT*)nt)->cv_, WAIT_TIME))
        ((FinishNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((FinishNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((FinishNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((FinishNT*)nt)->er_;
    //delete nt;
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

int ReleaseStamp(const std::string& machine)
{
    ReleaseStampNT* nt = new ReleaseNT;
    api_agent.AsynReleaseStamp(machine, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((ReleaseNT*)nt)->cv_, WAIT_TIME))
        ((ReleaseNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((ReleaseNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((ReleaseNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((ReleaseNT*)nt)->er_;
    //delete nt;
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

int GetError(int err_code, std::string& err_msg, std::string& err_resolver)
{
    GetErrorNT* nt = new GetErrNT;
    api_agent.AsynGetError(err_code, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((GetErrNT*)nt)->cv_, WAIT_TIME))
        ((GetErrNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((GetErrNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((GetErrNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    err_msg = ((GetErrNT*)nt)->msg_;
    err_resolver = ((GetErrNT*)nt)->resolver_;
    int ret = ((GetErrNT*)nt)->er_;
    //delete nt;
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

int Calibrate(int slot)
{
    CalibrationNT* nt = new CaliNT;
    api_agent.AsynCalibrate(slot, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((CaliNT*)nt)->cv_, WAIT_TIME))
        ((CaliNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((CaliNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((CaliNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((CaliNT*)nt)->er_;
    //delete nt;
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

int QueryStampers(int* staus)
{
    QueryStampersNT* nt = new QueryStamNT(staus);
    api_agent.AsynQueryStampers(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QueryStamNT*)nt)->cv_, WAIT_TIME))
        ((QueryStamNT*)nt)->er_ = MC::EC_TIMEOUT;
#else
    if (!((QueryStamNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QueryStamNT*)nt)->er_ = MC::EC_TIMEOUT;
#endif

    int ret = ((QueryStamNT*)nt)->er_;
    //delete nt;
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

int QuerySafe(int& status)
{
    QuerySafeNT* nt = new QuerySafeDoorNT;
    api_agent.AsynQuerySafe(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QuerySafeDoorNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QuerySafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QuerySafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    status = ((QuerySafeDoorNT*)nt)->status_;
    int ret = ((QuerySafeDoorNT*)nt)->er_;
    //delete nt;
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

int ControlSafe(int ctrl)
{
    CtrlSafeNT* nt = new CtrLSafeDoorNT;
    api_agent.AsynSafeControl(ctrl, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((CtrLSafeDoorNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((CtrLSafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((CtrLSafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((CtrLSafeDoorNT*)nt)->er_;
    //delete nt;
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

int ControlBeep(int ctrl)
{
    CtrlBeepNT* nt = new BeepCtrlNT;
    api_agent.AsynBeepControl(ctrl, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((BeepCtrlNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((BeepCtrlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((BeepCtrlNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((BeepCtrlNT*)nt)->er_;
    //delete nt;
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

int QuerySlot(int& num)
{
    QuerySlotNT* nt = new QuerySlNT;
    api_agent.AsynQuerySlot(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QuerySlNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QuerySlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QuerySlNT*)nt)->er_ = MC::EC_TIMEOUT;

    num = ((QuerySlNT*)nt)->num_;
    int ret = ((QuerySlNT*)nt)->er_;
    //delete nt;
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

int ControlAlarm(int alarm, int switches)
{
    CtrlAlarmNT* nt = new AlarmNT;
    api_agent.AsynAlarmControl(alarm, switches, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((AlarmNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((AlarmNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((AlarmNT*)nt)->er_ = MC::EC_TIMEOUT;

    int ret = ((AlarmNT*)nt)->er_;
    //delete nt;
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

int QueryMAC(std::string& mac1, std::string& mac2)
{
    QueryMACNT* nt = new QryMACNT;
    api_agent.AsynQueryMAC(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
#ifdef _XP
    if (WAIT_TIMEOUT == WaitForSingleObject(((QryMACNT*)nt)->cv_, WAIT_TIME))
#else
    if (!((QryMACNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
#endif
        ((QryMACNT*)nt)->er_ = MC::EC_TIMEOUT;

    mac1 = ((QryMACNT*)nt)->mac1_;
    mac2 = ((QryMACNT*)nt)->mac2_;
    int ret = ((QryMACNT*)nt)->er_;
    //delete nt;
    return ret;
}
