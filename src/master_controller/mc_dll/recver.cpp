#include <windows.h>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/exception/all.hpp>
#include "pipe_server.h"
#include "common_definitions.h"
#include "agent_cmd.h"
#include "seal_api.h"
#include "seria.h"
#include "recver.h"

#pragma warning(disable:4995)
#pragma warning(disable:4996)

MC::SynQueue<const RecvMsg*> Recver::recver_queue_;
HANDLE                       Recver::recv_msg_ev_;

HANDLE hPipe;

bool Recver::ParseConfig()
{
    std::string type = MC::SvrConfig::GetInst()->type_;
    if (type == "PIPE")
        cnn_type_ = MC::CT_PIPE;
    else if (type == "MQ")
        cnn_type_ = MC::CT_MQ;
    else
        cnn_type_ = MC::CT_MAX;

    return true;
}

bool Recver::StartMQ()
{
    return MQCnn::Start(MC::SvrConfig::GetInst()->name_, MC::SvrConfig::GetInst()->name_ + "copy");
}

bool Recver::StartPipe()
{
    recv_msg_ev_ = CreateEvent(
        NULL,       // 默认属性
        TRUE,       // 手工reset
        FALSE,      // 初始状态 signaled 
        NULL);      // 未命名

    HANDLE hConnectEvent;
    OVERLAPPED oConnect;
    LPPIPEINST lpPipeInst;
    DWORD dwWait, cbRet;
    BOOL fSuccess, fPendingIO;

    // 用于连接操作的事件对象 
    hConnectEvent = CreateEvent(
        NULL,    // 默认属性
        TRUE,    // 手工reset
        TRUE,    // 初始状态 signaled 
        NULL);   // 未命名

    if (hConnectEvent == NULL) {
        printf("CreateEvent failed with %d.\n", GetLastError());
        return 0;
    }
    // OVERLAPPED 事件
    oConnect.hEvent = hConnectEvent;

    // 创建连接实例，等待连接
    fPendingIO = CreateAndConnectInstance(&oConnect);

    while (1) {
        // 等待客户端连接或读写操作完成 
        dwWait = WaitForSingleObjectEx(
            hConnectEvent,  // 等待客户端连接的事件 
            INFINITE,       // 无限等待
            TRUE);

        switch (dwWait) {
        case 0:
            // pending
            if (fPendingIO) {
                // 获取 Overlapped I/O 的结果
                fSuccess = GetOverlappedResult(
                    hPipe,     // pipe 句柄
                    &oConnect, // OVERLAPPED 结构
                    &cbRet,    // 已经传送的数据量
                    FALSE);    // 不等待
                if (!fSuccess) {
                    printf("ConnectNamedPipe (%d)\n", GetLastError());
                    return 0;
                }
            }

            // 分配内存
            lpPipeInst = (LPPIPEINST)HeapAlloc(GetProcessHeap(), 0, sizeof(PIPEINST));
            if (lpPipeInst == NULL) {
                printf("GlobalAlloc failed (%d)\n", GetLastError());
                return 0;
            }
            lpPipeInst->hPipeInst = hPipe;

            // 读和写, 注意CompletedWriteRoutine和CompletedReadRoutine的相互调用
            lpPipeInst->cbToWrite = 0;
            CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst);

            // 再创建一个连接实例, 以响应下一个客户端的连接
            fPendingIO = CreateAndConnectInstance(&oConnect);
            break;
            // 读写完成 
        case WAIT_IO_COMPLETION:
            break;
        default: {
            printf("WaitForSingleObjectEx (%d)\n", GetLastError());
            return 0;
            }
        }
    }
}

bool Recver::Start()
{
    if (!ParseConfig()) {
        printf("Recver::Start->解析配置文件失败\n");
        Log::WriteLog(LL_ERROR, "Recver::Start->解析配置文件失败");
        return false;
    }

    bool conn;
    if (MC::CT_MQ == cnn_type_) {
        conn = StartMQ();
    } else if (MC::CT_PIPE == cnn_type_) {
        conn = StartPipe();
    } else {
        Log::WriteLog(LL_ERROR, "Recver::Start->未知的通信连接方式, %d", cnn_type_);
        conn = false;
    }

    if (!conn) {
        Log::WriteLog(LL_ERROR, "Recver::Start->fails to start conn channel, type: %d", 
            cnn_type_);
        return false;
    }

    printf("Recver::Start->start succ.\n");
    return true;
}

void Recver::Insert(const RecvMsg* msg)
{
    recver_queue_.Push(msg);
}

void Recver::Signal()
{
    SetEvent(recv_msg_ev_);
}

void Recver::Stop()
{
    MQCnn::Stop();
}

bool Recver::WriteResp(LPPIPEINST pipe_inst_, char* buf, unsigned int priority)
{
    return MQCnn::SendMsg(buf, CMD_BUF_SIZE, priority);

    // 管道连接
    if (cnn_type_ == MC::CT_PIPE) {
        boost::lock_guard<boost::mutex> lk(write_resp_mtx_);

        lstrcpyn(pipe_inst_->chReply, TEXT(buf), CMD_BUF_SIZE);
        pipe_inst_->cbToWrite = (lstrlen(pipe_inst_->chReply) + 1) * sizeof(TCHAR);
        // 将回复写入到pipe
        DWORD cbWritten;
        BOOL fWrite = WriteFile(
            pipe_inst_->hPipeInst,
            pipe_inst_->chReply,    //将响应写入pipe
            pipe_inst_->cbToWrite,
            &cbWritten,
            NULL);
        return fWrite != 0;
    }

    return true;
}

