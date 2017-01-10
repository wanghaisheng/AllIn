#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include "api_set.h"
#include "agent_cmd.h"
#include "log.h"
#include "cnn.h"
#include "common_definitions.h"

boost::mutex api_map_mtx;  // 全局变量, 互斥锁, 保护通知对象map

void AsynAPISet::ParseCmd(BaseCmd* cmd, char* chBuf)
{
    memcpy(cmd->xs_.buf_, chBuf, CMD_BUF_SIZE);
    cmd->Unser();
}

void* AsynAPISet::LookupSendTime(const std::string& send_time)
{
    void* ptr = NULL;
    boost::lock_guard<boost::mutex> lk(api_map_mtx);
    std::map<std::string, void*>::iterator it = api_maps_.begin();
    for (; it != api_maps_.end(); ++it) {
        if (it->first == send_time) {
            ptr = it->second;
            api_maps_.erase(it);

            Log::WriteLog(LL_DEBUG, "AsynAPISet::LookupSendTime->发送时间: %s, 通知对象值: %d",
                send_time.c_str(),
                ptr);
            break;
        }
    }

    return ptr;
}

void AsynAPISet::HandleQueryMachine(char* chBuf)
{
    QueryMachineCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryMachine->cmd:%s, sn:%s, ret:%d",
        cmd_des[cmd.ct_].c_str(),
        cmd.sn_,
        cmd.ret_);

    QueryMachineNT* nt = (QueryMachineNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.sn_, cmd.ret_);
}

void AsynAPISet::HandleSetMachine(char* chBuf)
{
    SetMachineCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSetMachine->cmd:%s, sn:%s, ret:%d",
        cmd_des[cmd.ct_].c_str(),
        cmd.sn_,
        cmd.ret_);

    SetMachineNT* nt = (SetMachineNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.sn_, cmd.ret_);
}

void AsynAPISet::HandleInitMachine(char* chBuf)
{
    InitMachineCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleInitMachine->cmd:%s, key:%s, ret:%d",
        cmd_des[cmd.ct_].c_str(),
        cmd.key_,
        cmd.ret_);

    InitMachineNT* nt = (InitMachineNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.key_, cmd.ret_);
    else
        printf("APISet::HandleInitMachine->通知对象为NULL.\n");
}

void AsynAPISet::HandleBindMac(char* chBuf)
{
    BindMACCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "APISet::HandleBindMac->cmd:%s, mac:%s, ret:%d",
        cmd_des[cmd.ct_].c_str(),
        cmd.mac_,
        cmd.ret_);

    BindMACNT* nt = (BindMACNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.mac_, cmd.ret_);
}

void AsynAPISet::HandleUnbindMAC(char* chBuf)
{
    UnbindCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleUnbindMAC->cmd: %s, mac: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.mac_,
        cmd.ret_);

    UnbindMACNT* nt = (UnbindMACNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.mac_, cmd.ret_);
}

void AsynAPISet::HandlePrepareStamp(char* chBuf)
{
    PrepareStampCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandlePrepareStamp->cmd: %s, 章槽号: %d, "
        "超时时间: %d, 任务ID: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.stamper_id_,
        cmd.timeout_,
        cmd.task_id_,
        cmd.ret_);

    PrepareStampNT* nt = (PrepareStampNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.stamper_id_, cmd.timeout_, cmd.task_id_, cmd.ret_);
}

void AsynAPISet::HandleQueryPaper(char* chBuf)
{
    ViewPaperCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryPaper->cmd: %s, 进纸门状态: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.status_,
        cmd.ret_);

    QueryPaperNT* nt = (QueryPaperNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_, cmd.ret_);
}

void AsynAPISet::HandleSnapshot(char* chBuf)
{
    SnapshotCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSnapshot->cmd: %s, 原图路径: %s, "
        "切图路径: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.original_path_,
        cmd.cut_path_,
        cmd.ret_);

    SnapshotNT* nt = (SnapshotNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.original_dpi_, cmd.cut_dpi_, cmd.original_path_, cmd.cut_path_, cmd.ret_);
}

