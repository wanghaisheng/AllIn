#include <windows.h>
#include <TlHelp32.h>
#include <shellapi.h>
#include <boost/exception/all.hpp>
#include "common_definitions.h"
#include "log.h"
#include "parse.h"
#include "cnn.h"

MC::Cnn* MC::Cnn::cnn_inst = NULL;

bool MC::Cnn::StartPipe(const char* cnn_name)
{
    if (pipe_ != INVALID_HANDLE_VALUE)
        return true;

    LPTSTR lpszPipename = TEXT((char*)cnn_name);
    pipe_ = CreateFile(
        lpszPipename,           // pipe 名 
        GENERIC_READ | GENERIC_WRITE,        // 可读可写
        0,                      // 不共享
        NULL,                   // 默认安全属性
        OPEN_EXISTING,          // 已经存在(由服务端创建)
        FILE_FLAG_OVERLAPPED,   // 默认属性
        NULL);
    if (pipe_ != INVALID_HANDLE_VALUE)
        goto NOR;

    // 如果不是 ERROR_PIPE_BUSY 错误, 直接退出  
    if (GetLastError() != ERROR_PIPE_BUSY) {
        printf("1->Could not open pipe, err: %d\n", GetLastError());
        return false;
    }

    // 如果所有pipe实例都处于繁忙状态, 等待2秒
    if (!WaitNamedPipe(lpszPipename, PIPE_BUSY_WAIT)) {
        printf("2->Could not open pipe\n");
        return false;
    }

    BOOL fSuccess;
    DWORD dwMode;
    // pipe已经连接, 设置为消息读状态
    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
        pipe_,    // 句柄
        &dwMode,  // 新状态
        NULL,     // 不设置最大缓存
        NULL);    // 不设置最长时间
    if (!fSuccess) {
        printf("SetNamedPipeHandleState failed, err: %d\n", GetLastError());
        return false;
    }

NOR:
    Log::WriteLog(LL_DEBUG, "AsynAPISet::StartPipe->CreateFile, pipe: %d", (int)pipe_);
    pipe_inst_ = (LPPIPEINST)HeapAlloc(GetProcessHeap(), 0, sizeof(PIPEINST));
    if (pipe_inst_ == NULL) {
        printf("APISet::Start->HeapAlloc failed, err: (%d)\n", GetLastError());
        CloseHandle(pipe_);
        return false;
    }

    pipe_inst_->hPipeInst = pipe_;
    pipe_inst_->cbToWrite = 0;
    return true;
}

bool MC::Cnn::StartMQ(
    const std::string& send_mq_name,
    const std::string& recv_mq_name)
{
    try {
        send_mq_ = new (std::nothrow) boost::interprocess::message_queue(
            boost::interprocess::open_only,
            send_mq_name.c_str());
        recv_mq_ = new (std::nothrow) boost::interprocess::message_queue(
            boost::interprocess::open_only,
            recv_mq_name.c_str());
    } catch (boost::interprocess::interprocess_exception &ex) {
        Log::WriteLog(LL_ERROR, "MC::Cnn::StartMQ->打开消息队列(send: %s, recv: %s)失败, er: %s",
            send_mq_name.c_str(),
            recv_mq_name.c_str(),
            boost::current_exception_diagnostic_information().c_str());
        return false;
    }

    return true;
}

bool MC::Cnn::Start()
{
    if (running_)
        return true;

    if (!ProcessExisted(MC::SERVER_NAME)) {
        bool ret = StartSvc(MC::SERVER_NAME);
        Log::WriteLog(LL_DEBUG, "AsynAPISet::Start->启动服务进程\"%s\"%s",
            MC::SERVER_NAME.c_str(),
            ret ? "成功" : "失败");
    }

    Sleep(2000); // 适当延时, 以便消息队列准备好

    bool ret;
    if (AgentConfig::GetInst()->conn_type_ == CT_PIPE)
        ret = StartPipe(AgentConfig::GetInst()->pipe_name_.c_str());
    else if (AgentConfig::GetInst()->conn_type_ == CT_MQ)
        ret = StartMQ(AgentConfig::GetInst()->send_mq_name_, AgentConfig::GetInst()->recv_mq_name_);

    send_ev_ = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL);
    running_ = true;
    // 收消息线程
    recver_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&Cnn::ReceiveFunc, this));
    // 发消息线程
    sender_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&Cnn::SendFunc, this));

    heart_ev_ = CreateEvent(
        NULL,
        FALSE,
        TRUE,      // default signaled 
        NULL);
    // 心跳线程
    heart_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&Cnn::HeartBeatingFunc, this));

    return true;
}

