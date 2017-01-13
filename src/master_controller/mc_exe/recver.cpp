#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/exception/all.hpp>
#include "common_definitions.h"
#include "recver.h"
#include "agent_cmd.h"
#include "boc_api.h"

#pragma warning(disable:4995)
#pragma warning(disable:4996)

Recver::~Recver()
{
    Stop();
}

bool Recver::ParseConfig()
{
    std::string path;
    if (!MC::GetMoudulePath(path))
        return false;

    std::string xml_path = path + "mc.xml";
    try {
        boost::property_tree::xml_parser::read_xml(xml_path, svr_config_pt_);
    } catch (...) {
        boost::exception_ptr e = boost::current_exception();
        Log::WriteLog(LL_ERROR, "Recver::ParseConfig->%s",
            boost::current_exception_diagnostic_information());
        return false;
    }

    return true;
}

MC::ConnType Recver::CnnType() const
{
    return cnn_type_;
}

bool Recver::Start()
{
    if (!ParseConfig()) {
        printf("Recver::Start->解析配置文件失败\n");
        Log::WriteLog(LL_ERROR, "Recver::Start->解析配置文件失败");
        return false;
    }

    std::string type = svr_config_pt_.get<std::string>("con.type");
    if (type == "PIPE")
        cnn_type_ = MC::CT_PIPE;
    else if (type == "MQ")
        cnn_type_ = MC::CT_MQ;
    else
        cnn_type_ = MC::CT_MAX;

    if (MC::CT_MQ == cnn_type_) {
        recv_mq_name_ = svr_config_pt_.get<std::string>("con.name");
        send_mq_name_ = svr_config_pt_.get<std::string>("con.name") + "copy";

        try {
            boost::interprocess::message_queue::remove(recv_mq_name_.c_str());
            boost::interprocess::message_queue::remove(send_mq_name_.c_str());

            recv_mq_ = new (std::nothrow) boost::interprocess::message_queue(
                boost::interprocess::create_only,
                recv_mq_name_.c_str(),
                MC::MQ_MAX_MSG_NUM,
                MC::MQ_MAX_MSG_SIZE);

            send_mq_ = new boost::interprocess::message_queue(
                boost::interprocess::create_only,
                send_mq_name_.c_str(),
                MC::MQ_MAX_MSG_NUM,
                MC::MQ_MAX_MSG_SIZE);
        } catch (boost::interprocess::interprocess_exception &ex) {
            std::cout << ex.what() << std::endl;
            Log::WriteLog(LL_ERROR, "Recver::Start->消息队列创建失败: %s", ex.what());
            return false;
        }
    } else if (MC::CT_PIPE == cnn_type_) {
        recv_msg_ev_ = CreateEvent(
            NULL,       // 默认属性
            TRUE,       // 手工reset
            FALSE,      // 初始状态 signaled 
            NULL);      // 未命名
    } else {
        Log::WriteLog(LL_ERROR, "Recver::Start->未知的通信连接方式, %d", cnn_type_);
        return false;
    }

    running_ = true;
    recver_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&Recver::ReceiveFunc, this));

    printf("Recver::Start->start succ\n");
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
    if (!running_)
        return;

    running_ = false;
    recver_thread_->join();

    if (cnn_type_ == MC::CT_MQ) {
        boost::interprocess::message_queue::remove(recv_mq_name_.c_str());
        boost::interprocess::message_queue::remove(send_mq_name_.c_str());

        delete recv_mq_;
        delete send_mq_;
    }
}

bool Recver::WriteResp(LPPIPEINST pipe_inst_, char* buf)
{
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

    // 消息队列连接
    try {
        LPTSTR lpvMessage = TEXT(buf);
        send_mq_->send(lpvMessage, (lstrlen(lpvMessage) + 1) * sizeof(TCHAR), 0);
    } catch (boost::interprocess::interprocess_exception &ex) {
        Log::WriteLog(LL_ERROR, "Recver::WriteResp->消息队列方式写回复失败, er: %s", ex.what());
        return false;
    }

    return true;
}

