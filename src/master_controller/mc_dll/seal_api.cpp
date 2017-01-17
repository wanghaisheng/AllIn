﻿#include "stdafx.h"
#include <stdlib.h>
#include <cmath>
#include <vector>
#include "event_cpu.h"
#include "tool.h"
#include "task_mgr.h"
#include "seal_api.h"

MC::STSealAPI* MC::STSealAPI::inst_ = NULL;

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

        // 中行-获取印控仪编号
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

        // 中行-设置印控机编号
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

        //中行-初始化印控机
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
        Log::WriteLog(LL_DEBUG, "MC::BindMACEv::SpecificExecute->绑定MAC地址, ec: %d, 待绑定MAC: %s, "
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
        ret = SetPaperDoor(timeout_);
        if (-1 == ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
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

        // 中行-查询进纸门
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
    SnapshotEv(std::string des, int original_dpi, int cut_dpi, 
        const std::string& ori_path, const std::string& cut_path,
        MC::NotifyResult* notify)
        : BaseEvent(des),
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
            PAPERCAMERA,
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
    int ori_dpi_;
    int cut_dpi_;
    std::string ori_;
    std::string cut_;

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::Snapshot(
    int original_dpi, 
    int cut_dpi, 
    const std::string& ori_path,
    const std::string& cut_path,
    NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) SnapshotEv(
        "拍照", 
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

        // 中行-照片合成
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

        // 中行-版面验证码识别
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

        // 中行-要素识别
        int ret = MC::ImgPro::GetInst()->IdentifyArea(path_, x_, y_, width_, height_, angle_, result);
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
        int x, int y, int angle, MC::NotifyResult* notify): BaseEvent(des), 
        task_(t), 
        voucher_(v), 
        num_(num),
        ink_(ink),
        x_(x),
        y_(y),
        angle_(angle),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 中行-普通用印
        MC::TaskState ts = MC::TaskMgr::GetInst()->QueryTaskState(task_);
        if (ts ==  MC::TS_ALIVE) {
            // 将原图用印坐标(像素)转换为设备用印坐标(毫米)
            MC::Point* ptSeal = MC::ImgPro::GetInst()->GetSealCoord(x_, y_);
            unsigned int rfid;
            int ret = GetStamperID(num_ - 1, rfid);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OridinaryEv::SpecificExecute->获取章槽号%d的RFID失败, er: %d",
                              num_,
                              ret);
                goto NT;
            }

            STAMPERPARAM pa;
            memcpy(&pa.seal, &rfid, sizeof(rfid));
            pa.serial_number = atoi(task_.c_str());
            pa.isPadInk = ink_;
            pa.x_point = ptSeal->x;
            pa.y_point = ptSeal->y;
            pa.w_time = MC::SvrConfig::GetInst()->wait_time_;
            pa.angle = 270;
            pa.type = 0;

            Log::WriteLog(LL_DEBUG, "MC::OridinaryEv::SpecificExecute->普通用印, 物理用印点(%d, %d)",
                ptSeal->x,
                ptSeal->y);
            ret = FStartStamperstrc(&pa);
            if (0 != ret) {
                ec = MC::EC_DRIVER_FAIL;
                Log::WriteLog(LL_ERROR, "MC::OridinaryEv::SpecificExecute->发起盖章失败, er: %d",
                    ret);
                goto NT;  
            }

            // 成功发起盖章, 标记当前任务号
            MC::TaskMgr::GetInst()->MarkUsed(task_);
        } else if (ts == MC::TS_USED) {
            ec = MC::EC_TASK_CONSUMED;
            goto NT;
        } else {
            ec = MC::EC_TASK_NON_EXIST;
            goto NT;
        }

        ec = MC::EC_SUCC;

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
    int         x_;         // 盖章位置x坐标, 原始图片中的像素
    int         y_;
    int         angle_;

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

        // 中行-自动用印
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

        // 中行-用印结束
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

        // 中行-释放印控机
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
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        if (err_ < 0 || err_ > MC::EC_MAX) {
            msg = "未定义的错误码";
            goto NT;
        }

        // 中行-获取错误信息
        msg = MC::GetErrMsg((MC::ErrorCode)err_);
        resolver = MC::GetErrResolver((MC::ErrorCode)err_);
        ec = MC::EC_SUCC;

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

        // 中行-校准印章
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

        // 中行-印章状态查询
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

        // 中行-安全门状态查询
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
        Log::WriteLog(LL_DEBUG, "MC::QuerySafeEv::SpecificExecute->安全门状态查询, ec: %s, 安全门状态: %s",
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

        // 中行-开关安全门
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
            // 安全门已关闭
            if (doors[1] == 0) {
                ec = MC::EC_SAFE_CLOSED;
                Log::WriteLog(LL_DEBUG, "MC::OperateSafeEv::SpecificExecute->安全门已关闭");
                goto NT;
            }

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
    OperateBeepEv(std::string des, int operation, MC::NotifyResult* notify) :
        BaseEvent(des),
        operation_(operation),
        notify_(notify) {

    }

    virtual void SpecificExecute() {
        MC::ErrorCode ec = exception_;
        if (ec != MC::EC_SUCC)
            goto NT;

        // 中行-蜂鸣器开关
        //0x16, 蜂鸣器控制
        //beep      --- 0 关闭; 1 长鸣; 2 间隔响
        //interval  --- 当beep=2时该值有效, 间隔响时常(单位秒)
        int ret = FBeepCtrl(operation_, 0);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
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

    MC::NotifyResult* notify_;
};

void MC::STSealAPI::OperateBeep(int operation, NotifyResult* notify)
{
    BaseEvent* ev = new (std::nothrow) OperateBeepEv(
        "蜂鸣器开关",
        operation,
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

        // 中行-卡槽数量查询
        unsigned int rfids[7] = { 0 };
        unsigned char stampers = 0;
        int ret = ReadAllRFID(rfids, 7, &stampers);
        if (0 != ret) {
            ec = MC::EC_DRIVER_FAIL;
            goto NT;
        }

        sprintf_s(slots_str, "%d", stampers - 1);
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

        // 中行-报警器控制
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
        Log::WriteLog(LL_DEBUG, "MC::QuerySlotEv::SpecificExecute->报警器控制, ec: %s, 报警器类型: %s, "
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

        // 中行-查询MAC地址
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

//////////////////////////////////////////////////////////////////////////