void MC::Cnn::Stop()
{
    running_ = false;
    recver_thread_->join();
    sender_thread_->join();
    heart_thread_->join();

    delete send_mq_;
    delete recv_mq_;
    CloseHandle(pipe_);
    if (NULL != pipe_inst_)
        HeapFree(GetProcessHeap(), 0, pipe_inst_);
}

bool MC::Cnn::PushCmd(BaseCmd* cmd)
{
    return send_cmd_queue_.Push(cmd);
}

bool MC::Cnn::RecvBuf(TCHAR* buf, int buf_len, DWORD* actual_read)
{
    bool fSuccess = true;
    if (MC::CT_PIPE == AgentConfig::GetInst()->conn_type_) {      // 收管道消息
        fSuccess = ReadFile(
            pipe_,                          // 句柄
            buf,                            // 读取内容的缓存
            CMD_BUF_SIZE * sizeof(TCHAR),   // 缓存大小
            actual_read,                    // 实际读的字节
            NULL) == 0? false: true;        // 非 overlapped
    } else {                                                // 消息队列
        unsigned int priority = 0;
        boost::interprocess::message_queue::size_type recvd_size;
        try {
            heart_mtx_.lock();
            if (recv_mq_->try_receive(buf, buf_len, recvd_size, priority))
                *actual_read = recvd_size;
            heart_mtx_.unlock();
        } catch (boost::interprocess::interprocess_exception &ex) {
            std::cout << ex.what() << std::endl;
            Log::WriteLog(LL_ERROR, "AsynAPISet::ReceiverFunc->消息队列异常: %s", ex.what());
            *actual_read = 0;
            fSuccess = false;
        }
    }

    return fSuccess;
}

