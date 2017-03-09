#ifndef MC_MQ_CNN_H_
#define MC_MQ_CNN_H_

#include <string>
#include <windows.h>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/thread/thread.hpp>
#include "agent_cmd.h"
#include "syn_queue.h"

class MQCnn {
public:
    MQCnn();
    virtual ~MQCnn();
    bool Start(
        std::string recv_name,
        std::string send_name,
        bool create_only_mode = true);
    void Stop();
    bool PushCmd(BaseCmd* cmd);
    bool SendMsg(const char* buf, int size, unsigned int priority = 0);
    
protected:
    // 在该方法中不宜进行耗时操作，更不能阻塞运行。
    virtual void OnRecvMQMsg(char* buf, int size) = 0;

private:
    void ReceiveFunc();
    void SendFunc();

private:
    volatile bool                       running_;
    boost::thread*                      recver_thread_;

    MC::SynQueue<BaseCmd*>              cmd_queue_;
    boost::thread*                      send_thread_;

    std::string                         recv_mq_name_;
    std::string                         send_mq_name_;
    boost::interprocess::message_queue* recv_mq_;
    boost::interprocess::message_queue* send_mq_;
};

#endif
