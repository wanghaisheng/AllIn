#include "stdafx.h"
#include <cstdio>
#include <stdlib.h>
#include <sstream>
#include <cmath>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "event_cpu.h"
#include "tool.h"
#include "task_mgr.h"
#include "stamping_mgr.h"
#include "seal_api.h"

MC::STSealAPI* MC::STSealAPI::inst_ = NULL;

// typedef boost::posix_time::ptime Time;
// typedef boost::posix_time::time_duration TimeDuration;
// Time t1(boost::posix_time::microsec_clock::local_time());
// 
// Time t2(boost::posix_time::microsec_clock::local_time());
// 
// TimeDuration dt = t2 - t1;
// 
// //print formatted date
// std::cout << dt << std::endl;
// 
// //number of elapsed miliseconds
// long msec = dt.total_milliseconds();
// 
// //print elapsed seconds (with millisecond precision)
// std::cout << msec / 1000.0 << std::endl;

//////////////////////////// 获取用印机编号 ///////////////////////////////

class QueryMachEv : public MC::BaseEvent {

public:
    QueryMachEv(std::string des, MC::NotifyResult* notify)
        : BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        // 获取用印机编号
        // 本命令可以在任何时间，由任何PC调用
        unsigned char sn[23] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // 获取印控仪编号
        int ret = ReadStamperIdentifier(sn, sizeof(sn));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, (char*)sn);
        Log::WriteLog(LL_DEBUG, "MC::QueryMachEv::SpecificExecute->ec: %s, 印控仪编号: %s",
            MC::ErrorMsg[ec].c_str(),
            (char*)sn);
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryMachine(MC::NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryMachEv("获取用印机编号", notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 设置印控机编号 //////////////////////////////////

class SetMachEv : public MC::BaseEvent {

public:
    SetMachEv(std::string des, std::string sn, MC::NotifyResult* notify)
        : BaseEvent(des), sn_(sn),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // 设置印控机编号
        int ret = WriteStamperIdentifier((unsigned char*)sn_.c_str(), sn_.size());
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, sn_);
        Log::WriteLog(LL_DEBUG, "MC::QueryMachEv::SpecificExecute->ec: %s, 待设置印控仪编号: %s",
            MC::ErrorMsg[ec].c_str(),
            sn_.c_str());
        delete this;
    }

private:
    std::string sn_;
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::SetMachine(const std::string& sn, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SetMachEv("设置印控机编号", sn, notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 初始化印控机 ////////////////////////////////

class InitMachEv : public MC::BaseEvent {

public:
    InitMachEv(std::string des, std::string code, MC::NotifyResult* notify)
        : BaseEvent(des),
        code_(code),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        //1、	执行本命令时，用印机先核对key是否与该用印机出厂时提供的一致，不一致，返回失败.
        //2、	用印机判断调用本命令的PC的MAC地址是否在用印机绑定列表中：
        //      A：若在列表中：本命令返回成功。后续其他接口都可以使用。
        //      B：若不在列表中：则判断绑定的PC的MAC地址数，若地址数 < 2, 则本命令返回成功，
        //      后续PC只能调用“用印机绑定PC”接口，调用其他接口时返回失败；若地址数 >= 2, 则本命令返回失败。
        //3、   本指令应不依赖其他指令，即在任何时候（包括未释放用印机时）执行后，都应正确返回。

        std::string local_mac;  // 本机MAC地址(to-do)
        char code[512] = { 0 }; // 出厂时设定认证码
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // 初始化印控机
        int ret = GetDevCode(code, 511);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // 校验认证码, 不一致则返回失败
        if (0 != strcmp(code_.c_str(), code)) {
            ec = MC::EC_CODE_MISMATCH;
            goto NT;
        }

        // 一致则继续判断MAC地址
        unsigned char mac1[18] = { 0 };
        unsigned char mac2[18] = { 0 };
        ret = ReadMAC(mac1, mac2);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // mac地址在列表中, 返回成功
        if (0 == strcmp(local_mac.c_str(), (char*)mac1) ||
            0 == strcmp(local_mac.c_str(), (char*)mac2)) {
            ec = MC::EC_SUCC;
            goto NT;
        }

        // mac地址不在列表中, 判断已绑定MAC地址数
        int bound_count = 0;
        for (int i = 0; i < 18; ++i) {
            if (mac1[i] != 0x0) {
                ++bound_count;
                break;
            }
        }

        for (int i = 0; i < 18; ++i) {
            if (mac2[i] != 0x0) {
                ++bound_count;
                break;
            }
        }

        if (bound_count >= 2) {
            ec = MC::EC_FAIL;
            goto NT;
        }

        // mac 地址不在列表中, 且绑定MAC地址数小于2
        ec = MC::EC_SUC_CALL_BIND;

    NT:
        notify_->Notify(ec, code_);
        Log::WriteLog(LL_DEBUG, "MC::InitMachEv::SpecificExecute->ec: %s, 待验证认证码: %s, "
            "出厂时认证码: %s",
            MC::ErrorMsg[ec].c_str(), 
            code_.c_str(),
            code);
        delete this;
    }

private:
    std::string         code_;      // 待验证用印机认证码
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::InitMachine(std::string key, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) InitMachEv("初始化印控机", key, notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE, key);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 用印机绑定MAC地址 ///////////////////////////

class BindMACEv : public MC::BaseEvent {

public:
    BindMACEv(std::string des, std::string mac, MC::NotifyResult* notify)
        : BaseEvent(des),
        mac_(mac),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        // 一台用印机最多只能绑定2个MAC地址.

        unsigned char mac1[32] = { 0 };
        unsigned char mac2[32] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (mac_.length() > MAX_MAC_SIZE) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = ReadMAC(mac1, mac2, 18);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // MAC地址已经绑定过
        if (0 == strcmp(mac_.c_str(), (char*)mac1) ||
            0 == strcmp(mac_.c_str(), (char*)mac2)) {
            ec = MC::EC_ALREADY_BOUND;
            goto NT;
        }

        // 已经达到绑定MAC地址上限
        size_t mac1_len = strlen((char*)mac1);
        size_t mac2_len = strlen((char*)mac2);
        if (0 != mac1_len && 0 != mac2_len) {
            ec = MC::EC_MAC_MAX;
            goto NT;
        }

        // 绑定MAC地址
        if ((0 == mac1_len && 0 == mac2_len) || 
            (0 == mac1_len && 0 != mac2_len)) {         // 保存到第一个存储MAC地址位置
            int ret = WriteMAC(
                (unsigned char*)mac_.c_str(),
                NULL);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        }
        else if (0 != mac1_len && 0 == mac2_len) {    // 保存到第二个存储MAC地址位置
            int ret = WriteMAC(
                NULL,
                (unsigned char*)mac_.c_str());
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, mac_);
        Log::WriteLog(LL_DEBUG, "MC::BindMACEv->绑定MAC地址, ec: %d, 待绑定MAC: %s, "
            "已绑定地址1: %s, 已绑定地址2: %s",
            ec,
            mac_.c_str(),
            mac1,
            mac2);
        delete this;
    }

private:
    std::string         mac_;           // 待绑定MAC地址
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::BindMAC(std::string mac, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) BindMACEv("用印机绑定MAC地址", mac, notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE, mac);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 用印机解绑MAC ///////////////////////////

class UnbindMACEv : public MC::BaseEvent {

public:
    UnbindMACEv(std::string des, std::string mac, MC::NotifyResult* notify)
        : BaseEvent(des),
        mac_(mac),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        // 需在绑定后的PC上解绑
        unsigned char mac1[18] = { 0 };
        unsigned char mac2[18] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (mac_.length() > MAX_MAC_SIZE) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 用印机解绑MAC
        // 首先获取所有已绑定的MAC
        int ret = ReadMAC(mac1, mac2);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // 待解绑MAC地址未绑定到该印控机
        int mac1_cmp = strcmp(mac_.c_str(), (char*)mac1);
        int mac2_cmp = strcmp(mac_.c_str(), (char*)mac2);
        if (0 != mac1_cmp && 0 != mac2_cmp) {
            ec = MC::EC_NOT_BOUND;
            goto NT;
        }

        // 解绑MAC地址原则:  将特定的MAC地址写到操作"解绑MAC"地址的PC上
        if (0 == mac1_cmp) {
            ret = WriteMAC(
                (unsigned char*)"",
                NULL,
                1,
                0);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        } else {
            ret = WriteMAC(
                NULL,
                (unsigned char*)"",
                0,
                1);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, mac_);
        Log::WriteLog(LL_DEBUG, "MC::UnbindMACEv::SpecificExecute->解绑MAC地址, ec: %s, 待解绑MAC: %s,"
            "已绑定地址1: %s, 已绑定地址2: %s",
            MC::ErrorMsg[ec].c_str(), 
            mac_.c_str(),
            mac1,
            mac2);
        delete this;
    }

private:
    std::string         mac_;
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::UnbindMAC(std::string mac, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) UnbindMACEv("用印机解绑MAC地址", mac, notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE, mac);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 准备用印 ////////////////////////////

class PrepareStampEv : public MC::BaseEvent {

public:
    PrepareStampEv(std::string des, int num, int timeout, MC::NotifyResult* notify)
        : BaseEvent(des),
        running_(false),
        paper_thread(NULL),
        num_(num),
        timeout_(timeout),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        // 用印任务启动，用印机自动弹出进纸门供用户放入需盖章票据，
        // 用户放入盖章票据并关闭用印机进纸门，用印机返回本次盖章任务的任务ID。
        // 用印机提前将对应章准备好用印。若超过超时时间，用印机蜂鸣。
        // 自动用印和人工用印都需要此命令。

        char num_str[5] = { 0 };
        char timeout_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (num_ < 1 || num_ > 6 || timeout_ < 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // check top-door
        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        if (doors[3] == 1) {
            ec = MC::EC_TOP_DOOR_OPEN;
            goto NT;
        }

        // 印控仪处于锁定状态下不能准备用印, 需要先解锁印控仪
        if (IsLocked()) {
            ec = MC::EC_MACHINE_LOCKED;
            goto NT;
        }

        // 用印准备时锁定印控仪
        if (0 != Lock()) {
            ec = MC::EC_LOCK_MACHINE_FAIL;
            goto NT;
        }

        // 设置推纸门开启后超时提示时间, 默认30秒
        if (0 != timeout_) {
            ret = SetPaperDoor(timeout_);
            if (0  != ret) {
                Log::WriteLog(LL_ERROR, "MC::PrepareStampEv->设置纸门超时失败, er: %d", ret);
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        }

        // 打开推纸门
        ret = FOpenDoorPaper();
        if (-1 == ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // 开新线程检测进纸门状态
        running_ = true;
        paper_thread =
            new (std::nothrow) boost::thread(boost::bind(&PrepareStampEv::ThreadFunc, this));
        return;

    NT:
        notify_->Notify(ec, _itoa(num_, num_str, 10), _itoa(timeout_, timeout_str, 10));
        Log::WriteLog(LL_DEBUG, "MC::PrepareStampEv::SpecificExecute->准备用印, ec: %s, 章槽号: %d, "
            "超时: %d",
            MC::ErrorMsg[ec].c_str(),
            num_,
            timeout_);
    }

private:
    int                 num_;       // 章槽号
    int                 timeout_;   // 进纸门超时未关闭时间(暂定15秒)
    MC::NotifyResult*   notify_;

private:
    bool                running_;
    boost::thread*      paper_thread;
    void                ThreadFunc();
};

void PrepareStampEv::ThreadFunc()
{
    while (running_) {
        ResetEvent(MC::Tool::GetInst()->paper_door_ev_);
        DWORD ret = WaitForSingleObject(MC::Tool::GetInst()->paper_door_ev_, timeout_ * 1000);
        char num_str[5] = { 0 };
        char timeout_str[5] = { 0 };
        _itoa(num_, num_str, 10);
        _itoa(timeout_, timeout_str, 10);

        switch (ret) {
            // 超时时间内收到纸门关闭信号, 纸门关闭信号自动复位
        case WAIT_OBJECT_0: {
            boost::this_thread::interruption_point();

            MC::ErrorCode ec = MC::EC_SUCC;
            std::string task_id = MC::TaskMgr::GetInst()->GeneTask();
            notify_->Notify(
                ec,
                num_str,
                timeout_str,
                task_id);
            running_ = false;

            Log::WriteLog(LL_DEBUG, "PrepareStampEv::ThreadFunc->准备用印成功, "
                "ec: %s, 任务号: %s", 
                MC::ErrorMsg[ec].c_str(),
                task_id.c_str());
        }
            break;
            // 超时进纸门未关闭
        case WAIT_TIMEOUT: {
            boost::this_thread::interruption_point();

            MC::ErrorCode ec = MC::EC_PAPER_TIMEOUT;
            std::string task_id;
            notify_->Notify(
                ec,
                num_str,
                timeout_str,
                task_id);
            running_ = false;

            Log::WriteLog(LL_DEBUG, "PrepareStampEv::ThreadFunc->准备用印超时, "
                "ec: %s, 任务号: %s", 
                MC::ErrorMsg[ec].c_str(),
                task_id.c_str());
        }
            break;
        default:
            break;
        }

        Log::WriteLog(LL_DEBUG, "PrepareStampEv::ThreadFunc->检测进纸门关闭线程退出");
        paper_thread->interrupt();
        delete paper_thread;
        delete this;
    }
}

void MC::STSealAPI::PrepareStamp(int stamp_num, int timeout, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) PrepareStampEv("准备用印", stamp_num, timeout, notify);
    if (NULL == ev)
        return notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 查询进纸门 /////////////////////////////////

class QueryPaperEv : public MC::BaseEvent {
public:
    QueryPaperEv(std::string des, MC::NotifyResult* notify)
        : BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char paper_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 查询进纸门
        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(paper_str, "%d", doors[0]);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            paper_str);
        Log::WriteLog(LL_DEBUG, "MC::QueryPaperEv::SpecificExecute->查询进纸门, ec: %s, 状态: %s",
            MC::ErrorMsg[ec].c_str(),
            paper_str);
        delete this;
    }

private:
    MC::NotifyResult* notify_;
};

void MC::STSealAPI::QueryPaperDoor(MC::NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryPaperEv(
        "查询进纸门",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////////////// 拍照 /////////////////////////////////

class SnapshotEv : public MC::BaseEvent {

public:
    SnapshotEv(std::string des, int which, int original_dpi, int cut_dpi, 
        const std::string& ori_path, const std::string& cut_path,
        MC::NotifyResult* notify)
        : BaseEvent(des),
        which_(which),
        ori_dpi_(original_dpi),
        cut_dpi_(cut_dpi),
        ori_(ori_path),
        cut_(cut_path),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char ori_dpi_str[5] = { 0 };
        char cut_dpi_str[5] = { 0 };
        // 拍照时不需要检测印控机状态
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC) {
            Log::WriteLog(LL_MSG, "SnapshotEv::SpecificExecute->拍照时设备状态: %s",
                MC::ErrorMsg[ec].c_str());
        }

        int ret = CapturePhoto(
            (CAMERAINDEX)which_,
            enum IMAGEFORMAT(MC::SvrConfig::GetInst()->img_format_),
            (char*)ori_.c_str());
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "SnapshotEv::SpecificExecute->拍照失败, er: %d", ret);
            ec = MC::EC_CAPTURE_FAIL;
            goto NT;
        }

        // 纠偏去黑边
        ret = MC::ImgPro::GetInst()->CutImage(ori_, cut_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "SnapshotEv::SpecificExecute->纠偏去黑边失败, er: %d", ret);
            ec = MC::EC_PROCESS_IMG_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec, 
            _itoa(ori_dpi_, ori_dpi_str, 10),
            _itoa(cut_dpi_, cut_dpi_str, 10), 
            ori_,
            cut_);
        Log::WriteLog(LL_DEBUG, "MC::SnapshotEv::SpecificExecute->拍照, ec: %s, 原图路径: %s, "
            "切图路径: %s",
            MC::ErrorMsg[ec].c_str(),
            ori_.c_str(), 
            cut_.c_str());
        delete this;
    }

private:
    int which_;
    int ori_dpi_;
    int cut_dpi_;
    std::string ori_;
    std::string cut_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::Snapshot(
    int which,
    int original_dpi, 
    int cut_dpi, 
    const std::string& ori_path,
    const std::string& cut_path,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SnapshotEv(
        "拍照", 
        which,
        original_dpi, 
        cut_dpi, 
        ori_path,
        cut_path,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 照片合成 //////////////////////////////////

class MergePhotoEv : public MC::BaseEvent {

public:
    MergePhotoEv(
        std::string des, 
        std::string photo1, 
        std::string photo2, 
        std::string merged,
        MC::NotifyResult* notify)
        : BaseEvent(des),
        photo1_(photo1),
        photo2_(photo2),
        merged_(merged),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        FILE* file = fopen(photo1_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        file = fopen(photo2_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        // 照片合成
        int ret = MC::ImgPro::GetInst()->MergeImage(
            photo1_,
            photo2_,
            merged_);
        if (0 != ret) {
            ec = MC::EC_MERGE_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            photo1_,
            photo2_,
            merged_.c_str());
        Log::WriteLog(LL_DEBUG, "MC::MergePhotoEv::SpecificExecute->照片合成, ec: %d, 合成后路径: %s", 
            ec, 
            merged_.c_str());
        delete this;
    }

private:
    std::string photo1_;
    std::string photo2_;
    std::string merged_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::MergePhoto(
    const std::string& photo1, 
    const std::string& photo2, 
    const std::string& merged,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) MergePhotoEv("照片合成", photo1, photo2, merged, notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 版面验证码识别 ////////////////////////////////

class RecognitionEv : public MC::BaseEvent {

public:
    RecognitionEv(std::string des, std::string img, MC::NotifyResult* notify)
        : BaseEvent(des),
        img_(img),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        std::string voucher_no; // 凭证编号
        std::string trace_no;   // 追溯码
        double angle = 0;
        int x = 0;
        int y = 0;
        std::string model_type;
        std::string out_model_type;
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        FILE* file = fopen(img_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        // 版面验证码识别
        // 模板类型、角度、用印点识别
        int ret = MC::ImgPro::GetInst()->GetModelTypeAnglePoint(img_, model_type, angle, x, y);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::RecognitionEv::SpecificExecute->模版类型角度识别失败, er: %d",
                          ret);
            ec = MC::EC_MODEL_TYPE_FAIL;
            goto NT;
        }

        int out_angle = 0;
        ret = MC::ImgPro::GetInst()->IdentifyImage(
            img_,
            model_type,
            out_model_type,
            voucher_no,
            trace_no,
            x,
            y,
            out_angle);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::RecognitionEv::SpecificExecute->版面识别失败, er: %d",
                          ret);
            ec = MC::EC_RECOG_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            img_,
            "",
            voucher_no,
            trace_no);
        Log::WriteLog(LL_DEBUG, "MC::RecognitionEv::SpecificExecute->版面验证码识别, ec: %s, "
            "模板ID: %s, 追溯码: %s", 
            MC::ErrorMsg[ec].c_str(),
            voucher_no.c_str(),
            trace_no.c_str());
        delete this;
    }

private:
    std::string img_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::RecognizeImage(const std::string& img, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) RecognitionEv("版面验证码识别", img, notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 模板类型、角度、用印点识别 ////////////////////////////////

class RecogEtcEv : public MC::BaseEvent {

public:
    RecogEtcEv(std::string des, std::string img, MC::NotifyResult* notify)
        : BaseEvent(des),
        img_(img),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        std::string model;
        double angle = 0.f;
        int x = 0;
        int y = 0;
        char angle_str[64] = { 0 };
        char x_str[11] = { 0 };
        char y_str[11] = { 0 };
        MC::ErrorCode ec = MC::EC_SUCC;

        FILE* file = fopen(img_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        // 模板类型、角度、用印点识别
        int ret = MC::ImgPro::GetInst()->GetModelTypeAnglePoint(
            img_, 
            model, 
            angle, 
            x, 
            y);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::RecogEtcEv::SpecificExecute->模版类型角度识别失败, er: %d",
                ret);
            ec = MC::EC_MODEL_TYPE_FAIL;
            goto NT;
        }

        sprintf(angle_str, "%f", angle);
        sprintf(x_str, "%d", x);
        sprintf(y_str, "%d", y);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            model,
            angle_str,
            x_str,
            y_str);
        Log::WriteLog(LL_DEBUG, "MC::RecogEtcEv->模板及用印点, ec: %s, "
            "角度: %f, x: %d, y: %d", 
            MC::ErrorMsg[ec].c_str(),
            angle,
            x,
            y);
        delete this;
    }

private:
    std::string img_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::RecoModelEtc(
    const std::string& img,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) RecogEtcEv("模板及用印点", img, notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 原图用印点查找 ////////////////////////////////

class SearchPointEv : public MC::BaseEvent {

public:
    SearchPointEv(std::string des, std::string img, int x, int y, double angle, 
        MC::NotifyResult* notify)
        : BaseEvent(des),
        img_(img),
        x_(x),
        y_(y),
        angle_(angle),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char out_x_str[64] = {0};
        char out_y_str[64] = {0};
        char out_angle_str[64] = {0};
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        FILE* file = fopen(img_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        int out_x;
        int out_y;
        double out_angle;
        int ret = MC::ImgPro::GetInst()->SearchSrcImgStampPoint(
            img_.c_str(),
            x_,
            y_,
            angle_,
            out_x,
            out_y,
            out_angle);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::SearchPointEv->原图用印点查找, er: %d",
                          ret);
            ec = MC::EC_SEARCH_STAMP_FAIL;
            goto NT;
        }

        sprintf(out_x_str, "%d", out_x);
        sprintf(out_y_str, "%d", out_y);
        sprintf(out_angle_str, "%f", out_angle);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            out_x_str,
            out_y_str,
            out_angle_str);
        Log::WriteLog(LL_DEBUG, "MC::SearchPointEv::SpecificExecute->原图用印点查找, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    std::string img_;
    int x_;
    int y_;
    double angle_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::SearchSrcImageStampPoint(
    const std::string&  src_img_name,
    int                 in_x,
    int                 in_y,
    double              in_angle,
    NotifyResult*       notify) {
    BaseEvent* ev = new (std::nothrow) SearchPointEv(
        "原图用印点查找", 
        src_img_name, 
        in_x,
        in_y,
        in_angle,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////// 要素识别 //////////////////////////////////////

class IdentifyEleEv : public MC::BaseEvent {

public:
    IdentifyEleEv(
        std::string des, 
        std::string path, 
        int x, 
        int y, 
        int width,
        int height, 
        int angle, 
        MC::NotifyResult* notify)
        : BaseEvent(des),
        path_(path),
        x_(x),
        y_(y),
        width_(width),
        height_(height),
        angle_(angle),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        std::string result;
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        FILE* file = fopen(path_.c_str(), "r");
        if (NULL == file) {
            ec = MC::EC_FILE_NOT_EXIST;
            goto NT;
        }

        // 要素识别
        int ret = MC::ImgPro::GetInst()->IdentifyArea(
            path_, 
            x_, 
            y_, 
            width_, 
            height_, 
            angle_, 
            result);
        if (0 != ret) {
            ec = MC::EC_ELEMENT_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(
            ec,
            path_,
            result);
        Log::WriteLog(LL_DEBUG, "MC::IdentifyEleEv::SpecificExecute->要素识别, ec: %s, 结果: %s",
            MC::ErrorMsg[ec].c_str(),
            result.c_str());
        delete this;
    }

private:
    std::string path_;
    int x_;
    int y_;
    int width_;
    int height_;
    int angle_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::IdentifyElement(
    const std::string& path,
    int x,
    int y,
    int width,
    int height,
    int angle_,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) IdentifyEleEv(
        "要素识别", 
        path, 
        x,
        y,
        width,
        height,
        angle_,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////// 普通用印 ////////////////////////////////////////

class OridinaryEv : public MC::BaseEvent {

public:
    OridinaryEv(std::string des, std::string t, std::string v, int num, int ink,
        int x, int y, int angle, int type, MC::NotifyResult* notify): BaseEvent(des), 
        task_(t), 
        voucher_(v), 
        num_(num),
        ink_(ink),
        x_(x),
        y_(y),
        angle_(angle),
        type_(type),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (num_ < 1 || num_ > 6 || task_.empty() || (type_ != 0 && type_ != 1)
            || x_ <= 0 || y_ <= 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        //0x0B, 获取门状态(len = 4)
        //P1:   推纸门状态  0 关闭，1 开启， 2检测错误
        //P2:   电子锁状态，同上
        //P3:   机械锁状态，同上
        //P4:   顶盖状态，同上
        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::OridinaryEv->获取门状态失败, er: %d", ret);
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        if (doors[3] == 1) {
            Log::WriteLog(LL_ERROR, "MC::OridinaryEv->顶盖开启，请先关闭顶盖再盖章");
            ec = MC::EC_TOP_DOOR_OPEN;
            goto NT;
        }

        // 设备只有处于“空闲状态”才允许盖章
        unsigned char dev_status[24] = { 0 };
        ret = ::FGetDevStatus(dev_status, sizeof(dev_status));
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "OridinaryEv::Execute->查询设备状态失败, er: %d",
                ret);
            ec = MC::EC_QUERY_DEV_STATUS_FAIL;
            goto NT;
        }
        if (dev_status[1] != 3) {
            Log::WriteLog(LL_ERROR, "OridinaryEv::Execute->设备不处于空闲状态,不能盖章, status: %d",
                (int)dev_status[1]);
            ec = MC::EC_NOT_FREE;
            goto NT;
        }

        // 普通用印
        MC::TaskState ts = MC::TaskMgr::GetInst()->QueryTaskState(task_);
        // check task_id state
        if (ts ==  MC::TS_ALIVE) {
            unsigned int rfid;
            int ret = GetStamperID(num_ - 1, rfid);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OridinaryEv->获取章槽号%d的RFID失败, er: %d",
                    num_, ret);
                goto NT;
            }

            STAMPERPARAM pa;
            memcpy(&pa.seal, &rfid, sizeof(rfid));
            pa.serial_number = atoi(task_.c_str());
            pa.isPadInk = ink_;
            pa.x_point = x_;
            pa.y_point = y_;
            pa.w_time = MC::SvrConfig::GetInst()->wait_time_;
            pa.angle = angle_;
            pa.type = type_;

            Log::WriteLog(LL_DEBUG, "MC::OridinaryEv->普通用印, 物理用印点(%d, %d)",
                x_,
                y_);
            ret = FStartStamperstrc(&pa);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OridinaryEv->发起盖章失败, er: %d",
                    ret);
                goto NT;  
            }

            // 成功发起盖章, 标记当前任务号used
            MC::TaskMgr::GetInst()->MarkUsed(task_);

            // waits stamping completion
            if (0 != StampingMgr::GetInst()->Wait(STAMPING_WAIT_TIME)) {
                ec = MC::EC_STAMPING_TIMEOUT;
                goto NT;
            }

            ec = StampingMgr::GetInst()->QueryStampingResult();
            goto NT;
        } else if (ts == MC::TS_USED) {
            ec = MC::EC_TASK_CONSUMED;
            goto NT;
        } else {
            ec = MC::EC_TASK_NON_EXIST;
            goto NT;
        }

    NT:
        notify_->Notify(ec, task_);
        Log::WriteLog(LL_DEBUG, "OridinaryEv::SpecificExecute->普通用印, ec: %s, 任务号: %s",
            MC::ErrorMsg[ec].c_str(),
            task_.c_str());
        delete this;
    }

private:
    std::string task_;
    std::string voucher_;
    int         num_;
    int         ink_;
    int         x_;         // 盖章位置物理x坐标
    int         y_;
    int         angle_;
    int         type_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::OrdinaryStamp(
    const std::string& task,
    const std::string& voucher,
    int num, 
    int ink,
    int x, 
    int y, 
    int angle,
    int type,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OridinaryEv(
        "普通用印",
        task,
        voucher,
        num, 
        ink,
        x,
        y,
        angle,
        type,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

//////////////////////// 自动用印 ////////////////////////////////////////

class AutoEv : public MC::BaseEvent {
public:
    AutoEv(std::string des, std::string t, std::string v, int num, MC::NotifyResult* notify) :
        BaseEvent(des), task_(t), voucher_(v), num_(num), notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 自动用印
        MC::TaskState ts = MC::TaskMgr::GetInst()->QueryTaskState(task_);
        if (ts == MC::TS_ALIVE) {
            // to-do 自动用印



        }
        else if (ts == MC::TS_DEAD) {
            ec = MC::EC_TASK_CONSUMED;
            goto NT;
        }
        else {
            ec = MC::EC_TASK_NON_EXIST;
            goto NT;
        }

        ec = MC::EC_SUCC;
        
    NT:
        notify_->Notify(ec, task_);
        Log::WriteLog(LL_DEBUG, "MC::AutoEv::SpecificExecute->自动用印, ec: %s, 任务号: %s",
            MC::ErrorMsg[ec].c_str(),
            task_.c_str());
        delete this;
    }

private:
    std::string task_;
    std::string voucher_;
    int         num_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::AutoStamp(const std::string& task,
    const std::string& voucher, int num, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) AutoEv(
        "自动用印",
        task,
        voucher,
        num,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 用印结束 /////////////////////////////////////

class FinishEv : public MC::BaseEvent {
public:
    FinishEv(std::string des, std::string t, MC::NotifyResult* notify) :
        BaseEvent(des),
        task_(t), 
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (task_.empty()) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 用印结束
        MC::TaskState ts = MC::TaskMgr::GetInst()->QueryTaskState(task_);
        // 任务号已被结束
        if (MC::TS_DEAD == ts) {
            ec = MC::EC_TASK_CONSUMED;
            goto NT;
        }

        // 任务号不存在
        if (MC::TS_NON_EXIST == ts) {
            ec = MC::EC_TASK_NON_EXIST;
            goto NT;
        }

        // 解锁印控仪
        int ret = Unlock();
        if (0 != ret) {
            ec = MC::EC_LOCK_FAILURE;
            goto NT;
        }

        // 解锁印控仪成功, 删除任务号
        if (!MC::TaskMgr::GetInst()->RemoveTask(task_))
            Log::WriteLog(LL_ERROR, "FinishEv::SpecificExecute->删除任务号%s失败", task_.c_str());

        // 弹出纸门
        ret = FOpenDoorPaper();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            Log::WriteLog(LL_ERROR, "FinishEv::SpecificExecute->打开纸门失败, er: %d", ret);
            goto NT;
        }
        
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, task_);
        Log::WriteLog(LL_DEBUG, "MC::FinishEv::SpecificExecute->用印结束, ec: %s, 任务号: %s", 
            MC::ErrorMsg[ec].c_str(),
            task_.c_str());
        delete this;
    }

private:
    std::string task_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::FinishStamp(const std::string& task, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) FinishEv(
        "用印结束",
        task,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 释放印控机 //////////////////////////////////

class ReleaseEv : public MC::BaseEvent {
public:
    ReleaseEv(std::string des, std::string machine, MC::NotifyResult* notify) : BaseEvent(des),
        machine_(machine), notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 释放印控机
        // 是否锁定
        if (!IsLocked()) {
            ec = MC::EC_MACHINE_UNLOCKED;
            goto NT;
        }

        // 若锁定则解锁印控机
        int ret = Unlock();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, machine_);
        Log::WriteLog(LL_DEBUG, "MC::ReleaseEv::SpecificExecute->释放印控机, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    std::string machine_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::ReleaseStamp(const std::string& machine, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) ReleaseEv(
        "释放印控机",
        machine,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

//////////////////////////// 获取错误信息 /////////////////////////////////

class GetErrEv : public MC::BaseEvent {
public:
    GetErrEv(std::string des, int err, MC::NotifyResult* notify) : BaseEvent(des),
        err_(err), notify_(notify) {

    }

    virtual void SpecificExecute() {
        char err_code_str[64] = { 0 };
        std::string msg;
        std::string resolver;
        MC::ErrorCode ec = MC::EC_SUCC;

        if (err_ < 0 || err_ > MC::EC_MAX) {
            msg = "未定义的错误码";
            resolver = "重新输入错误码";
            goto NT;
        }

        // 获取错误信息
        msg = MC::GetErrMsg((MC::ErrorCode)err_);
        resolver = MC::GetErrResolver((MC::ErrorCode)err_);

    NT:
        notify_->Notify(ec, msg, resolver, _itoa(err_, err_code_str, 10));
        Log::WriteLog(LL_DEBUG, "MC::GetErrEv::SpecificExecute->获取错误信息: ec: %s, 错误码: %d, "
            "错误描述: %s, 解决方案: %s", 
            MC::ErrorMsg[ec].c_str(),
            err_, 
            msg.c_str(),
            resolver.c_str());
        delete this;
    }

private:
    int err_;   // 错误码

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::GetError(int err_code, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) GetErrEv(
        "获取错误信息",
        err_code,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

//////////////////////////////// 校准印章 //////////////////////////////

class CaliStamperEv : public MC::BaseEvent {
public:
    CaliStamperEv(std::string des, int num, MC::NotifyResult* notify) : BaseEvent(des),
        num_(num), notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (num_ < 1 || num_ > 6) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 校准印章
        unsigned int rfid = 0;
        int ret = GetStamperID(num_ - 1, rfid);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::CaliStamperEv::SpecificExecute->获取%d号章的RFID失败",
                num_);
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        if (0 == rfid) {
            Log::WriteLog(LL_ERROR, "MC::CaliStamperEv::SpecificExecute->%d号章槽无章", num_);
            ec = MC::EC_STAMP_NOT_EXIST;
            goto NT;
        }

        Log::WriteLog(LL_DEBUG, "MC::CaliStamperEv::SpecificExecute->%d号章对应rfid: %d",
            num_,
            rfid);
        ret = CalibrationEx((char*)&rfid, NULL, 0);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CaliStamperEv::SpecificExecute->校准印章, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int num_;   // 印章卡槽号

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::CalibrateMachine(int num, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CaliStamperEv(
        "校准印章",
        num,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////// 印章状态查询 /////////////////////////////////////

class QueryStampersEv : public MC::BaseEvent{
public:
    QueryStampersEv(std::string des, MC::NotifyResult* notify) : 
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char stampers_sta[7] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 印章状态查询
        unsigned int rfids[7] = { 0 };
        unsigned char stampers = 0;
        int ret = ReadAllRFID(rfids, 7, &stampers);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        for (int i = 0; i < 6; ++i) {
            if (rfids[i] == 0)
                stampers_sta[i] = 0x30;
            else if (rfids[i] != 0)
                stampers_sta[i] = 0x31;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, stampers_sta);
        Log::WriteLog(LL_DEBUG, "QueryStampersEv::SpecificExecute->印章状态查询, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult* notify_;
};

void MC::STSealAPI::QueryStampers(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryStampersEv(
        "印章状态查询",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 安全门状态查询 ////////////////////////////////////

class QuerySafeEv : public MC::BaseEvent {
public:
    QuerySafeEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char safe_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 安全门状态查询
        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(safe_str, "%d", doors[1]);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, safe_str);
        Log::WriteLog(LL_DEBUG, "MC::QuerySafeEv->安全门状态查询, ec: %s, 安全门状态: %s",
            MC::ErrorMsg[ec].c_str(),
            safe_str);
        delete this;
    }

private:
    MC::NotifyResult* notify_;
};

void MC::STSealAPI::QuerySafeDoor(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QuerySafeEv(
        "安全门状态查询",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////// 开关安全门 /////////////////////////////////////

class OperateSafeEv : public MC::BaseEvent {
public:
    OperateSafeEv(std::string des, int operation, int timeout, MC::NotifyResult* notify) :
        BaseEvent(des),
        operation_(operation),
        timeout_(timeout),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char ctrl_str[5] = { 0 };
        char timeout_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (operation_ != 0 && operation_ != 1) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 开关安全门
        //0x0B, 获取门状态(len = 4)
        //P1:   推纸门状态  0 关闭，1 开启， 2检测错误
        //P2:   电子锁状态，同上
        //P3:   机械锁状态，同上
        //P4:   顶盖状态，同上
        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }
        
        switch (operation_) {
        case 0: { // 关安全门
            // 更新章映射
            ret = SetStampMap();
            if (0 != ret) {
                ec = MC::EC_UPDATE_STAMP_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OperateSafeEv::SpecificExecute->更新章映射失败");
                goto NT;
            }

            // 先关电子锁
            ret = FCloseDoorSafe();
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OperateSafeEv::SpecificExecute->开电子锁失败");
                goto NT;
            }

            // 再退出维护模式
            ret = FQuitMaintainMode();
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OperateSafeEv::SpecificExecute->退出维护模式失败");
                goto NT;
            }
        }
            break;
        case 1: { // 开安全门
            if (doors[1] == 1) {
                ec = MC::EC_SAFE_OPENED;
                Log::WriteLog(LL_DEBUG, "MC::OperateSafeEv::SpecificExecute->安全门已打开");
                goto NT;
            }

            // 先进入维护模式
            ret = FMaintenanceMode();
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OperateSafeEv::SpecificExecute->进入维护模式失败");
                goto NT;
            }

            // 打开电子锁
            ret = FOpenDoorSafe();
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OperateSafeEv::SpecificExecute->打开电子锁失败");
                goto NT;
            }
        }
            break;
        default:
            break;
        }

        ec = MC::EC_SUCC;

    NT:
        sprintf(ctrl_str, "%d", operation_);
        sprintf(timeout_str, "%d", timeout_);
        notify_->Notify(ec, ctrl_str, timeout_str);
        Log::WriteLog(LL_DEBUG, "MC::OperateSafeEv::SpecificExecute->开关安全门, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int operation_;
    int timeout_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::OperateSafeDoor(int operation, int timeout, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OperateSafeEv(
        "开关安全门",
        operation,
        timeout,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////// 蜂鸣器开关 ///////////////////////////////////////

class OperateBeepEv : public MC::BaseEvent {
public:
    OperateBeepEv(std::string des, int operation, int type, int interval, 
        MC::NotifyResult* notify) :
        BaseEvent(des),
        operation_(operation),
        type_(type),
        interval_(interval),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (operation_ != 0 && operation_ != 1) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (type_ != 0 && type_ != 1) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 蜂鸣器开关
        //0x16, 蜂鸣器控制
        //beep      --- 0 关闭; 1 长鸣; 2 间隔响
        //interval  --- 当beep=2时该值有效, 间隔响时常(单位秒)
        if (operation_ == 0) {
            int ret = FBeepCtrl(0, 0);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        } else if (1 == operation_) {
            if (0 == type_) {
                int ret = FBeepCtrl(1, 0);
                if (0 != ret) {
                    ec = MC::EC_DRIVER_FAIL;
                    goto NT;
                }
            } else if (1 == type_) {
                if (interval_ < 0) {
                    ec = MC::EC_INVALID_PARAMETER;
                    goto NT;
                }

                int ret = FBeepCtrl(2, interval_);
                if (0 != ret) {
                    ec = MC::EC_DRIVER_FAIL;
                    goto NT;
                }
            }
        }


        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::OperateBeepEv::SpecificExecute->蜂鸣器开关, ec: %s", 
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int operation_;
    int type_;
    int interval_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::OperateBeep(int operation, int type, int interval, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OperateBeepEv(
        "蜂鸣器开关",
        operation,
        type,
        interval,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////// 卡槽数量查询 ///////////////////////////////////

class QuerySlotEv : public MC::BaseEvent {
public:
    QuerySlotEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char slots_str[4] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 卡槽数量查询
        unsigned int rfids[7] = { 0 };
        unsigned char stampers = 0;
        int ret = ReadAllRFID(rfids, 7, &stampers);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(slots_str, "%d", stampers - 1); // 1 indicates the safe door rfid
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, slots_str);
        Log::WriteLog(LL_DEBUG, "MC::QuerySlotEv::SpecificExecute->卡槽数量查询, ec: %s, 卡槽数: %s",
            MC::ErrorMsg[ec].c_str(),
            slots_str);
        delete this;
    }

private:
    MC::NotifyResult* notify_;
};

void MC::STSealAPI::QuerySlot(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QuerySlotEv(
        "卡槽数量查询",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 报警器控制 //////////////////////////////////

class AlarmCtrlEv : public MC::BaseEvent {
public:
    AlarmCtrlEv(std::string des, int alarm, int ctrl, MC::NotifyResult* notify) :
        BaseEvent(des),
        alarm_(alarm),
        ctrl_(ctrl),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char alarm_str[5] = { 0 };
        char ctrl_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (alarm_ != 0 && alarm_ != 1) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (ctrl_ != 0 && ctrl_ != 1) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // 报警器控制
        //alarm     --- 0(开门报警器)
        //              1(振动报警器)
        //switchs   --- 报警器开关
        //              1(开启);
        //              0(关闭)
        int ret = SetAlarm(alarm_, ctrl_);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(alarm_str, "%d", alarm_);
        sprintf_s(ctrl_str, "%d", ctrl_);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, alarm_str, ctrl_str);
        Log::WriteLog(LL_DEBUG, "MC::QuerySlotEv->报警器控制, ec: %s, 报警器类型: %s, "
            "开关: %s",
            MC::ErrorMsg[ec].c_str(),
            alarm_str,
            ctrl_str);
        delete this;
    }

private:
    int                 alarm_;
    int                 ctrl_;
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::OperateAlarm(int alarm, int ctrl, MC::NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) AlarmCtrlEv(
        "报警器控制",
        alarm,
        ctrl,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////// 报警器状态查询 //////////////////////////////////

class QueryAlarmEv : public MC::BaseEvent {
public:
    QueryAlarmEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char alarm_str[5] = { 0 };
        char ctrl_str[5] = { 0 };
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

//0x38, 读取报警器控制状态(包括振动报警和门报警)
//
//door          --- 门报警, 0是关, 1是开
//vibration     --- 振动报警, 0是关, 1是开
//
//返回值:
//      0: 读报警器状态成功
//      1: 失败
//      2: 忙
//USBCONTROLF60_API int ReadAlarmStatus(char* door, char* vibration);

        char door;
        char vibration;
        char door_str[11] = {0};
        char vibration_str[11] = {0};
        int ret = ReadAlarmStatus(&door, &vibration);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(door_str, "%d", door);
        sprintf_s(vibration_str, "%d", vibration);

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, door_str, vibration_str);
        Log::WriteLog(LL_DEBUG, "MC::QueryAlarmEv->报警器状态查询, 门报警: %s, 振动报警: %s, ec: %s",
            door_str,
            vibration_str,
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryAlarm(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryAlarmEv(
    "报警器状态查询",
    notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 查询已绑定MAC地址 //////////////////////////////////

class QueryMACEv : public MC::BaseEvent {
public:
    QueryMACEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        unsigned char mac1[24] = { 0 };
        unsigned char mac2[24] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // 查询MAC地址
        int ret = ReadMAC(mac1, mac2, 18);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, (char*)mac1, (char*)mac2);
        Log::WriteLog(LL_DEBUG, "MC::QueryMACEv::SpecificExecute->查询MAC地址, ec: %s, MAC1: %s, "
            "MAC2: %s",
            MC::ErrorMsg[ec].c_str(),
            mac1,
            mac2);
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryMAC(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryMACEv(
        "查询已绑定MAC地址",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 锁定印控仪 ///////////////////////////////////

class LockMachineEv : public MC::BaseEvent {
public:
    LockMachineEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // lock machine
        int ret = Lock();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::LockMachineEv::SpecificExecute->锁定印控仪, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::Lock(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) LockMachineEv(
        "锁定印控仪",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 解锁印控仪 //////////////////////////////////

class UnlockMachineEv : public MC::BaseEvent {
public:
    UnlockMachineEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        // unlock machine
        int ret = Unlock();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::UnlockMachineEv::SpecificExecute->解锁印控仪, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::Unlock(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) UnlockMachineEv(
        "解锁印控仪",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 印控仪锁定状态 //////////////////////////////////

class QueryLockEv : public MC::BaseEvent {
public:
    QueryLockEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        bool ret = IsLocked();
        if (!ret) {
            ec = MC::EC_MACHINE_UNLOCKED;
            goto NT;
        }

        ec = MC::EC_MACHINE_LOCKED;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::QueryLockEv::SpecificExecute->印控仪锁定状态, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryLock(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryLockEv(
        "印控仪锁定状态",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 打开设备连接 //////////////////////////////////

class CnnEv: public MC::BaseEvent {
public:
    CnnEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = FOpenDev(NULL);
        if (0 != ret) {
            ec = MC::EC_OPEN_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CnnEv::SpecificExecute->打开设备失败, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::Connect(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CnnEv("打开设备", notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 关闭设备连接 //////////////////////////////////

class CloseCnnEv: public MC::BaseEvent {
public:
    CloseCnnEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = FCloseDev();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CloseCnnEv::SpecificExecute->关闭设备连接, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::Disconnect(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CloseCnnEv("关闭设备连接", notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 连接状态查询 //////////////////////////////////

class QueryCnnEv: public MC::BaseEvent {
public:
    QueryCnnEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        ec = MC::EC_CONNECTED;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::QueryCnnEv::SpecificExecute->连接状态, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryCnn(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryCnnEv("设备连接状态", notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 安全门报警器设置 //////////////////////////////////

class SetSideDoorEv: public MC::BaseEvent {
public:
    SetSideDoorEv(std::string des, int keep, int timeout, MC::NotifyResult* notify) :
        BaseEvent(des),
        keep_(keep),
        timeout_(timeout),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (keep_ < 0 || timeout_ < 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = SetSideDoor(keep_, timeout_);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::SetSideDoorEv::SpecificExecute->侧门报警器设置, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int keep_;
    int timeout_;
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::SetSideDoor(int keep_open, int timeout, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SetSideDoorEv(
        "安全门报警器设置", 
        keep_open,
        timeout,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 获取设备型号 //////////////////////////////////

class GetDevModelEv: public MC::BaseEvent {
public:
    GetDevModelEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        unsigned char sn[49] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;
        
        int ret = ReadBackupSN(sn, 48);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::GetDevModelEv->获取设备型号失败, er: %d", ret);
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, (char*)sn);
        Log::WriteLog(LL_DEBUG, "MC::GetDevModelEv::SpecificExecute->获取设备型号: %s, ec: %s",
            sn,
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryDevModel(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) GetDevModelEv("获取设备型号", notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 打开进纸门 //////////////////////////////////

class OpenPaperEv: public MC::BaseEvent {
public:
    OpenPaperEv(std::string des, int timeout, MC::NotifyResult* notify) :
        BaseEvent(des),
        timeout_(timeout),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (0 > timeout_) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (0 == timeout_)
            timeout_ = 30;

        int ret = SetPaperDoor(timeout_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::OpenPaperEv::SpecificExecute->设置纸门超时"
                "时间(%d)失败", timeout_);
        }

        ret = FOpenDoorPaper();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::OpenPaperEv::SpecificExecute->打开进纸门, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int timeout_;
    MC::NotifyResult*   notify_;
};


void MC::STSealAPI::OpenPaper(int timeout, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OpenPaperEv(
        "打开进纸门", 
        timeout,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 补光灯控制 //////////////////////////////////

class CtrlLEDEv: public MC::BaseEvent {
public:
    CtrlLEDEv(std::string des, int which, int swi, int val, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        switch_(swi),
        value_(val),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if ((which_ != 1 && which_ != 2) 
            || (switch_ != 0 && switch_ != 1)) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (1 == switch_) {
            if (value_ < 1 || value_ > 100) {
                ec = MC::EC_INVALID_PARAMETER;
                goto NT;
            }
        }
//0x0F, 补光灯控制
//light     --- 补光灯类型
//              1 -- 安全门旁边的补光灯; 
//              2 -- 凭证摄像头旁边的补光灯
//op        --- 操作(0 -- 关; 1 -- 开)
//USBCONTROLF60_API   int FLightCtrl(char light, int op);

//0x17, 补光灯亮度调节
//light         --- 补光灯类型
//                  1 安全门旁边的补光灯
//                  2 凭证摄像头旁边的补光灯
//brightness    --- 亮度值(建议范围 1-100, 1为最弱, 100为最亮)
//USBCONTROLF60_API   int FLightBrightness(char light, char brightness);

        int ret = FLightCtrl(which_, switch_);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        // 开补光灯需要同时设置亮度
        if (1 == switch_) {
            ret = FLightBrightness(which_, value_);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                goto NT;
            }
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CtrlLEDEv::SpecificExecute->补光灯控制, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    int switch_;
    int value_;

    MC::NotifyResult*   notify_;
};

// 补光灯控制
void MC::STSealAPI::CtrlLed(int which, int switchs, int value, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CtrlLEDEv(
        "补光灯控制", 
        which,
        switchs,
        value,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 用印参数检查 //////////////////////////////////

class CheckParaEv: public MC::BaseEvent {
public:
    CheckParaEv(std::string des, int x, int y, int angle, MC::NotifyResult* notify) :
        BaseEvent(des),
        x_(x),
        y_(y),
        angle_(angle),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        char physical[12] = { 0 };
        int ret = GetPhsicalRange(physical, 12);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::CheckParaEv->读盖章物理范围失败, er: %d", ret);
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        unsigned short x_width = 0;
        memcpy(&x_width, physical, sizeof(unsigned short));

        unsigned short y_width = 0;
        memcpy(&y_width, physical + sizeof(unsigned short), sizeof(unsigned short));

        unsigned short x_min = 0;
        memcpy(&x_min, physical + 2 * sizeof(unsigned short), sizeof(unsigned short));

        unsigned short x_max = 0;
        memcpy(&x_max, physical + 3 * sizeof(unsigned short), sizeof(unsigned short));

        unsigned short y_min = 0;
        memcpy(&y_min, physical + 4 * sizeof(unsigned short), sizeof(unsigned short));

        unsigned short y_max = 0;
        memcpy(&y_max, physical + 5 * sizeof(unsigned short), sizeof(unsigned short));

        if (x_ < x_min || x_ > x_max) {
            Log::WriteLog(LL_ERROR, "MC::CheckParaEv->x坐标:%d非法, 不属于(%d, %d)",
                x_,
                x_min,
                x_max);
            ec = MC::EC_INVALID_X_PARAM;
            goto NT;
        }

        if (y_ < y_min || y_ > y_max) {
            Log::WriteLog(LL_ERROR, "MC::CheckParaEv->y坐标:%d非法, 不属于(%d, %d)",
                y_,
                y_min,
                y_max);
            ec = MC::EC_INVALID_Y_PARAM;
            goto NT;
        }

        if (angle_ < 0 || angle_ > 360) {
            Log::WriteLog(LL_ERROR, "MC::CheckParaEv->章旋转角度: %d非法",
                angle_);
            ec = MC::EC_INVALID_ANGLE;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CheckParaEv::SpecificExecute->用印参数检查, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int x_;
    int y_;
    int angle_;
    MC::NotifyResult*   notify_;
};

// 用印参数合法性检查
void MC::STSealAPI::CheckParams(int x, int y, int angle, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CheckParaEv(
        "用印参数检查", 
        x,
        y,
        angle,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 打开摄像头 //////////////////////////////////

class OpenCameraEv: public MC::BaseEvent {
public:
    OpenCameraEv(std::string des, int which, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = OpenCamera((CAMERAINDEX)which_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::OpenCameraEv->打开摄像头失败, er: %d", ret);
            ec = MC::EC_OPEN_CAM_FAIL;
            goto NT;
        }

        MC::Tool::GetInst()->UpdateCams(which_, true);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::OpenCameraEv::SpecificExecute->打开摄像头, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::OpenCamera(int which, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OpenCameraEv(
        "打开摄像头", 
        which,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 关闭摄像头 //////////////////////////////////

class CloseCameraEv: public MC::BaseEvent {
public:
    CloseCameraEv(std::string des, int which, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = CloseCamera((CAMERAINDEX)which_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::CloseCameraEv->关闭摄像头失败，er: %d", ret);
            ec = MC::EC_CLOSE_CAM_FAIL;
            goto NT;
        }

        MC::Tool::GetInst()->UpdateCams(which_, false);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::CloseCameraEv::SpecificExecute->关闭摄像头, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::CloseCamera(int which, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CloseCameraEv(
        "关闭摄像头", 
        which,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 摄像头状态 //////////////////////////////////

class QueryCameraEv: public MC::BaseEvent {
public:
    QueryCameraEv(std::string des, int which, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char buffer[4] = {0};
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        bool status = MC::Tool::GetInst()->QueryCam(which_);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, status? _itoa(1, buffer, 10) : _itoa(0, buffer, 10));
        Log::WriteLog(LL_DEBUG, "MC::QueryCameraEv::SpecificExecute->摄像头状态, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryCameraStatus(int which, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryCameraEv(
        "摄像头状态", 
        which,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 设置分辨率 //////////////////////////////////

class SetResolutionEv: public MC::BaseEvent {
public:
    SetResolutionEv(
        std::string des, 
        int which,
        int x,
        int y,
        MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        x_(x),
        y_(y),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (x_ <= 0 || y_ <= 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = SetResolution((CAMERAINDEX)which_, x_, y_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::SetResolutionEv->设置分辨率失败, er: %d", ret);
            ec = MC::EC_SET_RESO_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::SetResolutionEv::SpecificExecute->设置分辨率, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    int x_;
    int y_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::SetResolution(int which, int x, int y, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SetResolutionEv(
        "设置分辨率",
        which,
        x,
        y,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 设置DPI值 //////////////////////////////////

class SetDPIEv: public MC::BaseEvent {
public:
    SetDPIEv(
        std::string des, 
        int which,
        int x,
        int y,
        MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        x_(x),
        y_(y),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        if (x_ <= 0 || y_ <= 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = SetDPIValue((CAMERAINDEX)which_, x_, y_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::SetDPIEv->设置DPI失败, er: %d", ret);
            ec = MC::EC_SET_DPI_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::SetDPIEv::SpecificExecute->设置DPI, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    int x_;
    int y_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::SetDPI(int which, int x, int y, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SetDPIEv(
        "设置DPI值",
        which,
        x,
        y,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 设置属性 //////////////////////////////////

class SetPropertyEv: public MC::BaseEvent {
public:
    SetPropertyEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = FOpenDev(NULL);
        if (!ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::SetPropertyEv::SpecificExecute->打开设备失败, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::SetProperty(int which, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SetPropertyEv("设置摄像头属性", notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 录像 //////////////////////////////////

class RecordVideoEv: public MC::BaseEvent {
public:
    RecordVideoEv(std::string des, int which, std::string path, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        path_(path),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        // start preview before capture video
        HWND preview_hwnd = ::CreateWindowA(
            "STATIC",
            "record", 
            WS_MINIMIZE,
            0, 
            0, 
            PREVIEW_WIDTH,
            PREVIEW_HEIGHT,
            NULL, NULL, NULL, NULL);
/*        ::SetWindowTextA(preview_hwnd, "Record Window!");*/

        int ret = StartPreview(
            (CAMERAINDEX)which_,
            PREVIEW_WIDTH, 
            PREVIEW_HEIGHT, 
            preview_hwnd);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::RecordVideoEv->开启预览失败, er: %d", ret);
            ec = MC::EC_START_PREVIEW_FAIL;
            goto NT;
        }

        ret = StartCaptureVideo((CAMERAINDEX)which_, (char*)path_.c_str());
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::RecordVideoEv->开启录像失败, er: %d, 存放路径: %s",
                ret,
                path_.c_str());
            ec = MC::EC_START_RECORD_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::RecordVideoEv::SpecificExecute->开启录像, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    std::string path_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::RecordVideo(int which, const std::string& path, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) RecordVideoEv(
        "录像",
        which,
        path,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 停止录像 //////////////////////////////////

class StopRecordVideoEv: public MC::BaseEvent {
public:
    StopRecordVideoEv(std::string des, int which, std::string path, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        path_(path),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (which_ != 0 && which_ != 1 && which_ != 2) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        StopPreview((CAMERAINDEX)which_);
        int ret = StopCaptureVideo((CAMERAINDEX)which_, (char*)path_.c_str());
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::StopRecordVideoEv->停止录像失败, er: %d", ret);
            ec = MC::EC_STOP_RECORD_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::StopRecordVideoEv::SpecificExecute->停止录像, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    std::string path_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::StopRecordVideo(int which, const std::string& path, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) StopRecordVideoEv(
        "停止录像",
        which,
        path,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

/////////////////////////////////////////////////

class GetRFIDEv: public MC::BaseEvent {
public:
    GetRFIDEv(std::string des, int slot, MC::NotifyResult* notify) :
            BaseEvent(des),
            slot_(slot),
            notify_(notify) {

    }

    virtual void SpecificExecute() {
        unsigned int rfid;
        char rfid_str[11] = {0};
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (slot_ < 1 || slot_ > 6) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        unsigned int rfids[7] = { 0 };
        unsigned char stampers = 0;
        int ret = ReadAllRFID(rfids, 7, &stampers);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        rfid = rfids[slot_ - 1];
        sprintf(rfid_str, "%u", rfid);
        ec = MC::EC_SUCC;

     NT:
        notify_->Notify(ec, rfid_str);
        Log::WriteLog(LL_DEBUG, "MC::GetRFIDEv::SpecificExecute->获取rfid, ec: %s",
                      MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int slot_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::GetRFID(int slot, NotifyResult *notify) {
    BaseEvent* ev = new (std::nothrow) GetRFIDEv(
            "获取rfid",
            slot,
            notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

////////////////////////// 设备状态 ////////////////////////////////////

class GetDevStatusEv : public MC::BaseEvent {
public:
    GetDevStatusEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char status_str[11] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        unsigned char dev_status[24] = { 0 };
        int ret = ::FGetDevStatus(dev_status, sizeof(dev_status));
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "GetDevStatusEv::Execute->查询设备状态失败, er: %d", 
                ret);
            exception_ = MC::EC_QUERY_DEV_STATUS_FAIL;
            goto NT;
        }

        // 设备状态机描述
        // char* status_des[] = {
        //     "未初始化",
        //     "启动自检",
        //     "检测章",
        //     "空闲状态",
        //     "测试模式",
        //     "故障模式",
        //     "盖章模式",
        //     "维护模式"
        // };
//         switch (dev_status[1]) {
//         case 0:
//             exception_ = MC::EC_UN_INITIATION;
//             break;
//         case 1:
//             exception_ = MC::EC_SELF_EXAMIN;
//             break;
//         case 2:
//             exception_ = MC::EC_EXAMIN_STAMPER;
//             break;
//         case 3:
//             exception_ = MC::EC_SUCC;
//             break;
//         case 4:
//             exception_ = MC::EC_IN_TEST;
//             break;
//         case 5:
//             exception_ = MC::EC_IN_BREAK_DOWN;
//             break;
//         case 6:
//             exception_ = MC::EC_IN_STAMPING;
//             break;
//         case 7:
//             exception_ = MC::EC_IN_MAINTAIN;
//             break;
//         default:
//             Log::WriteLog(LL_ERROR, "GetDevStatusEv::Execute->设备处于未知状态");
//             exception_ = MC::EC_FAIL;
//             break;
//         }

        sprintf(status_str, "%d", (int)dev_status[1]);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, status_str);
        Log::WriteLog(LL_DEBUG, "MC::GetDevStatusEv->获取设备状态, ec: %s, 状态: %s",
            MC::ErrorMsg[ec].c_str(),
            status_str);
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::GetDevStatus(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) GetDevStatusEv(
        "获取设备状态",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 坐标转换 /////////////////////////////////

class CoordCvtEv : public MC::BaseEvent {
public:
    CoordCvtEv(std::string des, int x, int y, MC::NotifyResult* notify) :
        BaseEvent(des),
        x_img_(x),
        y_img_(y),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char x_str[11] = { 0 };
        char y_str[11] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (x_img_ <= 0 || y_img_ <= 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        MC::Point* dev_pt = MC::ImgPro::GetInst()->GetSealCoord(x_img_, y_img_);
        sprintf(x_str, "%d", dev_pt->x);
        sprintf(y_str, "%d", dev_pt->y);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, x_str, y_str);
        Log::WriteLog(LL_DEBUG, "MC::CoordCvtEv::SpecificExecute->坐标转换, ec: %s, x: %s, y: %s",
            MC::ErrorMsg[ec].c_str(),
            x_str,
            y_str);
        delete this;
    }

private:
    int x_img_;
    int y_img_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::CvtCoord(int x_img, int y_img, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) CoordCvtEv(
        "坐标转换",
        x_img,
        y_img,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 写图像转换倍率 /////////////////////////////////

class WriteRatioEv : public MC::BaseEvent {
public:
    WriteRatioEv(std::string des, float x, float y, MC::NotifyResult* notify) :
        BaseEvent(des),
        x_(x),
        y_(y),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        if (x_ < 0 || y_ < 0) {
            ec = MC::EC_INVALID_PARAMETER;
            goto NT;
        }

        int ret = WriteImageConvRatio(&x_, &y_);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::WriteRatioEv->写倍率, ec: %s, x: %f, y: %f",
            MC::ErrorMsg[ec].c_str(),
            x_,
            y_);
        delete this;
    }

private:
    float x_;
    float y_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::WriteRatio(float x, float y, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) WriteRatioEv(
        "写倍率",
        x,
        y,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 读图像转换倍率 /////////////////////////////////

class ReadRatioEv : public MC::BaseEvent {
public:
    ReadRatioEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        float x = 0.f;
        float y = 0.f;
        char x_str[24] = { 0 };
        char y_str[24] = { 0 };
        std::stringstream stream;
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = ReadImageConvRatio(&x, &y);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        stream << std::fixed << std::setprecision(2) << x;
        sprintf(x_str, "%s", stream.str());
        
        stream.clear();
        stream << std::fixed << std::setprecision(2) << y;
        sprintf(y_str, "%s", stream.str());

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, x_str, y_str);
        Log::WriteLog(LL_DEBUG, "MC::ReadRatioEv->读倍率, ec: %s, x: %f, y: %f",
            MC::ErrorMsg[ec].c_str(),
            x,
            y);
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::ReadRatio(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) ReadRatioEv(
        "读倍率",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 写校准点 /////////////////////////////////

class WriteCaliEv : public MC::BaseEvent {
public:
    WriteCaliEv(std::string des, unsigned short* pts, unsigned short len, 
        MC::NotifyResult* notify) :
        BaseEvent(des),
        pts_(pts),
        len_(len),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = WriteCalibrationPoint(pts_, (unsigned char)len_);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::WriteCaliEv->写校准点, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    unsigned short* pts_;
    unsigned short len_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::WriteCalibration(unsigned short* pts, unsigned short len, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) WriteCaliEv(
        "写校准点",
        pts,
        len,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 读校准点 /////////////////////////////////

class ReadCaliEv : public MC::BaseEvent {
public:
    ReadCaliEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        const unsigned short len = 10;
        unsigned short pts[len] = { 0 };
        char pts_addr[64] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = CalibrationPoint(pts, len);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf(pts_addr, "%p", pts);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, pts_addr);
        Log::WriteLog(LL_DEBUG, "MC::ReadCaliEv->读校准点, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::ReadCalibration(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) ReadCaliEv(
        "读校准点",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 顶盖门状态查询 /////////////////////////////////

class QueryTopEv : public MC::BaseEvent {
public:
    QueryTopEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        char top_str[11] = { 0 };
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        char doors[4] = { 0 };
        int ret = FGetDoorsPresent(doors, sizeof(doors));
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(top_str, "%d", (int)doors[3]);
        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec, top_str);
        Log::WriteLog(LL_DEBUG, "MC::QueryTopEv->顶盖门状态, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::QueryTop(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) QueryTopEv(
        "顶盖门状态",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 退出维护模式 /////////////////////////////////

class ExitMaintainEv : public MC::BaseEvent {
public:
    ExitMaintainEv(std::string des, MC::NotifyResult* notify) :
        BaseEvent(des),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = FQuitMaintainMode();
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::ExitMaintainEv->退出维护模式, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::ExitMaintain(NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) ExitMaintainEv(
        "退出维护模式",
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 开始预览 /////////////////////////////////

class StartPreviewEv : public MC::BaseEvent {
public:
    StartPreviewEv(std::string des, int which, int width, int height, int hwnd, 
        MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        width_(width),
        height_(height),
        hwnd_(hwnd),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = StartPreview(
            (CAMERAINDEX)which_,
            width_,
            height_, 
            (HWND)hwnd_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::StartPreviewEv->开始预览失败, er: %d", ret);
            ec = MC::EC_START_PREVIEW_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::StartPreviewEv->开始预览, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;
    int width_;
    int height_;
    int hwnd_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::StartPreview(int which, int width, int height, int hwnd, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) StartPreviewEv(
        "开始预览",
        which,
        width,
        height,
        hwnd,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}

///////////////////////////// 停止预览 /////////////////////////////////

class StopPreviewEv : public MC::BaseEvent {
public:
    StopPreviewEv(std::string des, int which, MC::NotifyResult* notify) :
        BaseEvent(des),
        which_(which),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (MC::EC_SUCC != ec)
            goto NT;

        int ret = StopPreview((CAMERAINDEX)which_);
        if (0 != ret) {
            Log::WriteLog(LL_ERROR, "MC::StopPreviewEv->停止预览失败, er: %d", ret);
            ec = MC::EC_STOP_PREVIEW_FAIL;
            goto NT;
        }

        ec = MC::EC_SUCC;

    NT:
        notify_->Notify(ec);
        Log::WriteLog(LL_DEBUG, "MC::StopPreviewEv->停止预览, ec: %s",
            MC::ErrorMsg[ec].c_str());
        delete this;
    }

private:
    int which_;

    MC::NotifyResult*   notify_;
};

void MC::STSealAPI::StopPreview(int which, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) StopPreviewEv(
        "停止预览",
        which,
        notify);
    if (NULL == ev)
        notify->Notify(MC::EC_ALLOCATE_FAILURE);

    EventCPUCore::GetInstance()->PostEvent(ev);
}
