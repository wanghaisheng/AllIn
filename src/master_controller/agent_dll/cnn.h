#ifndef MC_AGENT_CNN_H_
#define MC_AGENT_CNN_H_

#include <windows.h>
#include <boost/thread/thread.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "api_set.h"
#include "agent_cmd.h"
#include "syn_queue.h"

#define SEND_QUEUE_WAIT     5000
#define HEART_BEATING_WAIT  3000

typedef struct {
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
    TCHAR chRequest[CMD_BUF_SIZE];
    DWORD cbRead;
    TCHAR chReply[CMD_BUF_SIZE];
    DWORD cbToWrite;
} PIPEINST, *LPPIPEINST;

VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);

namespace MC {

class Cnn {

public:
    static Cnn* GetInst() {
        if (NULL == cnn_inst)
            cnn_inst = new Cnn;

        return cnn_inst;
    }

    void SetAgent(AsynAPISet* api) {
        asyn_api_ = api;
    }

    bool Start();

    void Stop();

    bool PushCmd(BaseCmd* cmd);

private:
    static Cnn* cnn_inst;

    Cnn(): 
        running_(false), 
        asyn_api_(NULL), 
        pipe_inst_(NULL),
        send_mq_(NULL),
        recv_mq_(NULL), 
        pipe_(NULL),
        server_dead_(false) {

    }

private:
    bool StartPipe(const char* pipe_name);

    bool StartMQ(
        const std::string& send_mq_name,
        const std::string& recv_mq_name);

    void SendFunc();

    bool RecvBuf(TCHAR* buf, int buf_len, DWORD* actual_read);

    void ReceiveFunc();

    char GetCmdHeader(TCHAR * chBuf)
    {
        // 解析消息头, 判断具体消息类型
        char cmd_type;
        memcpy(&cmd_type, chBuf, sizeof(char));
        return cmd_type;
    }

    void HandleServerDeath();

    void HeartBeatingFunc();
   
    int WritePipe(BaseCmd* cmd);

    int WriteMQ(BaseCmd* cmd);

    int WriteCnn(BaseCmd* cmd);

    bool StartSvc(const std::string& svc);

    void HandleHeartBeating(char* chBuf);

private:
    AsynAPISet*             asyn_api_;

    HANDLE                  send_ev_;   // 有消息发送置为有信号
    bool                    running_;
    boost::thread*          recver_thread_;
    boost::thread*          sender_thread_;
    MC::SynQueue<BaseCmd*>  send_cmd_queue_;

    boost::mutex            write_ctx_;;

    bool                    server_dead_;
    HANDLE                  heart_ev_;
    boost::mutex            heart_mtx_;
    boost::thread*          heart_thread_;   // 心跳线程, 监听服务进程"mc_exe.exe"是否存在

    LPPIPEINST                          pipe_inst_;
    HANDLE                              pipe_;

    boost::interprocess::message_queue* send_mq_;
    boost::interprocess::message_queue* recv_mq_;
};

// 遍历系统进程列表, 查找是否有名为@process_name的进程
bool ProcessExisted(const std::string& process_name);

} // namespace MC

#endif // MC_AGENT_CNN_H_