void Recver::OnRecvMQMsg(char* buf, int size)
{
    char cmd;       // 消息头
    memcpy(&cmd, buf, sizeof(char));
    RecvMsg msg;
    msg.pipe_inst = NULL;
    strncpy(msg.msg, buf, size);
    // filter heart beat cmd
    if (cmd >= (char)CT_INIT_MACHINE && cmd <= (char)CT_STOP_RECORD && cmd != (char)CT_HEART_BEAT)
        Log::WriteLog(LL_DEBUG, "Recver::OnRecvMQMsg->收到: %s", cmd_des[cmd].c_str());

    switch (cmd) {
    case CT_QUERY_MACHINE:
        HandleQueryMachine(&msg);
        break;
    case CT_SET_MACHINE:
        HandleSetMachine(&msg);
        break;
    case CT_INIT_MACHINE:
        HandleInitMachine(&msg);
        break;
    case CT_BIND_MAC:
        HandleBindMAC(&msg);
        break;
    case CT_UNBIND_MAC:
        HandleUnbindMAC(&msg);
        break;
    case CT_PREPARE_STAMP:
        HandlePrepareStamp(&msg);
        break;
    case CT_PAPER_DOOR:
        HandleQueryPaper(&msg);
        break;
    case CT_SNAPSHOT:
        HandleSnapshot(&msg);
        break;
    case CT_PHOTO_SYNTHESIS:
        HandleMergePhoto(&msg);
        break;
    case CT_RECOGNITION:
        HandleRecognition(&msg);
        break;
    case CT_SEARCH_STAMP:
        HandleSearchStampPoint(&msg);
        break;
    case CT_ELEMENT_IDENTI:
        HandleElementIdenti(&msg);
        break;
    case CT_ORDINARY_STAMP:
        HandleOrdinary(&msg);
        break;
    case CT_AUTO_STAMP:
        HandleAuto(&msg);
        break;
    case CT_FINISH_STAMP:
        HandleFinish(&msg);
        break;
    case CT_RELEASE_STAMPER:
        HandleReleaseStamper(&msg);
        break;
    case CT_GET_ERROR:
        HandleGetError(&msg);
        break;
    case CT_CALIBRATION:
        HandleCalibrate(&msg);
        break;
    case CT_HEART_BEAT:
        HandleHeart(&msg);
        break;
    case CT_QUERY_STAMPERS:
        HandleQueryStampers(&msg);
        break;
    case CT_QUERY_SAFE:
        HandleQuerySafe(&msg);
        break;
    case CT_SAFE_CTL:
        HandleSafeControl(&msg);
        break;
    case CT_BEEP_CTL:
        HandleBeepControl(&msg);
        break;
    case CT_QUERY_SLOT:
        HandleQuerySlot(&msg);
        break;
    case CT_ALARM_CTL:
        HandleAlarmControl(&msg);
        break;
    case CT_QUERY_ALARM:
        HandleQueryAlarm(&msg);
        break;
    case CT_QUERY_MAC:
        HandleQueryMAC(&msg);
        break;
    case CT_LOCK:
        HandleLock(&msg);
        break;
    case CT_UNLOCK:
        HandleUnlock(&msg);
        break;
    case CT_LOCK_STATUS:
        HandleQueryLock(&msg);
        break;
    case CT_OPEN_CON:
        HandleOpenCnn(&msg);
        break;
    case CT_CLOSE_CON:
        HandleCloseCnn(&msg);
        break;
    case CT_QUERY_CON:
        HandleQueryCnn(&msg);
        break;
    case CT_SIDE_DOOR_ALARM:
        HandleSetSideDoor(&msg);
        break;
    case CT_QUERY_DEV_MODEL:
        HandleGetDevModel(&msg);
        break;
    case CT_OPEN_PAPER:
        HandleOpenPaper(&msg);
        break;
    case CT_LED_CTL:
        HandleCtrlLED(&msg);
        break;
    case CT_CHECK_PARAM:
        HandleCheckParam(&msg);
        break;
    case CT_OPEN_CAMERA:
        HandleOpenCamera(&msg);
        break;
    case CT_CLOSE_CAMERA:
        HandleCloseCamera(&msg);
        break;
    case CT_CAMERA_STATUS:
        HandleGetCameraStatus(&msg);
        break;
    case CT_SET_RESOLUTION:
        HandleSetResolution(&msg);
        break;
    case CT_SET_DPI:
        HandleSetDPI(&msg);
        break;
    case CT_SET_PROPERTY:
        HandleSetProperty(&msg);
        break;
    case CT_RECORD:
        HandleRecordVideo(&msg);
        break;
    case CT_STOP_RECORD:
        HandleStopRecordVideo(&msg);
        break;
    case CT_GET_RFID:
        HandleGetRFID(&msg);
        break;
    case CT_GET_DEV_STATUS:
        HandleGetDevStatus(&msg);
        break;
    case CT_GET_SEAL_COORD:
        HandleCvtCoord(&msg);
        break;
    case CT_WRITE_CVT_RATIO:
        HandleWriteRatio(&msg);
        break;
    case CT_READ_CVT_RATIO:
        HandleReadRatio(&msg);
        break;
    case CT_WRITE_CALIBRATION:
        HandleWriteCali(&msg);
        break;
    case CT_READ_CALIBRATION:
        HandleReadCali(&msg);
        break;
    case CT_QUERY_TOP:
        HandleQueryTop(&msg);
        break;
    case CT_ENTER_MAIN:
        HandleEnterMain(&msg);
        break;
    case CT_EXIT_MAIN:
        HandleExitMain(&msg);
        break;
    case CT_START_PREVIEW:
        HandleStartPreview(&msg);
        break;
    case CT_STOP_PREVIEW:
        HandleStopPreview(&msg);
        break;
    case CT_FACTORY_CTRL:
        HandleFactoryCtrl(&msg);
        break;
    case CT_RECOG_MODEL:
        HandleRegcoEtc(&msg);
        break;
    case CT_RESET:
        HandleReset(&msg);
        break;
    case CT_RESTART:
        HandleRestart(&msg);
        break;
    case CT_GET_SYSTEM:
        HandleGetSystem(&msg);
        break;
    case CT_READ_MAIN_SPARE:
        HandleReadMainSpare(&msg);
        break;
    case CT_WRITE_MAIN_SPARE:
        HandleWriteMainSpare(&msg);
        break;
    case CT_RECOG_QR:
        HandleRecogQR(&msg);
        break;
    case CT_CALCULATE_RATIO:
        HandleCalcRatio(&msg);
        break;
    case CT_FIND_2CIRCLES:
        HandleFind2Circles(&msg);
        break;
    case CT_FIND_4CIRCLES:
        HandleFind4Circles(&msg);
        break;
    default:
        printf("Recver::ReceiverFunc->Unknown cmd: %d\n", cmd);
        break;
    }
}

//////////////////////////// 获取印控仪编号 ///////////////////////////////

class QueryMachNT : public MC::NotifyResult {
public:
    QueryMachNT(LPPIPEINST inst, QueryMachineCmd* cmd, Recver* recv) :
        pipe_inst_(inst), 
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryMachNT::Notify->ec: " << ec <<
            ", 印控仪编号: " << data1.c_str() << std::endl;