void AsynAPISet::HandleMergePhoto(char* chBuf)
{
    SynthesizePhotoCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleMergePhoto->cmd: %s, 图1路径: %s, "
        "图2路径: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.photo1_,
        cmd.photo2_,
        cmd.ret_);

    MergePhotoNT* nt = (MergePhotoNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.photo1_, cmd.photo2_, cmd.merged_, cmd.ret_);
}

void AsynAPISet::HandleRecognition(char* chBuf)
{
    RecognitionCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleRecognition->cmd: %s, 图片路径: %s, "
        "模版ID: %s, 追溯码: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.path_,
        cmd.template_id_,
        cmd.trace_num_,
        cmd.ret_);

    RecognizeNT* nt = (RecognizeNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.path_, cmd.template_id_, cmd.trace_num_, cmd.ret_);
}

void AsynAPISet::HandleIdentify(char* chBuf)
{
    IdentifyElementCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleIdentify->cmd: %s, 图片路径: %s, "
        "识别结果: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.path_,
        cmd.content_str_,
        cmd.ret_);

    IdentifyNT* nt = (IdentifyNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.path_, cmd.x_, cmd.y_, cmd.width_, cmd.height_, cmd.angle_,
        cmd.content_str_, cmd.ret_);
}

void AsynAPISet::HandleOrdinary(char* chBuf)
{
    OridinaryStampCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleOrdinary->cmd: %s, 任务ID: %s, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.task_id_,
        cmd.ret_);

    OrdinaryStampNT* nt = (OrdinaryStampNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.task_id_, cmd.type_, cmd.stamper_num_, cmd.x_, cmd.y_, cmd.angle_, cmd.ret_);
}

void AsynAPISet::HandleAuto(char* chBuf)
{
    AutoStampCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleAuto->cmd: %s, 任务ID: %s, 类型: %s, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.task_id_,
        cmd.type_,
        cmd.ret_);

    AutoStampNT* nt = (AutoStampNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.task_id_, cmd.type_, cmd.stamper_num_, cmd.ret_);
}

void AsynAPISet::HandleFinish(char* chBuf)
{
    FinishStampCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleFinish->cmd: %s, 任务ID: %s, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.task_id_,
        cmd.ret_);

    FinishStampNT* nt = (FinishStampNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.task_id_, cmd.ret_);
}

void AsynAPISet::HandleRelease(char* chBuf)
{
    ReleaseStamperCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleRelease->cmd: %s, 印控机编号: %s, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.stamp_id_,
        cmd.ret_);

    ReleaseStampNT* nt = (ReleaseStampNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.stamp_id_, cmd.ret_);
    else
        Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleRelease->通知对象为null");
}

void AsynAPISet::HandleGetError(char* chBuf)
{
    GetErrorCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleGetError->cmd: %s, 错误码: %d, "
        "错误信息: %s, 解决方案: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.err_,
        cmd.err_msg_,
        cmd.err_resolver_,
        cmd.ret_);

    GetErrorNT* nt = (GetErrorNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.err_, cmd.err_msg_, cmd.err_resolver_, cmd.ret_);
    else
        Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleGetError->通知对象为null");
}

void AsynAPISet::HandleCalibrate(char* chBuf)
{
    CalibrateCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCalibrate->cmd: %s, 操作: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.slot_,
        cmd.ret_);

    CalibrationNT* nt = (CalibrationNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.slot_, cmd.ret_);
}

void AsynAPISet::HandleQueryStampers(char* chBuf)
{
    QueryStampersCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryStampers->cmd: %s, 操作: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.stamper_status_,
        cmd.ret_);

    QueryStampersNT* nt = (QueryStampersNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.stamper_status_, cmd.ret_);
}