void MC::Cnn::ReceiveFunc()
{
    TCHAR chBuf[CMD_BUF_SIZE] = { 0 };
    DWORD cbRead;
    while (true) {
        DWORD suc = WaitForSingleObject(send_ev_, INFINITE);
        if (suc == WAIT_TIMEOUT)
            continue;

        RecvBuf(chBuf, sizeof(chBuf), &cbRead);
        // 如实际读字节为0, 结束本次循环
        if (0 == cbRead)
            continue;

        char cmd_type = GetCmdHeader(chBuf);
        switch (cmd_type) {
        case CT_QUERY_MACHINE:
            asyn_api_->HandleQueryMachine(chBuf);
            break;
        case CT_SET_MACHINE:
            asyn_api_->HandleSetMachine(chBuf);
            break;
        case CT_INIT_MACHINE:
            asyn_api_->HandleInitMachine(chBuf);
            break;
        case CT_BIND_MAC:
            asyn_api_->HandleBindMac(chBuf);
            break;
        case CT_UNBIND_MAC:
            asyn_api_->HandleUnbindMAC(chBuf);
            break;
        case CT_PREPARE_STAMP:
            asyn_api_->HandlePrepareStamp(chBuf);
            break;
        case CT_PAPER_DOOR:
            asyn_api_->HandleQueryPaper(chBuf);
            break;
        case CT_SNAPSHOT:
            asyn_api_->HandleSnapshot(chBuf);
            break;
        case CT_PHOTO_SYNTHESIS:
            asyn_api_->HandleMergePhoto(chBuf);
            break;
        case CT_SEARCH_STAMP:
            asyn_api_->HandleSearchStamp(chBuf);
            break;
        case CT_RECOG_MODEL:
            asyn_api_->HandleRecogModelPoint(chBuf);
            break;
        case CT_RECOGNITION:
            asyn_api_->HandleRecognition(chBuf);
            break;
        case CT_ELEMENT_IDENTI:
            asyn_api_->HandleIdentify(chBuf);
            break;
        case CT_ORDINARY_STAMP:
            asyn_api_->HandleOrdinary(chBuf);
            break;
        case CT_AUTO_STAMP:
            asyn_api_->HandleAuto(chBuf);
            break;
        case CT_FINISH_STAMP:
            asyn_api_->HandleFinish(chBuf);
            break;
        case CT_RELEASE_STAMPER:
            asyn_api_->HandleRelease(chBuf);
            break;
        case CT_GET_ERROR:
            asyn_api_->HandleGetError(chBuf);
            break;
        case CT_CALIBRATION:
            asyn_api_->HandleCalibrate(chBuf);
            break;
        case CT_QUERY_STAMPERS:
            asyn_api_->HandleQueryStampers(chBuf);
            break;
        case CT_QUERY_SAFE:
            asyn_api_->HandleQuerySafe(chBuf);
            break;
        case CT_SAFE_CTL:
            asyn_api_->HandleSafeControl(chBuf);
            break;
        case CT_BEEP_CTL:
            asyn_api_->HandleBeepControl(chBuf);
            break;
        case CT_QUERY_SLOT:
            asyn_api_->HandleQuerySlot(chBuf);
            break;
        case CT_ALARM_CTL:
            asyn_api_->HandleAlarmControl(chBuf);
            break;
        case CT_QUERY_ALARM:
            asyn_api_->HandleQueryAlarm(chBuf);
            break;
        case CT_QUERY_MAC:
            asyn_api_->HandleQueryMAC(chBuf);
            break;
        case CT_HEART_BEAT:
            HandleHeartBeating(chBuf);
            break;
        case CT_LOCK:
            asyn_api_->HandleLock(chBuf);
            break;
        case CT_UNLOCK:
            asyn_api_->HandleUnlock(chBuf);
            break;
        case CT_LOCK_STATUS:
            asyn_api_->HandleQueryLock(chBuf);
            break;
        case CT_OPEN_CON:
            asyn_api_->HandleOpenCnn(chBuf);
            break;
        case CT_CLOSE_CON:
            asyn_api_->HandleCloseCnn(chBuf);
            break;
        case CT_QUERY_CON:
            asyn_api_->HandleQueryCnn(chBuf);
            break;
        case CT_SIDE_DOOR_ALARM:
            asyn_api_->HandleSetSideAlarm(chBuf);
            break;
        case CT_QUERY_DEV_MODEL:
            asyn_api_->HandleGetModel(chBuf);
            break;
        case CT_OPEN_PAPER:
            asyn_api_->HandleOpenPaper(chBuf);
            break;
        case CT_LED_CTL:
            asyn_api_->HandleCtrlLed(chBuf);
            break;
        case CT_CHECK_PARAM:
            asyn_api_->HandleCheckParam(chBuf);
            break;
        case CT_OPEN_CAMERA:
            asyn_api_->HandleOpenCamera(chBuf);
            break;
        case CT_CLOSE_CAMERA:
            asyn_api_->HandleCloseCamera(chBuf);
            break;
        case CT_CAMERA_STATUS:
            asyn_api_->HandleQueryCamera(chBuf);
            break;
        case CT_SET_RESOLUTION:
            asyn_api_->HandleSetResolution(chBuf);
            break;
        case CT_SET_DPI:
            asyn_api_->HandleSetDPI(chBuf);
            break;
        case CT_SET_PROPERTY:
            asyn_api_->HandleSetProperty(chBuf);
            break;
        case CT_RECORD:
            asyn_api_->HandleRecordVideo(chBuf);
            break;
        case CT_STOP_RECORD:
            asyn_api_->HandleStopRecordVideo(chBuf);
            break;
        case CT_GET_RFID:
            asyn_api_->HandleGetRFID(chBuf);
            break;
        case CT_GET_DEV_STATUS:
            asyn_api_->HandleGetStatus(chBuf);
            break;
        case CT_GET_SEAL_COORD:
            asyn_api_->HandleCvtCoord(chBuf);
            break;
        case CT_WRITE_CVT_RATIO:
            asyn_api_->HandleWriteRatio(chBuf);
            break;
        case CT_READ_CVT_RATIO:
            asyn_api_->HandleReadRatio(chBuf);
            break;
        default:
            printf("AsynAPISet::ReceiverFunc->Unknown cmd: %d", cmd_type);
            break;
        }
        if (0 != cmd_type)
            ResetEvent(send_ev_);

        chBuf[0] = 0x0;
    }

    Log::WriteLog(LL_DEBUG, "Cnn::ReceiverFunc->thread exited, %s.",
        running_? "running_为true": "running_为false");
}