// 接收来自客户端请求的线程, 反序列化, 分配
void Recver::ReceiveFunc()
{
    const RecvMsg* msg = NULL;
    while (running_) {
        char cmd;       // 消息头

        if (MC::CT_MQ == cnn_type_) {   // 消息队列
            msg = new RecvMsg;

            unsigned int priority;
            boost::interprocess::message_queue::size_type recvd_size;
            char buf[CMD_BUF_SIZE] = { 0 };
            try {
                recv_mq_->receive(buf, sizeof(buf), recvd_size, priority);  // blocked if empty
                memcpy(&cmd, buf, sizeof(char));

                const_cast<RecvMsg*>(msg)->pipe_inst = NULL;
                strcpy(const_cast<RecvMsg*>(msg)->msg, buf);
            } catch (boost::interprocess::interprocess_exception &ex) {
                std::cout << ex.what() << std::endl;
                Log::WriteLog(LL_ERROR, "Recver::ReceiveFunc->消息队列收消息失败, %s", ex.what());
                continue;
            }
        } else {
            DWORD wait_result = WaitForSingleObject(recv_msg_ev_, INFINITE);
            if (0 != wait_result)
                continue;

            ResetEvent(recv_msg_ev_);
            recver_queue_.Pop(msg);
            memcpy(&cmd, msg->msg, sizeof(char));
        }

//        printf("Recver::ReceiveFunc->Received msg type: (%d)\n", cmd);
        switch (cmd) {
        case CT_QUERY_MACHINE:
            HandleQueryMachine(msg);
            break;
        case CT_SET_MACHINE:
            HandleSetMachine(msg);
            break;
        case CT_INIT_MACHINE:
            HandleInitMachine(msg);
            break;
        case CT_BIND_MAC:
            HandleBindMAC(msg);
            break;
        case CT_UNBIND_MAC:
            HandleUnbindMAC(msg);
            break;
        case CT_PREPARE_STAMP:
            HandlePrepareStamp(msg);
            break;
        case CT_PAPER_DOOR:
            HandleQueryPaper(msg);
            break;
        case CT_SNAPSHOT:
            HandleSnapshot(msg);
            break;
        case CT_PHOTO_SYNTHESIS:
            HandleMergePhoto(msg);
            break;
        case CT_RECOGNITION:
            HandleRecognition(msg);
            break;
        case CT_ELEMENT_IDENTI:
            HandleElementIdenti(msg);
            break;
        case CT_ORDINARY_STAMP:
            HandleOrdinary(msg);
            break;
        case CT_AUTO_STAMP:
            HandleAuto(msg);
            break;
        case CT_FINISH_STAMP:
            HandleFinish(msg);
            break;
        case CT_RELEASE_STAMPER:
            HandleReleaseStamper(msg);
            break;
        case CT_GET_ERROR:
            HandleGetError(msg);
            break;
        case CT_CALIBRATION:
            HandleCalibrate(msg);
            break;
        case CT_HEART_BEAT:
            HandleHeart(msg);
            break;
        case CT_QUERY_STAMPERS:
            HandleQueryStampers(msg);
            break;
        case CT_QUERY_SAFE:
            HandleQuerySafe(msg);
            break;
        case CT_SAFE_CTL:
            HandleSafeControl(msg);
            break;
        case CT_BEEP_CTL:
            HandleBeepControl(msg);
            break;
        case CT_QUERY_SLOT:
            HandleQuerySlot(msg);
            break;
        case CT_ALARM_CTL:
            HandleAlarmControl(msg);
            break;
        case CT_QUERY_MAC:
            HandleQueryMAC(msg);
            break;
        default:
            printf("Recver::ReceiverFunc->Unknown cmd: %d\n", cmd);
            break;
        }

        delete msg;
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QueryMachine(notify);
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

/*        strcpy_s(cmd_->sn_, data1.c_str());*/
        cmd_->ret_ = ec;
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->SetMachine(cmd->sn_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->InitMachine(init_cmd->key_, notify);
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

        //回复结果
        cmd_->ret_ = ec;
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->BindMAC(bind_cmd->mac_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->UnbindMAC(unbind_cmd->mac_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->PrepareStamp(prepare_cmd->stamper_id_, prepare_cmd->timeout_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QueryPaperDoor(notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->Snapshot(
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->MergePhoto(merge_cmd->photo1_, merge_cmd->photo2_, merge_cmd->merged_, notify);
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
        strcpy_s(cmd_->template_id_, ctx1.c_str());
        strcpy_s(cmd_->trace_num_, ctx2.c_str());
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->RecognizeImage(recog_cmd->path_, notify);
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
            ", 识别图片路径: " << data1.c_str() << ", 识别结果: " << data2.c_str() << std::endl;

        cmd_->ret_ = ec;
        strcpy_s(cmd_->content_str_, data2.c_str());
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->IdentifyElement(
        identi_cmd->path_,
        identi_cmd->x_,
        identi_cmd->y_,
        identi_cmd->width_,
        identi_cmd->height_,
        identi_cmd->angle_,
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->OrdinaryStamp(
        ordi_cmd->task_id_,
        ordi_cmd->type_,
        ordi_cmd->stamper_num_,
        ordi_cmd->ink_,
        ordi_cmd->x_,
        ordi_cmd->y_,
        ordi_cmd->angle_,
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->AutoStamp(
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->FinishStamp(finish_cmd->task_id_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->ReleaseStamp(release_cmd->stamp_id_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->GetError(err_cmd->err_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->CalibrateMachine(cmd->slot_, notify);
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
                if (data1.at(i) == 0x30)
                    cmd_->stamper_status_[i] = 0;
                else if (data1.at(i) == 0x31)
                    cmd_->stamper_status_[i] = 1;
            }
        }
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QueryStampers(notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QuerySafeDoor(notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->OperateSafeDoor(cmd->ctrl_, cmd->timeout_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->OperateBeep(cmd->ctrl_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QuerySlot(notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->OperateAlarm(cmd->alarm_, cmd->ctrl_, notify);
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
        cmd_->Ser();

        bool suc = recver_->WriteResp(pipe_inst_, cmd_->xs_.GetBuf());
        delete cmd_;
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
    MC::BOCApi::GetInst()->QueryMAC(notify);
}

//////////////////////////////////////////////////////////////////////////

void Recver::HandleHeart(const RecvMsg* msg)
{
    HeartCmd heart_cmd;
    memcpy(heart_cmd.xs_.buf_, msg->msg, CMD_BUF_SIZE);
    heart_cmd.Unser();

    bool suc = this->WriteResp(msg->pipe_inst, heart_cmd.xs_.GetBuf());
}