void AsynAPISet::HandleQuerySafe(char* chBuf)
{
    QuerySafeCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSafeControl->cmd: %s, 安全门: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.status_,
        cmd.ret_);

    QuerySafeNT* nt = (QuerySafeNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_, cmd.ret_);
}

void AsynAPISet::HandleSafeControl(char* chBuf)
{
    SafeCtrlCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSafeControl->cmd: %s, 操作: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ctrl_,
        cmd.ret_);

    CtrlSafeNT* nt = (CtrlSafeNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ctrl_, cmd.ret_);
}

void AsynAPISet::HandleBeepControl(char* chBuf)
{
    BeepCtrlCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleBeepControl->cmd: %s, 操作: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ctrl_,
        cmd.ret_);

    CtrlBeepNT* nt = (CtrlBeepNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ctrl_, cmd.ret_);
}

void AsynAPISet::HandleQuerySlot(char* chBuf)
{
    QuerySlotCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQuerySlot->cmd: %s, 卡槽数: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.num_,
        cmd.ret_);

    QuerySlotNT* nt = (QuerySlotNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.num_, cmd.ret_);
}

void AsynAPISet::HandleAlarmControl(char* chBuf)
{
    AlarmCtrlCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleAlarmControl->cmd: %s, 报警器类型: %d, "
        "操作: %d, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.alarm_,
        cmd.ctrl_,
        cmd.ret_);

    CtrlAlarmNT* nt = (CtrlAlarmNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.alarm_, cmd.ctrl_, cmd.ret_);
}

void AsynAPISet::HandleQueryMAC(char* chBuf)
{
    QueryMACCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryMAC->cmd: %s, mac1: %s, "
        "mac2: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.mac1_,
        cmd.mac2_,
        cmd.ret_);

    QueryMACNT* nt = (QueryMACNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.mac1_, cmd.mac2_, cmd.ret_);
}