        strcpy_s(cmd_->sn_, data1.c_str());
        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    QueryMachineCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryMachine(const RecvMsg* msg)
{
    QueryMachineCmd* cmd = new (std::nothrow) QueryMachineCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    Log::WriteLog(LL_DEBUG, "Recver::HandleQueryMachine->获取印控仪编号");

    // 获取印控仪编号
    MC::NotifyResult* notify = new (std::nothrow) QueryMachNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryMachine(notify);
}

////////////////////////// 设置印控机编号 ////////////////////////////////////

class SetMachNT : public MC::NotifyResult {
public:
    SetMachNT(LPPIPEINST inst, SetMachineCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "SetMachNT::Notify->ec: " << ec <<
            ", 待设置印控仪编号: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    SetMachineCmd*      cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSetMachine(const RecvMsg* msg)
{
    SetMachineCmd* cmd = new (std::nothrow) SetMachineCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    Log::WriteLog(LL_DEBUG, "Recver::HandleSetMachine->设置印控仪编号");

    MC::NotifyResult* notify = new (std::nothrow) SetMachNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SetMachine(cmd->sn_, notify);
}

/////////////////////////// 初始化印控机 ////////////////////////////////

class InitMachNT: public MC::NotifyResult {
public:
    InitMachNT(LPPIPEINST inst, InitMachineCmd* cmd, Recver* recv) : 
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "InitMachNT::Notify->ec: " << ec << 
            ", 认证码: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    InitMachineCmd*  cmd_;
    LPPIPEINST pipe_inst_;
    Recver* recver_;
};

void Recver::HandleInitMachine(const RecvMsg* msg)
{
    InitMachineCmd* init_cmd = new (std::nothrow) InitMachineCmd;
    memcpy(init_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    init_cmd->Unser();
    Log::WriteLog(LL_DEBUG, "Recver::HandleInitMachine->初始化印控机, key: %s", init_cmd->key_);

    // 初始化印控机
    MC::NotifyResult* notify = new (std::nothrow) InitMachNT(msg->pipe_inst, init_cmd, this);
    MC::STSealAPI::GetInst()->InitMachine(init_cmd->key_, notify);
}

//////////////////////////// 绑定MAC地址 ////////////////////////////////

class BindMACNT : public MC::NotifyResult {
public:
    BindMACNT(LPPIPEINST inst, BindMACCmd* cmd, Recver* recv) : 
        pipe_inst_(inst), 
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "BindMACNT::Notify->ec: " << ec <<
            ", MAC地址: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    BindMACCmd*     cmd_;
    LPPIPEINST      pipe_inst_;
    Recver*         recver_;
};

void Recver::HandleBindMAC(const RecvMsg* msg)
{
    BindMACCmd* bind_cmd = new (std::nothrow) BindMACCmd;
    memcpy(bind_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    bind_cmd->Unser();
    Log::WriteLog(LL_DEBUG, "Recver::HandleBindMAC->绑定MAC地址, mac: %s", bind_cmd->mac_);

    // 绑定MAC地址
    MC::NotifyResult* notify = new (std::nothrow) BindMACNT(msg->pipe_inst, bind_cmd, this);
    MC::STSealAPI::GetInst()->BindMAC(bind_cmd->mac_, notify);
}

//////////////////////// 解绑MAC地址 ////////////////////////////

class UnbindMACNT : public MC::NotifyResult {
public:
    UnbindMACNT(LPPIPEINST inst, UnbindCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "UnbindMACNT::Notify->ec: " << ec <<
            ", 解绑MAC地址: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    UnbindCmd*      cmd_;
    LPPIPEINST      pipe_inst_;
    Recver*         recver_;
};

void Recver::HandleUnbindMAC(const RecvMsg* msg)
{
    UnbindCmd* unbind_cmd = new (std::nothrow) UnbindCmd;
    memcpy(unbind_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    unbind_cmd->Unser();
    printf("Recver::HandleUnbindMAC->解绑MAC地址, mac: %s\n", unbind_cmd->mac_);

    // 解绑MAC地址
    MC::NotifyResult* notify = new (std::nothrow) UnbindMACNT(msg->pipe_inst, unbind_cmd, this);
    MC::STSealAPI::GetInst()->UnbindMAC(unbind_cmd->mac_, notify);
}

////////////////////// 准备用印 ///////////////////////////////

class PrepareStampNT : public MC::NotifyResult {
public:
    PrepareStampNT(LPPIPEINST inst, PrepareStampCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "PrepareStampNT::Notify->ec: " << ec <<
            ", 任务ID: " << ctx1.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->task_id_, ctx1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    PrepareStampCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandlePrepareStamp(const RecvMsg* msg)
{
    PrepareStampCmd* prepare_cmd = new (std::nothrow) PrepareStampCmd;
    memcpy(prepare_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    prepare_cmd->Unser();
    printf("Recver::HandlePrepareStamp->准备用印, 印章卡槽号: %d, 超时时间(秒): %d\n", 
        prepare_cmd->stamper_id_,
        prepare_cmd->timeout_);

    MC::NotifyResult* notify = new (std::nothrow) PrepareStampNT(msg->pipe_inst, prepare_cmd, this);
    MC::STSealAPI::GetInst()->PrepareStamp(prepare_cmd->stamper_id_, prepare_cmd->timeout_, notify);
}

///////////////////////////// 查询进纸门状态 ///////////////////////////////////

class QueryPaperNT : public MC::NotifyResult {
public:
    QueryPaperNT(LPPIPEINST inst, ViewPaperCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryPaperNT::Notify->ec: " << ec <<
            ", 进纸门状态: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;
        cmd_->status_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    ViewPaperCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryPaper(const RecvMsg* msg)
{
    ViewPaperCmd* cmd = new (std::nothrow) ViewPaperCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryPaper->查进纸门状态\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryPaperNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryPaperDoor(notify);
}

/////////////////////////// 拍照 ///////////////////////////////////

class SnapshotNT : public MC::NotifyResult {
public:
    SnapshotNT(LPPIPEINST inst, SnapshotCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "SnapshotNT::Notify->ec: " << ec <<
            ", 原始图像路径: " << ctx1.c_str() << 
            ", 剪切后图像路径: " << ctx2.c_str() << std::endl;

        //回复结果
        cmd_->ret_ = ec;
        strcpy_s(cmd_->original_path_, ctx1.c_str());
        strcpy_s(cmd_->cut_path_, ctx2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    SnapshotCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSnapshot(const RecvMsg* msg)
{
    SnapshotCmd* snap_cmd = new (std::nothrow) SnapshotCmd;
    memcpy(snap_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    snap_cmd->Unser();
    printf("Recver::HandleSnapshot->拍照, 原始图像DPI: %d, 剪切后图像DPI: %d\n",
        snap_cmd->original_dpi_,
        snap_cmd->cut_dpi_);

    MC::NotifyResult* notify = new (std::nothrow) SnapshotNT(msg->pipe_inst, snap_cmd, this);
    MC::STSealAPI::GetInst()->Snapshot(
        snap_cmd->which_,
        snap_cmd->original_dpi_,
        snap_cmd->cut_dpi_, 
        snap_cmd->original_path_,
        snap_cmd->cut_path_,
        notify);
}

//////////////////////// 照片合成 ///////////////////////////////////////

class MergePhotoNT : public MC::NotifyResult {
public:
    MergePhotoNT(LPPIPEINST inst, SynthesizePhotoCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "MergePhotoNT::Notify->ec: " << ec <<
            ", 合成后图像路径: " << ctx1.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->merged_, ctx1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    SynthesizePhotoCmd* cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleMergePhoto(const RecvMsg* msg)
{
    SynthesizePhotoCmd* merge_cmd = new (std::nothrow) SynthesizePhotoCmd;
    memcpy(merge_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    merge_cmd->Unser();
    printf("Recver::HandleMergePhoto->合成照片, 图像1路径: %s, 图像2路径: %s\n",
        merge_cmd->photo1_,
        merge_cmd->photo2_);

    MC::NotifyResult* notify = new (std::nothrow) MergePhotoNT(msg->pipe_inst, merge_cmd, this);
    MC::STSealAPI::GetInst()->MergePhoto(merge_cmd->photo1_, merge_cmd->photo2_, merge_cmd->merged_, notify);
}

/////////////////////// 版面验证码识别 //////////////////////////////////////

class RecognitionNT : public MC::NotifyResult {
public:
    RecognitionNT(LPPIPEINST inst, RecognitionCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "RecognitionNT::Notify->ec: " << ec <<
            ", 模板ID: " << ctx1.c_str() << ", 追溯码: " << ctx2.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->model_type_, ctx1.c_str());
        strcpy_s(cmd_->trace_num_, ctx2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    RecognitionCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleRecognition(const RecvMsg* msg)
{
    RecognitionCmd* recog_cmd = new (std::nothrow) RecognitionCmd;
    memcpy(recog_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    recog_cmd->Unser();
    printf("Recver::HandleRecognition->待识别图片路径: %s\n",
        recog_cmd->path_);

    MC::NotifyResult* notify = new (std::nothrow) RecognitionNT(msg->pipe_inst, recog_cmd, this);
    MC::STSealAPI::GetInst()->RecognizeImage(recog_cmd->path_, notify);
}

/////////////////////// 原图用印点查找 //////////////////////////////////////

class SearchStampNT : public MC::NotifyResult {
public:
    SearchStampNT(LPPIPEINST inst, SearchStampPointCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "SearchStampNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->out_x_ = atoi(data1.c_str());
        cmd_->out_y_ = atoi(data2.c_str());
        cmd_->out_angle_ = atof(ctx1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    SearchStampPointCmd* cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSearchStampPoint(const RecvMsg* msg)
{
    SearchStampPointCmd* cmd = new (std::nothrow) SearchStampPointCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSearchStampPoint->查找原图用印点: %s\n",
        cmd->src_);

    MC::NotifyResult* notify = new (std::nothrow) SearchStampNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SearchSrcImageStampPoint(
        cmd->src_,
        cmd->in_x_,
        cmd->in_y_,
        cmd->in_angle_, 
        notify);
}

///////////////////////// 要素识别 //////////////////////////////////////

class IdentifyElementNT : public MC::NotifyResult {
public:
    IdentifyElementNT(LPPIPEINST inst, IdentifyElementCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "IdentifyElementNT::Notify->ec: " << ec <<
            ", 识别图片路径: " << data1.c_str() << 
            ", 识别结果: " << data2.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->content_str_, data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    IdentifyElementCmd* cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleElementIdenti(const RecvMsg* msg)
{
    IdentifyElementCmd* identi_cmd = new (std::nothrow) IdentifyElementCmd;
    memcpy(identi_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    identi_cmd->Unser();
    printf("Recver::HandleElementIdenti->待识别图片路径: %s\n",
        identi_cmd->path_);

    MC::NotifyResult* notify = new (std::nothrow) IdentifyElementNT(msg->pipe_inst, identi_cmd, this);
    MC::STSealAPI::GetInst()->IdentifyElement(
        identi_cmd->path_,
        identi_cmd->x_,
        identi_cmd->y_,
        identi_cmd->width_,
        identi_cmd->height_,
        notify);
}

///////////////////////// 模板及用印点 //////////////////////////////////////

class RegcoEtcNT : public MC::NotifyResult {
public:
    RegcoEtcNT(LPPIPEINST inst, RecoModelTypeEtcCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "RegcoEtcNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        memcpy(cmd_->model_result_, data1.c_str(), data1.length());
        cmd_->angle_ = atof(data2.c_str());
        cmd_->x_ = atoi(ctx1.c_str());
        cmd_->y_ = atoi(ctx2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    RecoModelTypeEtcCmd*cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleRegcoEtc(const RecvMsg* msg)
{
    RecoModelTypeEtcCmd* cmd = new (std::nothrow) RecoModelTypeEtcCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleRegcoEtc->图片路径: %s\n",
        cmd->path_);

    MC::NotifyResult* notify = new (std::nothrow) RegcoEtcNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->RecoModelEtc(
        cmd->path_,
        notify);
}

////////////////////////// 普通用印 /////////////////////////////////////

class OridinaryStampNT: public MC::NotifyResult {
public:
    OridinaryStampNT(LPPIPEINST inst, OridinaryStampCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "OridinaryStampNT::Notify->ec: " << ec <<
            ", 任务号: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    OridinaryStampCmd*  cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleOrdinary(const RecvMsg* msg)
{
    OridinaryStampCmd* ordi_cmd = new (std::nothrow) OridinaryStampCmd;
    memcpy(ordi_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    ordi_cmd->Unser();
    printf("Recver::HandleOrdinary->普通用印, 任务号: %s, 凭证类型: %s, 印章卡槽号: %d\n",
        ordi_cmd->task_id_,
        ordi_cmd->type_,
        ordi_cmd->stamper_num_);

    MC::NotifyResult* notify = new (std::nothrow) OridinaryStampNT(msg->pipe_inst, ordi_cmd, this);
    MC::STSealAPI::GetInst()->OrdinaryStamp(
        ordi_cmd->task_id_,
        ordi_cmd->type_,
        ordi_cmd->stamper_num_,
        ordi_cmd->ink_,
        ordi_cmd->x_,
        ordi_cmd->y_,
        ordi_cmd->angle_,
        ordi_cmd->seal_type_,
        notify);
}

/////////////////////////// 自动用印 ////////////////////////////////////////

class AutoStampNT : public MC::NotifyResult {
public:
    AutoStampNT(LPPIPEINST inst, AutoStampCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "AutoStampNT::Notify->ec: " << ec <<
            ", 任务号: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    AutoStampCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleAuto(const RecvMsg* msg)
{
    AutoStampCmd* auto_cmd = new (std::nothrow) AutoStampCmd;
    memcpy(auto_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    auto_cmd->Unser();
    printf("Recver::HandleAuto->自动用印, 任务号: %s, 凭证类型: %s, 印章卡槽号: %d\n",
        auto_cmd->task_id_,
        auto_cmd->type_,
        auto_cmd->stamper_num_);

    MC::NotifyResult* notify = new (std::nothrow) AutoStampNT(msg->pipe_inst, auto_cmd, this);
    MC::STSealAPI::GetInst()->AutoStamp(
        auto_cmd->task_id_,
        auto_cmd->type_,
        auto_cmd->stamper_num_,
        notify);
}

///////////////////////// 结束用印 //////////////////////////////////////

class FinishStampNT : public MC::NotifyResult {
public:
    FinishStampNT(LPPIPEINST inst, FinishStampCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "FinishStampNT::Notify->ec: " << ec <<
            ", 任务号: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    FinishStampCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleFinish(const RecvMsg* msg)
{
    FinishStampCmd* finish_cmd = new (std::nothrow) FinishStampCmd;
    memcpy(finish_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    finish_cmd->Unser();
    printf("Recver::HandleAuto->结束用印, 任务号: %s\n", finish_cmd->task_id_);

    MC::NotifyResult* notify = new (std::nothrow) FinishStampNT(msg->pipe_inst, finish_cmd, this);
    MC::STSealAPI::GetInst()->FinishStamp(finish_cmd->task_id_, notify);
}

////////////////////////// 释放印控机 /////////////////////////////////////

class ReleaNT : public MC::NotifyResult {
public:
    ReleaNT(LPPIPEINST inst, ReleaseStamperCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "ReleaNT::Notify->ec: " << ec <<
            ", 印控机编号: " << data1.c_str() << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    ReleaseStamperCmd*  cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleReleaseStamper(const RecvMsg* msg)
{
    ReleaseStamperCmd* release_cmd = new (std::nothrow) ReleaseStamperCmd;
    memcpy(release_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    release_cmd->Unser();
    printf("Recver::HandleReleaseStamper->印控机编号: %s\n", release_cmd->stamp_id_);

    MC::NotifyResult* notify = new (std::nothrow) ReleaNT(msg->pipe_inst, release_cmd, this);
    MC::STSealAPI::GetInst()->ReleaseStamp(release_cmd->stamp_id_, notify);
}

////////////////////////// 获取错误信息 ///////////////////////////////////

class GetErNT : public MC::NotifyResult {
public:
    GetErNT(LPPIPEINST inst, GetErrorCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "GetErNT::Notify->ec: " << ec <<
            ", 错误码: " << atoi(ctx1.c_str()) << 
            ", 错误信息: " << data1.c_str() << 
            ", 解决方案: " << data2.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->err_msg_, data1.c_str());
        strcpy_s(cmd_->err_resolver_, data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    GetErrorCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleGetError(const RecvMsg* msg)
{
    GetErrorCmd* err_cmd = new (std::nothrow) GetErrorCmd;
    memcpy(err_cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    err_cmd->Unser();
    printf("Recver::HandleGetError->错误码: %d\n", err_cmd->err_);

    MC::NotifyResult* notify = new (std::nothrow) GetErNT(msg->pipe_inst, err_cmd, this);
    MC::STSealAPI::GetInst()->GetError(err_cmd->err_, notify);
}

///////////////////////// 校准印章 //////////////////////////////////

class CalibrateNT : public MC::NotifyResult {
public:
    CalibrateNT(LPPIPEINST inst, CalibrateCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "CalibrateNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->slot_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    CalibrateCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCalibrate(const RecvMsg* msg)
{
    CalibrateCmd* cmd = new (std::nothrow) CalibrateCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCalibrate->校准印章, 印章号: %d\n", cmd->slot_);

    MC::NotifyResult* notify = new (std::nothrow) CalibrateNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CalibrateMachine(cmd->slot_, notify);
}

/////////////////////////// 印章状态查询 //////////////////////////////////

class QueryStampersNT : public MC::NotifyResult {
public:
    QueryStampersNT(LPPIPEINST inst, QueryStampersCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryStampersNT::Notify->ec: " << ec <<
            ", 印章状态: " << data1.c_str();

        cmd_->ret_ = ec;
        if (ec == MC::EC_SUCC) {
            for (int i = 0; i < MAX_STAMPER_NUM; ++i) {
                cmd_->stamper_status_[i] = data1.at(i);
            }
            cmd_->stamper_status_[MAX_STAMPER_NUM] = 0x0;
        }

        recver_->PushCmd(cmd_);
    }

private:
    QueryStampersCmd*   cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryStampers(const RecvMsg* msg)
{
    QueryStampersCmd* cmd = new (std::nothrow) QueryStampersCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryStampers->印章状态查询\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryStampersNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryStampers(notify);
}

/////////////////////////// 查询安全门状态/ /////////////////////////////////

class QuerySafeNT : public MC::NotifyResult {
public:
    QuerySafeNT(LPPIPEINST inst, QuerySafeCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "QuerySafeNT::Notify->ec: " << ec <<
            ", 安全门状态: " << data1.c_str();

        cmd_->ret_ = ec;
        cmd_->status_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QuerySafeCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQuerySafe(const RecvMsg* msg)
{
    QuerySafeCmd* cmd = new (std::nothrow) QuerySafeCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQuerySafe->查询安全门状态\n");

    MC::NotifyResult* notify = new (std::nothrow) QuerySafeNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QuerySafeDoor(notify);
}

///////////////////////////// 开关安全门 ////////////////////////////////

class SafeCtlNT : public MC::NotifyResult {
public:
    SafeCtlNT(LPPIPEINST inst, SafeCtrlCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "SafeCtlNT::Notify->ec: " << ec <<
            ", 操作: " << data1.c_str();

        cmd_->ret_ = ec;
        cmd_->ctrl_ = atoi(data1.c_str());
        cmd_->timeout_ = atoi(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    SafeCtrlCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSafeControl(const RecvMsg* msg)
{
    SafeCtrlCmd* cmd = new (std::nothrow) SafeCtrlCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSafeControl->开关安全门, 操作: %d, 超时时间: %d\n", 
        cmd->ctrl_,
        cmd->timeout_);

    MC::NotifyResult* notify = new (std::nothrow) SafeCtlNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->OperateSafeDoor(cmd->ctrl_, cmd->timeout_, notify);
}

//////////////////////////// 蜂鸣器控制 /////////////////////////////////

class BeepCtlNT : public MC::NotifyResult {
public:
    BeepCtlNT(LPPIPEINST inst, BeepCtrlCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "BeepCtlNT::Notify->ec: " << ec <<
            ", 操作: " << data1.c_str();

        cmd_->ret_ = ec;
        cmd_->ctrl_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    BeepCtrlCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleBeepControl(const RecvMsg* msg)
{
    BeepCtrlCmd* cmd = new (std::nothrow) BeepCtrlCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleBeepControl->蜂鸣器控制, 操作: %d\n", cmd->ctrl_);

    MC::NotifyResult* notify = new (std::nothrow) BeepCtlNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->OperateBeep(
        cmd->ctrl_, 
        cmd->type_,
        cmd->interval_,
        notify);
}

///////////////////////// 卡槽数量查询 ////////////////////////////////////

class QueryStNT : public MC::NotifyResult {
public:
    QueryStNT(LPPIPEINST inst, QuerySlotCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "QueryStNT::Notify->ec: " << ec <<
            ", 卡槽数量: " << data1.c_str();

        cmd_->ret_ = ec;
        cmd_->num_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QuerySlotCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQuerySlot(const RecvMsg* msg)
{
    QuerySlotCmd* cmd = new (std::nothrow) QuerySlotCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQuerySlot->卡槽数量查询\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryStNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QuerySlot(notify);
}

//////////////////////// 报警器控制 ////////////////////////////////

class AlarmCtrlNT : public MC::NotifyResult {
public:
    AlarmCtrlNT(LPPIPEINST inst, AlarmCtrlCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "AlarmCtrlNT::Notify->ec: " << ec <<
            ", 报警器类型: " << data1.c_str() <<
            ", 开关: " << data2.c_str() << std::endl;

        cmd_->ret_ = ec;
        cmd_->alarm_ = atoi(data1.c_str());
        cmd_->ctrl_ = atoi(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    AlarmCtrlCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleAlarmControl(const RecvMsg* msg)
{
    AlarmCtrlCmd* cmd = new (std::nothrow) AlarmCtrlCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleAlarmControl->报警器控制\n");

    MC::NotifyResult* notify = new (std::nothrow) AlarmCtrlNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->OperateAlarm(cmd->alarm_, cmd->ctrl_, notify);
}

//////////////////////// 报警器状态查询 ////////////////////////////////

class QueryAlarmNT : public MC::NotifyResult {
public:
    QueryAlarmNT(LPPIPEINST inst, QueryAlarmCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryAlarmNT::Notify->ec: " << std::endl;

        cmd_->ret_ = ec;
        cmd_->door_ = atoi(data1.c_str());
        cmd_->vibration_ = atoi(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QueryAlarmCmd*      cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryAlarm(const RecvMsg* msg)
{
    QueryAlarmCmd* cmd = new (std::nothrow) QueryAlarmCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryAlarm->报警器状态查询\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryAlarmNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryAlarm(notify);
}

/////////////////////////// 查询已绑定MAC //////////////////////////////////

class QueryMACNT : public MC::NotifyResult {
public:
    QueryMACNT(LPPIPEINST inst, QueryMACCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "QueryMACNT::Notify->ec: " << ec <<
            ", mac1: " << data1.c_str() <<
            ", mac2: " << data2.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->mac1_, data1.c_str());
        strcpy_s(cmd_->mac2_, data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QueryMACCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryMAC(const RecvMsg* msg)
{
    QueryMACCmd* cmd = new (std::nothrow) QueryMACCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryMAC->查询已绑定MAC地址\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryMACNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryMAC(notify);
}

////////////////////////////////////

class LockNT: public MC::NotifyResult {
public:
    LockNT(LPPIPEINST inst, LockCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "LockNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    LockCmd*            cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleLock(const RecvMsg* msg)
{
    LockCmd* cmd = new (std::nothrow) LockCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleLock->锁定印控仪\n");

    MC::NotifyResult* notify = new (std::nothrow) LockNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Lock(notify);
}

////////////////////////////

class UnlockNT: public MC::NotifyResult {
public:
    UnlockNT(LPPIPEINST inst, UnlockCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "UnlockNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    UnlockCmd*          cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleUnlock(const RecvMsg* msg)
{
    UnlockCmd* cmd = new (std::nothrow) UnlockCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleUnlock->解锁印控仪\n");

    MC::NotifyResult* notify = new (std::nothrow) UnlockNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Unlock(notify);
}

////////////////////////////

class QueryLockNT: public MC::NotifyResult {
public:
    QueryLockNT(LPPIPEINST inst, QueryLockCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryLockNT::Notify->ec: " << ec << std::endl;

        if (ec == 31 || ec == 32)
            cmd_->ret_ = MC::EC_SUCC;
        else
            cmd_->ret_ = ec;

        if (31 == ec)
            cmd_->status_ = 1;
        else if (32 == ec)
            cmd_->status_ = 0;
        else
            cmd_->status_ = -1;

        recver_->PushCmd(cmd_);
    }

private:
    QueryLockCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryLock(const RecvMsg* msg)
{
    QueryLockCmd* cmd = new (std::nothrow) QueryLockCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryLock->印控仪锁定状态\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryLockNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryLock(notify);
}

////////////////////////////

class OpenCnnNT: public MC::NotifyResult {
public:
    OpenCnnNT(LPPIPEINST inst, OpenCnnCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "QueryLockNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    OpenCnnCmd*         cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleOpenCnn(const RecvMsg* msg)
{
    OpenCnnCmd* cmd = new (std::nothrow) OpenCnnCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleOpenCnn->打开设备连接\n");

    MC::NotifyResult* notify = new (std::nothrow) OpenCnnNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Connect(notify);
}

////////////////////////////

class CloseCnnNT: public MC::NotifyResult {
public:
    CloseCnnNT(LPPIPEINST inst, CloseCnnCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "QueryLockNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    CloseCnnCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCloseCnn(const RecvMsg* msg)
{
    CloseCnnCmd* cmd = new (std::nothrow) CloseCnnCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCloseCnn->关闭设备连接\n");

    MC::NotifyResult* notify = new (std::nothrow) CloseCnnNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Disconnect(notify);
}

////////////////////////////

class QueryCnnNT: public MC::NotifyResult {
public:
    QueryCnnNT(LPPIPEINST inst, QueryCnnCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryCnnNT::Notify->ec: " << ec << std::endl;

        if (5 == ec) {
            cmd_->ret_ = MC::EC_SUCC;
            cmd_->status_ = 1;
        } else if (7 == ec){
            cmd_->ret_ = MC::EC_SUCC;
            cmd_->status_ = 0;
        } else {
            cmd_->ret_ = ec;
        }

        recver_->PushCmd(cmd_);
    }

private:
    QueryCnnCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryCnn(const RecvMsg* msg)
{
    QueryCnnCmd* cmd = new (std::nothrow) QueryCnnCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleQueryCnn->连接状态查询\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryCnnNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryCnn(notify);
}

////////////////////////////

class SetSideDoorNT: public MC::NotifyResult {
public:
    SetSideDoorNT(LPPIPEINST inst, SideDoorAlarmCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "SetSideDoorNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    SideDoorAlarmCmd*   cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSetSideDoor(const RecvMsg* msg)
{
    SideDoorAlarmCmd* cmd = new (std::nothrow) SideDoorAlarmCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSetSideDoor->设置安全门报警器\n");

    MC::NotifyResult* notify = new (std::nothrow) SetSideDoorNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SetSideDoor(cmd->keep_open_,
                                        cmd->timeout_,
                                        notify);
}

////////////////////////////

class GetModelNT: public MC::NotifyResult {
public:
    GetModelNT(LPPIPEINST inst, GetDevModelCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "GetModelNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        strcpy(cmd_->model_, data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    GetDevModelCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleGetDevModel(const RecvMsg* msg)
{
    GetDevModelCmd* cmd = new (std::nothrow) GetDevModelCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleGetDevModel->获取设备型号\n");

    MC::NotifyResult* notify = new (std::nothrow) GetModelNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryDevModel(notify);
}

////////////////////////////

class OpenPaperNT: public MC::NotifyResult {
public:
    OpenPaperNT(LPPIPEINST inst, OpenPaperCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "OpenPaperNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    OpenPaperCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleOpenPaper(const RecvMsg* msg)
{
    OpenPaperCmd* cmd = new (std::nothrow) OpenPaperCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleOpenPaper->打开进纸门\n");

    MC::NotifyResult* notify = new (std::nothrow) OpenPaperNT(
        msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->OpenPaper(cmd->timeout_, notify);
}

////////////////////////////

class CtrlLedNT: public MC::NotifyResult {
public:
    CtrlLedNT(LPPIPEINST inst, CtrlLEDCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "CtrlLedNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    CtrlLEDCmd*         cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCtrlLED(const RecvMsg* msg)
{
    CtrlLEDCmd* cmd = new (std::nothrow) CtrlLEDCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCtrlLED->控制补光灯\n");

    MC::NotifyResult* notify = new (std::nothrow) CtrlLedNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CtrlLed(
        cmd->which_,
        cmd->switch_,
        cmd->value_,
        notify);
}

////////////////////////////

class CheckParaNT: public MC::NotifyResult {
public:
    CheckParaNT(LPPIPEINST inst, CheckParamCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "CheckParaNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    CheckParamCmd*      cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCheckParam(const RecvMsg* msg)
{
    CheckParamCmd* cmd = new (std::nothrow) CheckParamCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCheckParam->用印参数检查\n");

    MC::NotifyResult* notify = new (std::nothrow) CheckParaNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CheckParams(
        cmd->x_,
        cmd->y_,
        cmd->angle_,
        notify);
}

////////////////////////////

class OpenCameraNT: public MC::NotifyResult {
public:
    OpenCameraNT(LPPIPEINST inst, OpenCameraCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "OpenCameraNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    OpenCameraCmd*      cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleOpenCamera(const RecvMsg* msg)
{
    OpenCameraCmd* cmd = new (std::nothrow) OpenCameraCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleOpenCamera->打开摄像头\n");

    MC::NotifyResult* notify = new (std::nothrow) OpenCameraNT(
        msg->pipe_inst, 
        cmd,
        this);
    MC::STSealAPI::GetInst()->OpenCamera(
        cmd->which_,
        notify);
}

////////////////////////////

class CloseCameraNT: public MC::NotifyResult {
public:
    CloseCameraNT(LPPIPEINST inst, CloseCameraCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "CloseCameraNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    CloseCameraCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCloseCamera(const RecvMsg* msg)
{
    CloseCameraCmd* cmd = new (std::nothrow) CloseCameraCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCloseCamera->关闭摄像头\n");

    MC::NotifyResult* notify = new (std::nothrow) CloseCameraNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CloseCamera(
        cmd->which_,
        notify);
}

////////////////////////////

class QueryCameraNT: public MC::NotifyResult {
public:
    QueryCameraNT(LPPIPEINST inst, QueryCameraStatusCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "QueryCameraNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->status_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QueryCameraStatusCmd*   cmd_;
    LPPIPEINST              pipe_inst_;
    Recver*                 recver_;
};

void Recver::HandleGetCameraStatus(const RecvMsg* msg)
{
    QueryCameraStatusCmd* cmd = new (std::nothrow) QueryCameraStatusCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleGetCameraStatus->摄像头状态\n");

    MC::NotifyResult* notify = new (std::nothrow) QueryCameraNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryCameraStatus(
        cmd->which_,
        notify);
}

////////////////////////////

class SetResolutionNT: public MC::NotifyResult {
public:
    SetResolutionNT(LPPIPEINST inst, SetResolutionCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "SetResolutionNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    SetResolutionCmd*   cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSetResolution(const RecvMsg* msg)
{
    SetResolutionCmd* cmd = new (std::nothrow) SetResolutionCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSetResolution->设置分辨率\n");

    MC::NotifyResult* notify = new (std::nothrow) SetResolutionNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SetResolution(
        cmd->which_,
        cmd->x_,
        cmd->y_,
        notify);
}

////////////////////////////

class SetDPINT: public MC::NotifyResult {
public:
    SetDPINT(LPPIPEINST inst, SetDPICmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "SetDPINT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    SetDPICmd*          cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSetDPI(const RecvMsg* msg)
{
    SetDPICmd* cmd = new (std::nothrow) SetDPICmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSetDPI->设置DPI\n");

    MC::NotifyResult* notify = new (std::nothrow) SetDPINT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SetDPI(
        cmd->which_,
        cmd->dpi_x_,
        cmd->dpi_y_,
        notify);
}

////////////////////////////

class SetPropertyNT: public MC::NotifyResult {
public:
    SetPropertyNT(LPPIPEINST inst, SetPropertyCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        int err_code = 0;
        std::cout << "SetPropertyNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    SetPropertyCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleSetProperty(const RecvMsg* msg)
{
    SetPropertyCmd* cmd = new (std::nothrow) SetPropertyCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleSetProperty->设置摄像头属性\n");

    MC::NotifyResult* notify = new (std::nothrow) SetPropertyNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->SetProperty(
        cmd->which_,
        notify);
}

////////////////////////////

class RecordVideoNT: public MC::NotifyResult {
public:
    RecordVideoNT(LPPIPEINST inst, RecordVideoCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "RecordVideoNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    RecordVideoCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleRecordVideo(const RecvMsg* msg)
{
    RecordVideoCmd* cmd = new (std::nothrow) RecordVideoCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleRecordVideo->录像\n");

    MC::NotifyResult* notify = new (std::nothrow) RecordVideoNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->RecordVideo(
        cmd->which_,
        cmd->path_,
        notify);
}

////////////////////////////

class StopRecordVideoNT: public MC::NotifyResult {
public:
    StopRecordVideoNT(LPPIPEINST inst, StopRecordVideoCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "StopRecordVideoNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    StopRecordVideoCmd* cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleStopRecordVideo(const RecvMsg* msg)
{
    StopRecordVideoCmd* cmd = new (std::nothrow) StopRecordVideoCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleStopRecordVideo->停止录像\n");

    MC::NotifyResult* notify = new (std::nothrow) StopRecordVideoNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->StopRecordVideo(
        cmd->which_,
        cmd->path_,
        notify);
}

////////////////// 获取rfid /////////////

class GetRFIDNT: public MC::NotifyResult {
public:
    GetRFIDNT(LPPIPEINST inst, GetRFIDCmd* cmd, Recver* recv) :
            pipe_inst_(inst),
            cmd_(cmd),
            recver_(recv)
    {

    }

    void Notify(
            MC::ErrorCode ec,
            std::string data1 = "",
            std::string data2 = "",
            std::string ctx1 = "",
            std::string ctx2 = "")
    {
        std::cout << "GetRFIDNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        strcpy(cmd_->rfid_, data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    GetRFIDCmd*         cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleGetRFID(const RecvMsg* msg)
{
    GetRFIDCmd* cmd = new (std::nothrow) GetRFIDCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleGetRFID->获取rfid\n");

    MC::NotifyResult* notify = new (std::nothrow) GetRFIDNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->GetRFID(
            cmd->slot_,
            notify);
}

////////////////// 获取设备状态 /////////////

class GetDevStatusNT : public MC::NotifyResult {
public:
    GetDevStatusNT(LPPIPEINST inst, GetDevStatusCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "GetDevStatusNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->status_code_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    GetDevStatusCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleGetDevStatus(const RecvMsg* msg)
{
    GetDevStatusCmd* cmd = new (std::nothrow) GetDevStatusCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleGetDevStatus->获取设备状态\n");

    MC::NotifyResult* notify = new (std::nothrow) GetDevStatusNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->GetDevStatus(notify);
}

////////////////// 获取设备状态 /////////////

class CvtCoordNT : public MC::NotifyResult {
public:
    CvtCoordNT(LPPIPEINST inst, CoordCvtCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "CvtCoordNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->x_dev_ = atoi(data1.c_str());
        cmd_->y_dev_ = atoi(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    CoordCvtCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCvtCoord(const RecvMsg* msg)
{
    CoordCvtCmd* cmd = new (std::nothrow) CoordCvtCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleCvtCoord->坐标转换\n");

    MC::NotifyResult* notify = new (std::nothrow) CvtCoordNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->GetDevStatus(notify);
}

//////////////////////////////////////////////////////////////////////////

void Recver::HandleHeart(const RecvMsg* msg)
{
    HeartCmd heart_cmd;
    memcpy(heart_cmd.xs_.buf_, msg->msg, CMD_BUF_SIZE);
    heart_cmd.Unser();

    bool suc = this->WriteResp(msg->pipe_inst, heart_cmd.xs_.GetBuf(), 1);
}

////////////////// 写转换倍率 /////////////

class WriteRatioNT : public MC::NotifyResult {
public:
    WriteRatioNT(LPPIPEINST inst, WriteRatioCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "WriteRatioNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    WriteRatioCmd*      cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleWriteRatio(const RecvMsg* msg)
{
    WriteRatioCmd* cmd = new (std::nothrow) WriteRatioCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleWriteRatio->写倍率\n");

    MC::NotifyResult* notify = new (std::nothrow) WriteRatioNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->WriteRatio(cmd->x_, cmd->y_, notify);
}

////////////////// 读转换倍率 /////////////

class ReadRatioNT : public MC::NotifyResult {
public:
    ReadRatioNT(LPPIPEINST inst, ReadRatioCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "ReadRatioNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        cmd_->x_ = (float)atof(data1.c_str());
        cmd_->y_ = (float)atof(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    ReadRatioCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleReadRatio(const RecvMsg* msg)
{
    ReadRatioCmd* cmd = new (std::nothrow) ReadRatioCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleReadRatio->读倍率\n");

    MC::NotifyResult* notify = new (std::nothrow) ReadRatioNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->ReadRatio(notify);
}

////////////////// 写校准点 /////////////

class WriteCaliNT : public MC::NotifyResult {
public:
    WriteCaliNT(LPPIPEINST inst, WriteCaliPtsCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "WriteCaliNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    WriteCaliPtsCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleWriteCali(const RecvMsg* msg)
{
    WriteCaliPtsCmd* cmd = new (std::nothrow) WriteCaliPtsCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleReadRatio->写校准点\n");

    MC::NotifyResult* notify = new (std::nothrow) WriteCaliNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->WriteCalibration(cmd->pts_, cmd->len_, notify);
}

////////////////// 读校准点 /////////////

class ReadCaliNT : public MC::NotifyResult {
public:
    ReadCaliNT(LPPIPEINST inst, ReadCaliPtsCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "ReadCaliNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        if (0 == ec) {
            long pts_addr = atol(data1.c_str());
            for (int i = 0; i < 10; ++i) {
                memcpy(
                    &cmd_->pts_[i],
                    (void*)(pts_addr + i * sizeof(unsigned short)),
                    sizeof(unsigned short));

                Log::WriteLog(LL_DEBUG, "校准点: %u", cmd_->pts_[i]);
            }
        }

        recver_->PushCmd(cmd_);
    }

private:
    ReadCaliPtsCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleReadCali(const RecvMsg* msg)
{
    ReadCaliPtsCmd* cmd = new (std::nothrow) ReadCaliPtsCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();
    printf("Recver::HandleReadCali->读校准点\n");

    MC::NotifyResult* notify = new (std::nothrow) ReadCaliNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->ReadCalibration(notify);
}

////////////////// 顶盖门状态 /////////////

class ReadTopNT : public MC::NotifyResult {
public:
    ReadTopNT(LPPIPEINST inst, QueryTopCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "ReadTopNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;
        if (MC::EC_SUCC == ec)
            cmd_->status_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    QueryTopCmd*        cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleQueryTop(const RecvMsg* msg)
{
    QueryTopCmd* cmd = new (std::nothrow) QueryTopCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) ReadTopNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->QueryTop(notify);
}

////////////////// 进入维护模式 /////////////

class EnterMainNT : public MC::NotifyResult {
public:
    EnterMainNT(LPPIPEINST inst, EnterMaintainCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "EnterMainNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    EnterMaintainCmd*   cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleEnterMain(const RecvMsg* msg)
{
    EnterMaintainCmd* cmd = new (std::nothrow) EnterMaintainCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) EnterMainNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->EnterMaintain(notify);
}

////////////////// 退出维护模式 /////////////

class ExitMainNT : public MC::NotifyResult {
public:
    ExitMainNT(LPPIPEINST inst, ExitMaintainCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "ExitMainNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    ExitMaintainCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleExitMain(const RecvMsg* msg)
{
    ExitMaintainCmd* cmd = new (std::nothrow) ExitMaintainCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) ExitMainNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->ExitMaintain(notify);
}

////////////////// 开始预览 /////////////

class StartPreNT : public MC::NotifyResult {
public:
    StartPreNT(LPPIPEINST inst, StartPreviewCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "StartPreNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    StartPreviewCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleStartPreview(const RecvMsg* msg)
{
    StartPreviewCmd* cmd = new (std::nothrow) StartPreviewCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) StartPreNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->StartPreview(
        cmd->which_,
        cmd->width_,
        cmd->height_,
        cmd->hwnd_,
        notify);
}

////////////////// 停止预览 /////////////

class StopPreNT : public MC::NotifyResult {
public:
    StopPreNT(LPPIPEINST inst, StopPreviewCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "StopPreNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    StopPreviewCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleStopPreview(const RecvMsg* msg)
{
    StopPreviewCmd* cmd = new (std::nothrow) StopPreviewCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) StopPreNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->StopPreview(
        cmd->which_,
        notify);
}

////////////////// 工厂模式 /////////////

class CtrlFactoryNT : public MC::NotifyResult {
public:
    CtrlFactoryNT(LPPIPEINST inst, CtrlFactoryCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        std::cout << "CtrlFactoryNT::Notify->ec: " << ec << std::endl;

        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    CtrlFactoryCmd*     cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleFactoryCtrl(const RecvMsg* msg)
{
    CtrlFactoryCmd* cmd = new (std::nothrow) CtrlFactoryCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) CtrlFactoryNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CtrlFactory(
        cmd->ctrl_,
        notify);
}

////////////////// 复位 /////////////

class ResetNT : public MC::NotifyResult {
public:
    ResetNT(LPPIPEINST inst, ResetCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    ResetCmd*           cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleReset(const RecvMsg* msg)
{
    ResetCmd* cmd = new (std::nothrow) ResetCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) ResetNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Reset(notify);
}

////////////////// 重启 /////////////

class RestartNT : public MC::NotifyResult {
public:
    RestartNT(LPPIPEINST inst, RestartCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    RestartCmd*         cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleRestart(const RecvMsg* msg)
{
    RestartCmd* cmd = new (std::nothrow) RestartCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) RestartNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Restart(notify);
}

////////////////// 获取系统信息 /////////////

class GetSystemNT : public MC::NotifyResult {
public:
    GetSystemNT(LPPIPEINST inst, GetSystemCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;
        cmd_->status_ = atoi(data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    GetSystemCmd*       cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleGetSystem(const RecvMsg* msg)
{
    GetSystemCmd* cmd = new (std::nothrow) GetSystemCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) GetSystemNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->GetSystem(notify);
}

////////////////// read main/spare sn /////////////

class ReadMainSpareNT : public MC::NotifyResult {
public:
    ReadMainSpareNT(LPPIPEINST inst, ReadMainSpareCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;
        strcpy(cmd_->sn_, data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    ReadMainSpareCmd*   cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleReadMainSpare(const RecvMsg* msg)
{
    ReadMainSpareCmd* cmd = new (std::nothrow) ReadMainSpareCmd();
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) ReadMainSpareNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->ReadMainSpareSN(notify);
}

////////////////// write main/spare sn /////////////

class WriteMainSpareNT : public MC::NotifyResult {
public:
    WriteMainSpareNT(LPPIPEINST inst, WriteMainSpareCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;

        recver_->PushCmd(cmd_);
    }

private:
    WriteMainSpareCmd*  cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleWriteMainSpare(const RecvMsg* msg)
{
    WriteMainSpareCmd* cmd = new (std::nothrow) WriteMainSpareCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) WriteMainSpareNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->WriteMainSpareSN(cmd->sn_, notify);
}

////////////////// recognize qr code /////////////

class RecogQRCodeNT : public MC::NotifyResult {
public:
    RecogQRCodeNT(LPPIPEINST inst, RecogQRCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;
        strcpy(cmd_->qr_code_, data1.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    RecogQRCmd*         cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleRecogQR(const RecvMsg* msg)
{
    RecogQRCmd* cmd = new (std::nothrow) RecogQRCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) RecogQRCodeNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->RecogQRCode(
        cmd->file_,
        cmd->left_,
        cmd->top_,
        cmd->right_,
        cmd->bottom_,
        notify);
}

////////////////// calc image ratio /////////////

class CalcRatioNT : public MC::NotifyResult {
public:
    CalcRatioNT(LPPIPEINST inst, CalculateRatioCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;
        cmd_->ratio_x_ = atof(data1.c_str());
        cmd_->ratio_y_ = atof(data2.c_str());

        recver_->PushCmd(cmd_);
    }

private:
    CalculateRatioCmd*  cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleCalcRatio(const RecvMsg* msg)
{
    CalculateRatioCmd* cmd = new (std::nothrow) CalculateRatioCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) CalcRatioNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->CalculateRatio(
        cmd->file_,
        cmd->dpi_,
        notify);
}

////////////////// find 2 circles /////////////

class Find2CirclesNT : public MC::NotifyResult {
public:
    Find2CirclesNT(LPPIPEINST inst, Find2CirclesCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;

        std::vector<std::string> data_set;
        char* str = (char*)data1.c_str();
        char* pch;

        pch = strtok(str, ",");
        while (pch != NULL) {
            //printf("%s\n", pch);
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x1_ = atoi(data_set.at(0).c_str());
        cmd_->y1_ = atoi(data_set.at(1).c_str());
        cmd_->radius1_ = atoi(data_set.at(2).c_str());

        // circle2
        data_set.clear();

        str = (char*)data2.c_str();
        pch = strtok(str, ",");
        while (pch != NULL) {
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x2_ = atoi(data_set.at(0).c_str());
        cmd_->y2_ = atoi(data_set.at(1).c_str());
        cmd_->radius2_ = atoi(data_set.at(2).c_str());

        recver_->PushCmd(cmd_);
    }

private:
    Find2CirclesCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleFind2Circles(const RecvMsg* msg)
{
    Find2CirclesCmd* cmd = new (std::nothrow) Find2CirclesCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) Find2CirclesNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Find2Circles(
        cmd->file_,
        notify);
}

////////////////// find 4 circles /////////////

class Find4CirclesNT : public MC::NotifyResult {
public:
    Find4CirclesNT(LPPIPEINST inst, Find4CirclesCmd* cmd, Recver* recv) :
        pipe_inst_(inst),
        cmd_(cmd),
        recver_(recv)
    {

    }

    void Notify(
        MC::ErrorCode ec,
        std::string data1 = "",
        std::string data2 = "",
        std::string ctx1 = "",
        std::string ctx2 = "")
    {
        cmd_->ret_ = ec;

        std::vector<std::string> data_set;
        char* str = (char*)data1.c_str();
        char* pch;

        pch = strtok(str, ",");
        while (pch != NULL) {
            //printf("%s\n", pch);
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x1_ = atoi(data_set.at(0).c_str());
        cmd_->y1_ = atoi(data_set.at(1).c_str());
        cmd_->radius1_ = atoi(data_set.at(2).c_str());

        // circle2
        data_set.clear();

        str = (char*)data2.c_str();
        pch = strtok(str, ",");
        while (pch != NULL) {
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x2_ = atoi(data_set.at(0).c_str());
        cmd_->y2_ = atoi(data_set.at(1).c_str());
        cmd_->radius2_ = atoi(data_set.at(2).c_str());

        // circle3
        data_set.clear();

        str = (char*)ctx1.c_str();
        pch = strtok(str, ",");
        while (pch != NULL) {
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x3_ = atoi(data_set.at(0).c_str());
        cmd_->y3_ = atoi(data_set.at(1).c_str());
        cmd_->radius3_ = atoi(data_set.at(2).c_str());

        // circle4
        data_set.clear();

        str = (char*)ctx2.c_str();
        pch = strtok(str, ",");
        while (pch != NULL) {
            data_set.push_back(pch);
            pch = strtok(NULL, ",");
        }
        cmd_->x4_ = atoi(data_set.at(0).c_str());
        cmd_->y4_ = atoi(data_set.at(1).c_str());
        cmd_->radius4_ = atoi(data_set.at(2).c_str());

        recver_->PushCmd(cmd_);
    }

private:
    Find4CirclesCmd*    cmd_;
    LPPIPEINST          pipe_inst_;
    Recver*             recver_;
};

void Recver::HandleFind4Circles(const RecvMsg* msg)
{
    Find4CirclesCmd* cmd = new (std::nothrow) Find4CirclesCmd;
    memcpy(cmd->xs_.buf_, msg->msg, CMD_BUF_SIZE);
    cmd->Unser();

    MC::NotifyResult* notify = new (std::nothrow) Find4CirclesNT(msg->pipe_inst, cmd, this);
    MC::STSealAPI::GetInst()->Find4Circles(
        cmd->file_,
        notify);
}