int MC::Cnn::WritePipe(BaseCmd* cmd)
{
    // 加互斥锁, 否则多线程写管道会有问题.
    boost::lock_guard<boost::mutex> lk(write_ctx_);

    if (!running_) {
        //AsynErrorNotify(cmd, MC::EC_PIPE_STOPPED);
        return MC::EC_CON_DISCONN;
    }

    if (NULL == cmd)
        return MC::EC_INVALID_PARAMETER;

    cmd->Ser();
    // 写入pipe
    LPTSTR lpvMessage = TEXT(cmd->xs_.GetBuf());
    BOOL fSuccess = WriteFileEx(
        pipe_,
        lpvMessage,
        (lstrlen(lpvMessage) + 1) * sizeof(TCHAR), // 写入内容的长度
        (LPOVERLAPPED)pipe_inst_,
        (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
    if (!fSuccess) {
        Log::WriteLog(LL_ERROR, "APISet::WritePipe->WriteFileEx failed, err: %d",
            GetLastError());
        return -1;
    }

    return 0;
}

int MC::Cnn::WriteMQ(BaseCmd* cmd)
{
    // message queue 自身支持多线程, 不需要自己再加锁
    /*    boost::lock_guard<boost::mutex> lk(write_ctx_);*/

    if (!running_)
        return MC::EC_CON_DISCONN;

    if (NULL == cmd)
        return MC::EC_INVALID_PARAMETER;

    cmd->Ser();
    LPTSTR lpvMessage = TEXT(cmd->xs_.GetBuf());
    int msg_size = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
    try {
        if (ProcessExisted(MC::SERVER_NAME)) {
            heart_mtx_.lock();
            bool succ = send_mq_->try_send(lpvMessage, msg_size, 0);
            heart_mtx_.unlock();

            // 过滤心跳消息
            if (cmd->ct_ != CT_HEART_BEAT)
                Log::WriteLog(LL_DEBUG, "Cnn::WriteMQ->Cmd: %s, 消息大小: %d, 发送结果: %s",
                    cmd_des[cmd->ct_].c_str(),
                    msg_size,
                    succ ? "成功" : "失败");
        } else {
            Log::WriteLog(LL_ERROR, "Cnn::WriteMQ->连接通道断开");
            return MC::EC_CON_DISCONN;
        }
    } catch (boost::interprocess::interprocess_exception &ex) {
        std::cout << "AsynAPISet::WriteMQ->msg size: " << msg_size << ", exception: "
            << ex.what() << std::endl;
    }

    return MC::EC_SUCC;
}

int MC::Cnn::WriteCnn(BaseCmd* cmd)
{
    SetEvent(send_ev_);

    if (MC::CT_PIPE == AgentConfig::GetInst()->conn_type_)
        return WritePipe(cmd);
    else if (MC::CT_MQ == AgentConfig::GetInst()->conn_type_)
        return WriteMQ(cmd);
    else
        return -1;
}

void MC::Cnn::SendFunc()
{
    BaseCmd* cmd = NULL;
    while (running_) {
        if (0 == send_cmd_queue_.WaitForRead(SEND_QUEUE_WAIT)) {
            send_cmd_queue_.Pop(cmd);
            WriteCnn(cmd);
            delete cmd;
        }
    }
}

// 收到服务进程发送回的心跳响应
void MC::Cnn::HandleHeartBeating(char* chBuf)
{
    SetEvent(heart_ev_);
}

void MC::Cnn::HandleServerDeath()
{
    // 服务进程停止, 需要重新建立共享内存通信连接, 锁保护发送和接收共享内存队列
    heart_mtx_.lock();
    server_dead_ = true;
    // 启动服务
    if (StartSvc(MC::SERVER_NAME)) {
        Log::WriteLog(LL_DEBUG, "MC::Cnn::HandleServerDeath->启动%s成功",
            MC::SERVER_NAME.c_str());

        Sleep(1000);
        delete recv_mq_;
        delete send_mq_;
        recv_mq_ = NULL;
        send_mq_ = NULL;

        try {
            send_mq_ = new (std::nothrow) boost::interprocess::message_queue(
                boost::interprocess::open_only,
                AgentConfig::GetInst()->send_mq_name_.c_str());

            recv_mq_ = new (std::nothrow) boost::interprocess::message_queue(
                boost::interprocess::open_only,
                AgentConfig::GetInst()->recv_mq_name_.c_str());
        }  catch (boost::interprocess::interprocess_exception &ex) {
            std::cout << "MC::Cnn::HandleServerDeath->打开消息队列失败: " 
                << ex.what() << std::endl;
            Log::WriteLog(LL_ERROR, "MC::Cnn::HandleServerDeath->打开消息队列失败: %s", 
                ex.what());
        }
        heart_mtx_.unlock();

        // 启动成功后需要发送一次心跳
//         HeartCmd heart_cmd;
//         WriteCnn(&heart_cmd);
    } else {
        Log::WriteLog(LL_ERROR, "MC::Cnn::HandleServerDeath->启动%s失败",
            MC::SERVER_NAME.c_str());
        heart_mtx_.unlock();
    }
}

void MC::Cnn::HeartBeatingFunc()
{
    while (running_) { 
        Sleep(HEART_BEATING_WAIT);
        bool is_alive = LookupProcessByName(MC::SERVER_NAME.c_str());
        if (!is_alive) {
            Log::WriteLog(LL_DEBUG, "AsynAPISet::HeartBeatingFunc->timeout");
            MC::KillProcessByName(MC::SERVER_NAME.c_str());
            HandleServerDeath();
        }
        continue;

        DWORD ret = WaitForSingleObject(heart_ev_, HEART_BEATING_WAIT);
        switch (ret) {
        case WAIT_OBJECT_0: {
            HeartCmd heart_cmd;
            WriteCnn(&heart_cmd);
        }
            break;
        case WAIT_TIMEOUT: {    // 超时未收到心跳响应
            Log::WriteLog(LL_DEBUG, "AsynAPISet::HeartBeatingFunc->timeout");
            MC::KillProcessByName(MC::SERVER_NAME.c_str());
            HandleServerDeath();
        }
            break;
        default:
            break;
        }
    }
}

bool MC::Cnn::StartSvc(const std::string& svc)
{
    std::string root_path;
    if (!MC::GetMoudulePath(root_path))
        return false;

    char buf[512] = { 0 };
    sprintf_s(buf, "%s%s exe",
        root_path.c_str(),
        svc.c_str());

    if (31 < WinExec(buf, SW_HIDE))
        return true;

    return false;

    // 启动指定服务
    SC_HANDLE hSC = ::OpenSCManager(
        NULL,
        NULL,
        GENERIC_EXECUTE);
    if (hSC == NULL)
        return false;

    SC_HANDLE hSvc = ::OpenService(
        hSC,
        svc.c_str(),
        SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);
    if (hSvc == NULL) {
        ::CloseServiceHandle(hSC);
        return false;
    }

    // 获得服务的状态  
    SERVICE_STATUS status;
    if (::QueryServiceStatus(hSvc, &status) == FALSE) {
        ::CloseServiceHandle(hSvc);
        ::CloseServiceHandle(hSC);
        return false;
    }

    if (status.dwCurrentState == SERVICE_STOPPED) {
        // 启动服务  
        if (::StartService(hSvc, NULL, NULL) == FALSE) {
            ::CloseServiceHandle(hSvc);
            ::CloseServiceHandle(hSC);
            return false;
        }

        return true;
    }
    else if (status.dwCurrentState == SERVICE_RUNNING) {
        return true;
    }
    else {
        Log::WriteLog(LL_DEBUG, "AsynAPISet::StartSvc->当前服务状态是: %d", status.dwCurrentState);
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////

bool MC::ProcessExisted(const std::string& process_name)
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cout << "CreateToolhelp32Snapshot Error!" << std::endl;;
        return false;
    }

    BOOL bResult = Process32First(hProcessSnap, &pe32);
    bool ret = false;
    while (bResult) {
        std::string name = pe32.szExeFile;      // process name
        int id = pe32.th32ProcessID;            // process id
        if (process_name == name) {
            ret = true;
            break;
        }

        bResult = Process32Next(hProcessSnap, &pe32);
    }

    CloseHandle(hProcessSnap);
    return ret;
}

VOID WINAPI CompletedWriteRoutine(
    DWORD dwErr,
    DWORD cbWritten,
    LPOVERLAPPED lpOverLap)
{
    Log::WriteLog(LL_DEBUG, "CompletedWriteRoutine->Agent写管道, Err: %d, 实际写入: %d",
        dwErr,
        cbWritten);
}