void AsynAPISet::AsynErrorNotify(BaseCmd* cmd, enum MC::ErrorCode ec)
{
    switch (cmd->ct_) {
    case CT_INIT_MACHINE: {
        InitMachineNT* nt = (InitMachineNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(((InitMachineCmd*)cmd)->key_, ec);
    }
        break;
    case CT_BIND_MAC: {
        BindMACNT* nt = (BindMACNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(((BindMACCmd*)cmd)->mac_, ec);
    }
        break;
    case CT_UNBIND_MAC: {
        UnbindMACNT* nt = (UnbindMACNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(((UnbindCmd*)cmd)->mac_, ec);
    }
        break;
    case CT_PREPARE_STAMP: {
        PrepareStampNT* nt = (PrepareStampNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((PrepareStampCmd*)cmd)->stamper_id_, 
            ((PrepareStampCmd*)cmd)->timeout_, 
            "", 
            ec);
    }
        break;
    case CT_SNAPSHOT: {
        SnapshotNT* nt = (SnapshotNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((SnapshotCmd*)cmd)->original_dpi_,
            ((SnapshotCmd*)cmd)->cut_dpi_,
            ((SnapshotCmd*)cmd)->original_path_,
            ((SnapshotCmd*)cmd)->cut_path_,
            ec);
    }
        break;
    case CT_PHOTO_SYNTHESIS: {
        MergePhotoNT* nt = (MergePhotoNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((SynthesizePhotoCmd*)cmd)->photo1_, 
            ((SynthesizePhotoCmd*)cmd)->photo2_,
            "",
            ec);
    }
        break;
    case CT_RECOGNITION: {
        RecognizeNT* nt = (RecognizeNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((RecognitionCmd*)cmd)->path_,
            "",
            "",
            ec);
    }
        break;
    case CT_ELEMENT_IDENTI: {
        IdentifyNT* nt = (IdentifyNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((IdentifyElementCmd*)cmd)->path_,
            ((IdentifyElementCmd*)cmd)->x_, 
            ((IdentifyElementCmd*)cmd)->y_, 
            ((IdentifyElementCmd*)cmd)->width_, 
            ((IdentifyElementCmd*)cmd)->height_,
            ((IdentifyElementCmd*)cmd)->angle_,
            "", 
            ec);
    }
        break;
    case CT_ORDINARY_STAMP: {
        OrdinaryStampNT* nt = (OrdinaryStampNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((OridinaryStampCmd*)cmd)->task_id_, 
            ((OridinaryStampCmd*)cmd)->type_, 
            ((OridinaryStampCmd*)cmd)->stamper_num_,
            ((OridinaryStampCmd*)cmd)->x_, 
            ((OridinaryStampCmd*)cmd)->y_, 
            ((OridinaryStampCmd*)cmd)->angle_, 
            ec);
    }
        break;
    case CT_AUTO_STAMP: {
        AutoStampNT* nt = (AutoStampNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((AutoStampCmd*)cmd)->task_id_, 
            ((AutoStampCmd*)cmd)->type_, 
            ((AutoStampCmd*)cmd)->stamper_num_,
            ec);
    }
        break;
    case CT_FINISH_STAMP: {
        FinishStampNT* nt = (FinishStampNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(((FinishStampCmd*)cmd)->task_id_, ec);
    }
        break;
    case CT_RELEASE_STAMPER: {
        ReleaseStampNT* nt = (ReleaseStampNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(((ReleaseStamperCmd*)cmd)->stamp_id_, ec);
    }
        break;
    case CT_GET_ERROR: {
        GetErrorNT* nt = (GetErrorNT*)LookupSendTime(cmd->send_time_);
        nt->Notify(
            ((GetErrorCmd*)cmd)->err_,
            "",
            "", 
            ec);
    }
        break;
    default:
        break;
    }
}


//////////////////////////////////////////////////////////////////////////

int AsynAPISet::AsynQueryMachine(const QueryMachineNT* const nt)
{
    QueryMachineCmd* cmd = new QueryMachineCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSetMachine(const std::string& sn, SetMachineNT* nt)
{
    SetMachineCmd* cmd = new SetMachineCmd;
    strcpy_s(cmd->sn_, sn.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynInitMachine(const std::string& key, InitMachineNT* nt)
{
    InitMachineCmd* cmd = new InitMachineCmd;
    strcpy(cmd->key_, key.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynBindMAC(const std::string& mac, BindMACNT* nt)
{
    BindMACCmd* cmd = new BindMACCmd;
    strcpy(cmd->mac_, mac.c_str());
    InsertNotify(cmd->send_time_, nt);
    
    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynUnbindMAC(const std::string& mac, UnbindMACNT* nt)
{
    UnbindCmd* cmd = new UnbindCmd;
    strcpy(cmd->mac_, mac.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynPrepareStamp(int num, int timeout, PrepareStampNT* nt)
{
    PrepareStampCmd* cmd = new PrepareStampCmd;
    cmd->stamper_id_ = num;
    cmd->timeout_ = timeout;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryPaper(QueryPaperNT* nt)
{
    ViewPaperCmd* cmd = new ViewPaperCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSnapshot(
    int ori_dpi, 
    int cut_dpi, 
    const std::string& ori_path, 
    const std::string& cut_path,
    SnapshotNT* nt)
{
    SnapshotCmd* cmd = new SnapshotCmd;
    cmd->original_dpi_ = ori_dpi;
    cmd->cut_dpi_ = cut_dpi;
    strcpy_s(cmd->original_path_, ori_path.c_str());
    strcpy_s(cmd->cut_path_, cut_path.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynMergePhoto(
    const std::string& p1, 
    const std::string& p2, 
    const std::string& merged, 
    MergePhotoNT* nt)
{
    SynthesizePhotoCmd* cmd = new SynthesizePhotoCmd;
    strcpy_s(cmd->photo1_, p1.c_str());
    strcpy_s(cmd->photo2_, p2.c_str());
    strcpy_s(cmd->merged_, merged.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynRecognizeImage(const std::string& path, RecognizeNT* nt)
{
    RecognitionCmd* cmd = new RecognitionCmd;
    strcpy_s(cmd->path_, path.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynIdentifyElement(const std::string& path, int x, int y, int width, int height,
    int angle, IdentifyNT* nt)
{
    IdentifyElementCmd* cmd = new IdentifyElementCmd;
    strcpy_s(cmd->path_, path.c_str());
    cmd->x_ = x;
    cmd->y_ = y;
    cmd->width_ = width;
    cmd->height_ = height;
    cmd->angle_ = angle;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::InsertNotify(const std::string& st, const void* const nt)
{
    api_map_mtx.lock();
    api_maps_.insert(std::make_pair(st, (void*)nt));
    Log::WriteLog(LL_DEBUG, "AsynAPISet::InsertNotify->通知对象值: %d, 发送时间: %s",
        nt,
        st.c_str());
    api_map_mtx.unlock();
}

int AsynAPISet::AsynOrdinaryStamp(
    const std::string& task,
    const std::string& voucher, 
    int num, 
    int ink,
    int x, 
    int y, 
    int angle,
     OrdinaryStampNT* nt)
{
    OridinaryStampCmd* cmd = new OridinaryStampCmd;
    strcpy_s(cmd->task_id_, task.c_str());
    strcpy_s(cmd->type_, voucher.c_str());
    cmd->stamper_num_ = num;
    cmd->ink_ = ink;
    cmd->x_ = x;
    cmd->y_ = y;
    cmd->angle_ = angle;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynAutoStamp(const std::string& task,
    const std::string& voucher, int num, AutoStampNT* nt)
{
    AutoStampCmd* cmd = new AutoStampCmd;
    strcpy_s(cmd->task_id_, task.c_str());
    strcpy_s(cmd->type_, voucher.c_str());
    cmd->stamper_num_ = num;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynFinishStamp(const std::string& task, FinishStampNT* nt)
{
    FinishStampCmd* cmd = new FinishStampCmd;
    strcpy_s(cmd->task_id_, task.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynReleaseStamp(const std::string& machine, ReleaseStampNT* nt)
{
    ReleaseStamperCmd* cmd = new ReleaseStamperCmd;
    strcpy_s(cmd->stamp_id_, machine.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynGetError(int err_code, GetErrorNT* nt)
{
    GetErrorCmd* cmd = new GetErrorCmd;
    cmd->err_ = err_code;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynCalibrate(int slot, CalibrationNT* nt)
{
    CalibrateCmd* cmd = new CalibrateCmd;

    cmd->slot_ = slot;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryStampers(QueryStampersNT* nt)
{
    QueryStampersCmd* cmd = new QueryStampersCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQuerySafe(QuerySafeNT* nt)
{
    QuerySafeCmd* cmd = new QuerySafeCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSafeControl(int ctrl, CtrlSafeNT* nt)
{
    SafeCtrlCmd* cmd = new SafeCtrlCmd;
    cmd->ctrl_ = ctrl;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynBeepControl(int ctrl, CtrlBeepNT* nt)
{
    BeepCtrlCmd* cmd = new BeepCtrlCmd;
    cmd->ctrl_ = ctrl;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQuerySlot(QuerySlotNT* nt)
{
    QuerySlotCmd* cmd = new QuerySlotCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynAlarmControl(int alarm, int ctrl, CtrlAlarmNT* nt)
{
    AlarmCtrlCmd* cmd = new AlarmCtrlCmd;
    cmd->alarm_ = alarm;
    cmd->ctrl_ = ctrl;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryMAC(QueryMACNT* nt)
{
    QueryMACCmd* cmd = new QueryMACCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}
