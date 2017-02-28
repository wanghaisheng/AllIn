#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include "log.h"
#include "cnn.h"
#include "api_set.h"

boost::mutex api_map_mtx;  // 全局变量, 互斥锁, 保护通知对象map

void AsynAPISet::ParseCmd(BaseCmd* cmd, char* chBuf)
{
    memcpy(cmd->xs_.buf_, chBuf, CMD_BUF_SIZE);
    cmd->Unser();
}

void AsynAPISet::DeleteNotify(void* nt)
{
    boost::lock_guard<boost::mutex> lk(api_map_mtx);
    std::map<void*, std::string>::iterator nt_it = nt_maps_.find(nt);
    if (nt_it != nt_maps_.end()) {
        std::map<std::string, void*>::iterator it = api_maps_.begin();
        for (; it != api_maps_.end(); ++it) {
            if (it->first == nt_it->second) {
                Log::WriteLog(LL_DEBUG, "AsynAPISet::DeleteNotify->发送时间: %s, 通知对象值: %d",
                    nt_it->second.c_str(),
                    it->second);

                api_maps_.erase(it);
                break;
            }
        }
    }
}

void* AsynAPISet::LookupSendTime(const std::string& send_time)
{
    void* ptr = NULL;
    boost::lock_guard<boost::mutex> lk(api_map_mtx);
    std::map<std::string, void*>::iterator it = api_maps_.begin();
    for (; it != api_maps_.end(); ++it) {
        if (it->first == send_time) {
            ptr = it->second;
            
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

void AsynAPISet::HandleRecogModelPoint(char* chBuf)
{
    RecoModelTypeEtcCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleRecogModelPoint->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    RecogModelPointNT* nt = (RecogModelPointNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.model_result_, cmd.angle_, cmd.x_, cmd.y_, cmd.ret_);
}

void AsynAPISet::HandleSearchStamp(char* chBuf)
{
    SearchStampPointCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSearchStamp->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    SearchStampPointNT* nt = (SearchStampPointNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.out_x_, cmd.out_y_, cmd.out_angle_, cmd.ret_);

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
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryStampers->cmd: %s, 章状态: %s, "
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

void AsynAPISet::HandleQueryAlarm(char* chBuf)
{
    QueryAlarmCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryAlarm->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    QueryAlarmNT* nt = (QueryAlarmNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.door_, cmd.vibration_, cmd.ret_);
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

void AsynAPISet::HandleLock(char* chBuf)
{
    LockCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleLock->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    LockNT* nt = (LockNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleUnlock(char* chBuf)
{
    UnlockCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleUnlock->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    UnlockNT* nt = (UnlockNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleQueryLock(char* chBuf)
{
    QueryLockCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryLock->cmd: %s, whether_lock: %d, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.status_,
        cmd.ret_);

    QueryLockNT* nt = (QueryLockNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_, cmd.ret_);
}

void AsynAPISet::HandleOpenCnn(char* chBuf)
{
    OpenCnnCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleOpenCnn->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    OpenCnnNT* nt = (OpenCnnNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleCloseCnn(char* chBuf)
{
    CloseCnnCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCloseCnn->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CloseCnnNT* nt = (CloseCnnNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleQueryCnn(char* chBuf)
{
    QueryCnnCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryCnn->cmd: %s, "
        "status: %d, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.status_,
        cmd.ret_);

    QueryCnnNT* nt = (QueryCnnNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_, cmd.ret_);
}

void AsynAPISet::HandleSetSideAlarm(char* chBuf)
{
    SideDoorAlarmCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSetSideAlarm->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    SideDoorAlarmNT* nt = (SideDoorAlarmNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleGetModel(char* chBuf)
{
    GetDevModelCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleGetModel->cmd: %s, model: %s, "
        "ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.model_,
        cmd.ret_);

    DevModelNT* nt = (DevModelNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.model_, cmd.ret_);
}

void AsynAPISet::HandleOpenPaper(char* chBuf)
{
    OpenPaperCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleOpenPaper->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    OpenPaperNT* nt = (OpenPaperNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleCtrlLed(char* chBuf)
{
    CtrlLEDCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCtrlLed->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CtrlLedNT* nt = (CtrlLedNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleCheckParam(char* chBuf)
{
    CheckParamCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCheckParam->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CheckParamNT* nt = (CheckParamNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleOpenCamera(char* chBuf)
{
    OpenCameraCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleOpenCamera->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    OpenCameraNT* nt = (OpenCameraNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleCloseCamera(char* chBuf)
{
    CloseCameraCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCloseCamera->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CloseCameraNT* nt = (CloseCameraNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleQueryCamera(char* chBuf)
{
    QueryCameraStatusCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryCamera->cmd: %s, which: %d, "
        "status: %d, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.which_,
        cmd.status_,
        cmd.ret_);

    QueryCameraNT* nt = (QueryCameraNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.which_, cmd.status_, cmd.ret_);
}

void AsynAPISet::HandleSetResolution(char* chBuf)
{
    SetResolutionCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSetResolution->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    SetResolutionNT* nt = (SetResolutionNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleSetDPI(char* chBuf)
{
    SetDPICmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSetDPI->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    SetDPIValueNT* nt = (SetDPIValueNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleSetProperty(char* chBuf)
{
    SetPropertyCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleSetProperty->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    SetPropertyNT* nt = (SetPropertyNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleRecordVideo(char* chBuf)
{
    RecordVideoCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleRecordVideo->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    RecordVideoNT* nt = (RecordVideoNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleStopRecordVideo(char* chBuf)
{
    StopRecordVideoCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleStopRecordVideo->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    StopRecordVideoNT* nt = (StopRecordVideoNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

void AsynAPISet::HandleGetRFID(char *chBuf) {
    GetRFIDCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleGetRFID->cmd: %s, ret: %d",
                  cmd_des[cmd.ct_].c_str(),
                  cmd.ret_);

    GetRFIDNT* nt = (GetRFIDNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.rfid_, cmd.ret_);
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
    int which,
    int ori_dpi, 
    int cut_dpi, 
    const std::string& ori_path, 
    const std::string& cut_path,
    SnapshotNT* nt)
{
    SnapshotCmd* cmd = new SnapshotCmd;
    cmd->which_ = which;
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

int AsynAPISet::AsynRecogModelPoint(
    const std::string& path,
    RecogModelPointNT* nt) {
    RecoModelTypeEtcCmd* cmd = new RecoModelTypeEtcCmd;
    strcpy_s(cmd->path_, path.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSearchStampPoint(
    const std::string& img,
    int x,
    int y,
    double angle,
    SearchStampPointNT* nt) {
    SearchStampPointCmd* cmd = new SearchStampPointCmd;
    strcpy_s(cmd->src_, img.c_str());
    cmd->in_x_ = x;
    cmd->in_y_ = y;
    cmd->in_angle_ = angle;
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
    nt_maps_.insert(std::make_pair((void*)nt, st));
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
    int type,
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
    cmd->seal_type_ = type;
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

int AsynAPISet::AsynBeepControl(int ctrl, int type, int interval, CtrlBeepNT* nt)
{
    BeepCtrlCmd* cmd = new BeepCtrlCmd;
    cmd->ctrl_ = ctrl;
    cmd->type_ = type;
    cmd->interval_ = interval;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQuerySlot(QuerySlotNT* nt)
{
    QuerySlotCmd* cmd = new QuerySlotCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryAlarm(QueryAlarmNT* nt)
{
    QueryAlarmCmd* cmd = new QueryAlarmCmd;
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

int AsynAPISet::AsynLock(LockNT* nt)
{
    LockCmd* cmd = new LockCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynUnlock(UnlockNT* nt)
{
    UnlockCmd* cmd = new UnlockCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryLock(QueryLockNT* nt)
{
    QueryLockCmd* cmd = new QueryLockCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynOpen(OpenCnnNT* nt)
{
    OpenCnnCmd* cmd = new OpenCnnCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynClose(CloseCnnNT* nt)
{
    CloseCnnCmd* cmd = new CloseCnnCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryCnn(QueryCnnNT* nt)
{
    QueryCnnCmd* cmd = new QueryCnnCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSetSideAlarm(int keep, int timeout, SideDoorAlarmNT* nt)
{
    SideDoorAlarmCmd* cmd = new SideDoorAlarmCmd;
    cmd->keep_open_ = keep;
    cmd->timeout_ = timeout;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryModel(DevModelNT* nt)
{
    GetDevModelCmd* cmd = new GetDevModelCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynOpenPaper(int timeout, OpenPaperNT* nt)
{
    OpenPaperCmd* cmd = new OpenPaperCmd;
    cmd->timeout_ = timeout;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynCtrlLed(int which, int ctrl, int value, CtrlLedNT* nt)
{
    CtrlLEDCmd* cmd = new CtrlLEDCmd;
    cmd->which_ = which;
    cmd->switch_ = ctrl;
    cmd->value_ = value;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynCheckParam(int x, int y, int angle, CheckParamNT* nt)
{
    CheckParamCmd* cmd = new CheckParamCmd;
    cmd->x_ = x;
    cmd->y_ = y;
    cmd->angle_ = angle;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynOpenCamera(int which, OpenCameraNT* nt)
{
    OpenCameraCmd* cmd = new OpenCameraCmd;
    cmd->which_ = which;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynCloseCamera(int which, CloseCameraNT* nt)
{
    CloseCameraCmd* cmd = new CloseCameraCmd;
    cmd->which_ = which;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynQueryCamera(int which, QueryCameraNT* nt)
{
    QueryCameraStatusCmd* cmd = new QueryCameraStatusCmd;
    cmd->which_ = which;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSetResolution(int which, int x, int y, SetResolutionNT* nt)
{
    SetResolutionCmd* cmd = new SetResolutionCmd;
    cmd->which_ = which;
    cmd->x_ = x;
    cmd->y_ = y;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynSetDPI(int which, int x, int y, SetDPIValueNT* nt)
{
    SetDPICmd* cmd = new SetDPICmd;
    cmd->which_ = which;
    cmd->dpi_x_ = x;
    cmd->dpi_y_ = y;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);   
}

int AsynAPISet::AsynSetProperty(int which, SetPropertyNT* nt)
{
    SetPropertyCmd* cmd = new SetPropertyCmd;
    cmd->which_ = which;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynStartRecordVideo(int which, const std::string& path, RecordVideoNT* nt)
{
    RecordVideoCmd* cmd = new RecordVideoCmd;
    cmd->which_ = which;
    strcpy(cmd->path_, path.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynStopRecordVideo(int which, const std::string& path, StopRecordVideoNT* nt)
{
    StopRecordVideoCmd* cmd = new StopRecordVideoCmd;
    cmd->which_ = which;
    strcpy(cmd->path_, path.c_str());
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynGetRFID(int slot, GetRFIDNT *nt) {
    GetRFIDCmd* cmd = new GetRFIDCmd;
    cmd->slot_ = slot;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

int AsynAPISet::AsynGetStatus(GetStatusNT* nt)
{
    GetDevStatusCmd* cmd = new GetDevStatusCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleGetStatus(char* chBuf)
{
    GetDevStatusCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleGetStatus->cmd: %s, status: %d, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.status_code_,
        cmd.ret_);

    GetStatusNT* nt = (GetStatusNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_code_, cmd.ret_);
}

int AsynAPISet::AsynGetSealCoord(int x_img, int y_img, CvtCoordNT* nt)
{
    CoordCvtCmd* cmd = new CoordCvtCmd;
    cmd->x_img_ = x_img;
    cmd->y_img_ = y_img;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleCvtCoord(char* chBuf)
{
    CoordCvtCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleCvtCoord->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CvtCoordNT* nt = (CvtCoordNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.x_dev_, cmd.y_dev_, cmd.ret_);
}

int AsynAPISet::AsynWriteRatio(float x, float y, WriteRatioNT* nt)
{
    WriteRatioCmd* cmd = new WriteRatioCmd;
    cmd->x_ = x;
    cmd->y_ = y;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleWriteRatio(char* chBuf)
{
    WriteRatioCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleWriteRatio->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    WriteRatioNT* nt = (WriteRatioNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

int AsynAPISet::AsynReadRatio(ReadRatioNT* nt)
{
    ReadRatioCmd* cmd = new ReadRatioCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleReadRatio(char* chBuf)
{
    ReadRatioCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleReadRatio->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    ReadRatioNT* nt = (ReadRatioNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.x_, cmd.y_, cmd.ret_);
}

int AsynAPISet::AsynWriteCali(unsigned short* pts, unsigned short len, WriteCaliNT* nt)
{
    WriteCaliPtsCmd* cmd = new WriteCaliPtsCmd;
    cmd->len_ = len;
    memcpy(cmd->pts_, pts, len * sizeof(unsigned short));
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleWriteCali(char* chBuf)
{
    WriteCaliPtsCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleWriteCali->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    WriteCaliNT* nt = (WriteCaliNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

int AsynAPISet::AsynReadCali(ReadCaliNT* nt)
{
    ReadCaliPtsCmd* cmd = new ReadCaliPtsCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleReadCali(char* chBuf)
{
    ReadCaliPtsCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleReadCali->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    ReadCaliNT* nt = (ReadCaliNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.pts_, cmd.len_, cmd.ret_);
}

int AsynAPISet::AsynQueryTop(QueryTopNT* nt)
{
    QueryTopCmd* cmd = new QueryTopCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleQueryTop(char* chBuf)
{
    QueryTopCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleQueryTop->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    QueryTopNT* nt = (QueryTopNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.status_, cmd.ret_);
}

int AsynAPISet::AsynExitMain(ExitMaintainNT* nt)
{
    ExitMaintainCmd* cmd = new ExitMaintainCmd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleExitMain(char* chBuf)
{
    ExitMaintainCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleExitMain->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    ExitMaintainNT* nt = (ExitMaintainNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

int AsynAPISet::AsynStartPreview(int which, int width, int height, int hwnd, StartPreviewNT* nt)
{
    StartPreviewCmd* cmd = new StartPreviewCmd;
    cmd->which_ = which;
    cmd->width_ = width;
    cmd->height_ = height;
    cmd->hwnd_ = hwnd;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleStartPreview(char* chBuf)
{
    StartPreviewCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleStartPreview->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    StartPreviewNT* nt = (StartPreviewNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

int AsynAPISet::AsynStopPreview(int which, StopPreviewNT* nt)
{
    StopPreviewCmd* cmd = new StopPreviewCmd;
    cmd->which_ = which;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleStopPreview(char* chBuf)
{
    StopPreviewCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleStopPreview->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    StopPreviewNT* nt = (StopPreviewNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}

int AsynAPISet::AsynCtrlFactory(int ctrl, CtrlFactoryNT* nt)
{
    CtrlFactoryCmd* cmd = new CtrlFactoryCmd;
    cmd->ctrl_ = ctrl;
    InsertNotify(cmd->send_time_, nt);

    return MC::Cnn::GetInst()->PushCmd(cmd);
}

void AsynAPISet::HandleFactoryCtrl(char* chBuf)
{
    CtrlFactoryCmd cmd;
    ParseCmd(&cmd, chBuf);
    Log::WriteLog(LL_DEBUG, "AsynAPISet::HandleFactoryCtrl->cmd: %s, ret: %d",
        cmd_des[cmd.ct_].c_str(),
        cmd.ret_);

    CtrlFactoryNT* nt = (CtrlFactoryNT*)LookupSendTime(cmd.send_time_);
    if (NULL != nt)
        nt->Notify(cmd.ret_);
}
