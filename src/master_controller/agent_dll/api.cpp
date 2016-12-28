#include "api.h"
#include "api_set.h"
#include "log.h"

AsynAPISet api_agent;

const int WAIT_TIME = 5000; // 等待异步通知回调超时时间(毫秒)

// 同步接口, 异步改同步接口, 同步阻塞等异步通知

///////////////////////////// 获取印控机编号 ////////////////////////////////

class QueryMachNT : public QueryMachineNT {
public:
    virtual void Notify(std::string sn, int ec)
    {
        Log::WriteLog(LL_DEBUG, "QueryMachNT::Notify->获取印控仪编号, ec: %d, sn: %s",
            ec,
            sn.c_str());
        sn_ = sn;
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    std::string sn_;
    int er_;
};

int QueryMachine(std::string& sn)
{
    QueryMachineNT* nt = new (std::nothrow) QueryMachNT;
    api_agent.AsynQueryMachine(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((QueryMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QueryMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    sn = ((QueryMachNT*)nt)->sn_;
    return ((QueryMachNT*)nt)->er_;
}

/////////////////////////////// 设置印控机编号 ////////////////////////////////

class SetMachNT : public SetMachineNT {
public:
    virtual void Notify(std::string sn, int ec) {
        Log::WriteLog(LL_DEBUG, "SetMachNT::Notify->设置印控机编号, ec: %d, sn: %s",
            ec,
            sn.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int SetMachine(const std::string& sn)
{
    SetMachineNT* nt = new (std::nothrow) SetMachNT;
    api_agent.AsynSetMachine(sn, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((SetMachNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((SetMachNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((SetMachNT*)nt)->er_;
}

//////////////////////////// 初始化印控机 ///////////////////////////////////

class InitMaNT: public InitMachineNT {
public:
    virtual void Notify(std::string key, int ec)
    {
        Log::WriteLog(LL_DEBUG, "InitMaNT::Notify->初始化印控机, ec: %d, key: %s", 
            ec, 
            key.c_str());
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int InitMachine(const std::string& key)
{
    InitMachineNT* nt = new (std::nothrow) InitMaNT;
    api_agent.AsynInitMachine(key, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((InitMaNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((InitMaNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((InitMaNT*)nt)->er_;
}

///////////////////////////////// 绑定MAC地址 ///////////////////////////

class BindNT : public BindMACNT {
public:
    virtual void Notify(std::string mac, int ec)
    {
        Log::WriteLog(LL_DEBUG, "BindNT::Notify->绑定MAC地址, ec: %d, mac: %s", 
            ec, 
            mac.c_str());
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int BindMAC(const std::string& mac)
{
    BindMACNT* nt = new BindNT;
    api_agent.AsynBindMAC(mac, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((BindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((BindNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((BindNT*)nt)->er_;
}

//////////////////////// 解绑MAC地址 /////////////////////////////////

class UnbindNT : public UnbindMACNT {
public:
    virtual void Notify(std::string mac, int ec)
    {
        Log::WriteLog(LL_DEBUG, "UnbindNT::Notify->解绑MAC地址, ec: %d, mac: %s", 
            ec,
            mac.c_str());
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int UnbindMAC(const std::string& mac)
{
    UnbindMACNT* nt = new UnbindNT;
    api_agent.AsynUnbindMAC(mac, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((UnbindNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((UnbindNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((UnbindNT*)nt)->er_;
}

///////////////////////// 准备用印 //////////////////////////////

class PrepareNT : public PrepareStampNT {
public:
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
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    std::string task_id_;
    int er_;
};

int PrepareStamp(char stamp_num, int timeout, std::string& task_id)
{
    PrepareStampNT* nt = new PrepareNT;
    api_agent.AsynPrepareStamp(stamp_num, timeout, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((PrepareNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(timeout * 1000)))
        ((PrepareNT*)nt)->er_ = MC::EC_TIMEOUT;

    task_id = ((PrepareNT*)nt)->task_id_;
    return ((PrepareNT*)nt)->er_;
}

////////////////////////// 查进进纸门状态 //////////////////////////////////

class PaperNT : public QueryPaperNT {
public:
    virtual void Notify(int status, int ec)
    {
        Log::WriteLog(LL_DEBUG, "PaperNT::Notify->查进纸门状态, ec: %d, 进纸门状态: %d",
            ec,
            status);

        status_ = status;
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int status_;
    int er_;
};

int QueryPaper(int& status)
{
    QueryPaperNT* nt = new PaperNT;
    api_agent.AsynQueryPaper(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((PaperNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((PaperNT*)nt)->er_ = MC::EC_TIMEOUT;

    status = ((PaperNT*)nt)->status_;
    return ((PaperNT*)nt)->er_;
}

////////////////////////// 拍照 ////////////////////////////////////

class SnapNT : public SnapshotNT {
public:
    virtual void Notify(int ori_dpi, int cut_dpi, std::string ori_path, std::string cut_path, int ec) {
        Log::WriteLog(LL_DEBUG, "SnapNT::Notify->拍照, ec: %d, ori_dpi: %d, cut_dpi: %d, "
            "ori_path: %s, cut_path: %s",
            ec,
            ori_dpi,
            cut_dpi,
            ori_path.c_str(),
            cut_path.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;

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
    if (!((SnapNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((SnapNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((SnapNT*)nt)->er_;
}

/////////////////////////// 照片合成 /////////////////////////////////////

class MergeNT : public MergePhotoNT {
public:
    virtual void Notify(std::string p1, std::string p2, std::string merged, int ec) {
        Log::WriteLog(LL_DEBUG, "MergeNT::Notify->合成照片, ec: %d, "
        "图片1: %s, 图片2: %s, 合成图片: %s",
            ec,
            p1.c_str(),
            p2.c_str(),
            merged.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((MergeNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((MergeNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((MergeNT*)nt)->er_;
}


////////////////////////// 版面验证码识别 //////////////////////////////////

class RecogNT : public RecognizeNT {
public:
    virtual void Notify(std::string path, std::string template_id, std::string trace, int ec) {
        Log::WriteLog(LL_DEBUG, "RecogNT::Notify->版面验证码识别, ec: %d, 模板ID: %s, 追溯码: %s",
            ec,
            template_id.c_str(),
            trace.c_str());

        er_ = ec;
        template_id_ = template_id;
        trace_num_ = trace;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((RecogNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((RecogNT*)nt)->er_ = MC::EC_TIMEOUT;

    template_id = ((RecogNT*)nt)->template_id_;
    trace_num = ((RecogNT*)nt)->trace_num_;
    return ((RecogNT*)nt)->er_;
}

///////////////////////////// 要素识别 ////////////////////////////////

class IdentiNT: public IdentifyNT {
public:
    virtual void Notify(std::string path, int x, int y, int width, int height, int angle,
        std::string result, int ec) {
        Log::WriteLog(LL_DEBUG, "IdentiNT::Notify->要素识别, ec: %d, 识别结果: %s",
            ec,
            result.c_str());

        er_ = ec;
        re_ = result;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((IdentiNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((IdentiNT*)nt)->er_ = MC::EC_TIMEOUT;

    result = ((IdentiNT*)nt)->re_;
    return ((IdentiNT*)nt)->er_;
}

///////////////////////////// 普通用印 /////////////////////////////////

class OridinaryNT: public OrdinaryStampNT {
public:
    virtual void Notify(std::string task, std::string voucher_type, int stamp_num,
        int x, int y, int angle, int ec) {
        Log::WriteLog(LL_DEBUG, "OridinaryNT::Notify->普通用印, ec: %d, 任务ID: %s", 
            ec,
            task.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((OridinaryNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((OridinaryNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((OridinaryNT*)nt)->er_;
}

///////////////////////////// 自动用印 ////////////////////////////////////

class AutoNT: public AutoStampNT {
public:
    virtual void Notify(std::string task, std::string voucher_type, int stamp_num, int ec) {
        Log::WriteLog(LL_DEBUG, "AutoNT::Notify->自动用印, ec: %d, 任务ID: %s", 
            ec,
            task.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int AutoStamp(const std::string& task,
    const std::string& voucher, int num)
{
    AutoStampNT* nt = new AutoNT;
    api_agent.AsynAutoStamp(task, voucher, num, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((AutoNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((AutoNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((AutoNT*)nt)->er_;
}

/////////////////////////// 用印结束 //////////////////////////////////////

class FinishNT: public FinishStampNT {
public:
    virtual void Notify(std::string task, int ec) {
        Log::WriteLog(LL_DEBUG, "FinishNT::Notify->用印结束, ec: %d, 任务号: %s",
            ec, 
            task.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int FinishStamp(const std::string& task)
{
    FinishStampNT* nt = new FinishNT;
    api_agent.AsynFinishStamp(task, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((FinishNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((FinishNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((FinishNT*)nt)->er_;
}

//////////////////////////// 释放印控机 //////////////////////////////////////

class ReleaseNT: public ReleaseStampNT {
public:
    virtual void Notify(std::string machine, int ec) {
        Log::WriteLog(LL_DEBUG, "ReleaseNT::Notify->释放印控机, ec: %d, 机器唯一编号: %s",
            ec, 
            machine.c_str());

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int ReleaseStamp(const std::string& machine)
{
    ReleaseStampNT* nt = new ReleaseNT;
    api_agent.AsynReleaseStamp(machine, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((ReleaseNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((ReleaseNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((ReleaseNT*)nt)->er_;
}

///////////////////////////// 获取错误信息 ////////////////////////////////

class GetErrNT: public GetErrorNT {
public:
    virtual void Notify(int er_code, std::string err_msg, std::string err_resolver, int ec) {
        Log::WriteLog(LL_DEBUG, "GetErrNT::Notify->获取错误信息, 错误码: %d, 错误信息: %s, 解决方案: %s",
            er_code,
            err_msg.c_str(),
            err_resolver.c_str());

        er_ = ec;
        msg_ = err_msg;
        resolver_ = err_resolver;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((GetErrNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((GetErrNT*)nt)->er_ = MC::EC_TIMEOUT;

    err_msg = ((GetErrNT*)nt)->msg_;
    err_resolver = ((GetErrNT*)nt)->resolver_;
    return ((GetErrNT*)nt)->er_;
}

/////////////////////////// 校准印章 ///////////////////////////////

class CaliNT: public CalibrationNT {
public:
    virtual void Notify(int slot, int ec) {
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int Calibrate(int slot)
{
    CalibrationNT* nt = new CaliNT;
    api_agent.AsynCalibrate(slot, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((CaliNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((CaliNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((CaliNT*)nt)->er_;
}

/////////////////////////// 印章状态查询 ////////////////////////////////////

class QueryStamNT: public QueryStampersNT {
public:
    QueryStamNT(int* sta) {
        stat_ = sta;
    }

    virtual void Notify(int* status, int ec) {
        int i = 0;
        while (status[i] != 0x0) {
            stat_[i] = status[i];
            ++i;
        }

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int* stat_;
    int er_;
};

int QueryStampers(int* staus)
{
    QueryStampersNT* nt = new QueryStamNT(staus);
    api_agent.AsynQueryStampers(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((QueryStamNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QueryStamNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((QueryStamNT*)nt)->er_;
}

/////////////////////////// 安全门状态查询 ////////////////////////////////////

class QuerySafeDoorNT: public QuerySafeNT {
public:
    virtual void Notify(int status, int ec) {
        status_ = status;
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int status_;
    int er_;
};

int QuerySafe(int& status)
{
    QuerySafeNT* nt = new QuerySafeDoorNT;
    api_agent.AsynQuerySafe(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((QuerySafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QuerySafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    status = ((QuerySafeDoorNT*)nt)->status_;
    return ((QuerySafeDoorNT*)nt)->er_;
}

//////////////////////////// 安全门控制 //////////////////////////////////

class CtrLSafeDoorNT: public CtrlSafeNT {
public:
    virtual void Notify(int ctrl, int ec) {
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int ControlSafe(int ctrl)
{
    CtrlSafeNT* nt = new CtrLSafeDoorNT;
    api_agent.AsynSafeControl(ctrl, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((CtrLSafeDoorNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((CtrLSafeDoorNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((CtrLSafeDoorNT*)nt)->er_;
}

///////////////////////// 蜂鸣器控制 //////////////////////////////////

class BeepCtrlNT: public CtrlBeepNT {
public:
    virtual void Notify(int ctrl, int ec) {
        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int ControlBeep(int ctrl)
{
    CtrlBeepNT* nt = new BeepCtrlNT;
    api_agent.AsynBeepControl(ctrl, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((BeepCtrlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((BeepCtrlNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((BeepCtrlNT*)nt)->er_;
}

////////////////////////////// 卡槽数量查询 ////////////////////////////////

class QuerySlNT : public QuerySlotNT {
public:
    virtual void Notify(int num, int ec) {
        Log::WriteLog(LL_DEBUG, "QuerySlNT::Notify->卡槽数量: %d", num);

        er_ = ec;
        num_ = num;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int num_;
    int er_;
};

int QuerySlot(int& num)
{
    QuerySlotNT* nt = new QuerySlNT;
    api_agent.AsynQuerySlot(nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((QuerySlNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QuerySlNT*)nt)->er_ = MC::EC_TIMEOUT;

    num = ((QuerySlNT*)nt)->num_;
    return ((QuerySlNT*)nt)->er_;
}

///////////////////////////////// 报警器开关 ////////////////////////////

class AlarmNT : public CtrlAlarmNT {
public:
    virtual void Notify(int alarm, int ctrl, int ec) {
        Log::WriteLog(LL_DEBUG, "AlarmNT::Notify->报警器类型: %d, 开关: %d", 
            alarm,
            ctrl);

        er_ = ec;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
    int er_;
};

int ControlAlarm(int alarm, int switches)
{
    CtrlAlarmNT* nt = new AlarmNT;
    api_agent.AsynAlarmControl(alarm, switches, nt);

    boost::mutex mtx;
    boost::mutex::scoped_lock lk(mtx);
    if (!((AlarmNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((AlarmNT*)nt)->er_ = MC::EC_TIMEOUT;

    return ((AlarmNT*)nt)->er_;
}

/////////////////////////////// 查询已绑定MAC地址 /////////////////////////////

class QryMACNT: public QueryMACNT {
public:
    virtual void Notify(std::string mac1, std::string mac2, int ec) {
        er_ = ec;
        mac1_ = mac1;
        mac2_ = mac2;
        cv_.notify_one();
    }

public:
    boost::condition_variable cv_;
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
    if (!((QryMACNT*)nt)->cv_.timed_wait(lk, boost::posix_time::milliseconds(WAIT_TIME)))
        ((QryMACNT*)nt)->er_ = MC::EC_TIMEOUT;

    mac1 = ((QryMACNT*)nt)->mac1_;
    mac2 = ((QryMACNT*)nt)->mac2_;
    return ((QryMACNT*)nt)->er_;
}
