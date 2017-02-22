#include <boost/exception/all.hpp>
#include "log.h"
#include "common_definitions.h"
#include "mq_cnn.h"

MQCnn::MQCnn() : running_(false)
{

}

MQCnn::~MQCnn()
{
    Stop();
}

bool MQCnn::Start(std::string recv_name, std::string send_name, bool create_only_mode)
{
    if (running_)
        return true;

    recv_mq_name_ = recv_name;
    send_mq_name_ = send_name;
    try {
        boost::interprocess::message_queue::remove(recv_mq_name_.c_str());
        boost::interprocess::message_queue::remove(send_mq_name_.c_str());

        if (create_only_mode) {
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
        } else {
            recv_mq_ = new (std::nothrow) boost::interprocess::message_queue(
                boost::interprocess::open_only,
                recv_mq_name_.c_str());

            send_mq_ = new boost::interprocess::message_queue(
                boost::interprocess::open_only,
                send_mq_name_.c_str());
        }
    } catch (boost::interprocess::interprocess_exception &ex) {
        std::cout << ex.what() << std::endl;
        Log::WriteLog(LL_ERROR, "MQCnn::Start->消息队列创建失败: %s", ex.what());
        return false;
    }

    running_ = true;
    recver_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&MQCnn::ReceiveFunc, this));
    return true;
}

void MQCnn::Stop()
{
    if (!running_)
        return;

    running_ = false;
    recver_thread_->join();

    boost::interprocess::message_queue::remove(recv_mq_name_.c_str());
    boost::interprocess::message_queue::remove(send_mq_name_.c_str());
    delete recv_mq_;
    delete send_mq_;
}

bool MQCnn::SendMsg(const char* buf, int size)
{
    try {
        LPTSTR lpvMessage = TEXT((char*)buf);
        send_mq_->send(lpvMessage, (lstrlen(lpvMessage) + 1) * sizeof(TCHAR), 0);
        Log::WriteLog(LL_DEBUG, "MQCnn::SendMsg->消息队列发送%d大小的消息", size);
        return true;
    } catch (boost::interprocess::interprocess_exception &ex) {
        Log::WriteLog(LL_ERROR, "MQCnn::SendMsg->消息队列方式写消息失败, er: %s",
            ex.what());
        return false;
    }
}

void MQCnn::ReceiveFunc()
{
    char buf[CMD_BUF_SIZE] = { 0 };
    while (running_) {
        unsigned int priority;
        boost::interprocess::message_queue::size_type recvd_size;
        try {
            // receiver is blocked if message queue is empty.
            recv_mq_->receive(buf, sizeof(buf), recvd_size, priority);
            OnRecvMQMsg(buf, recvd_size);
        } catch (boost::interprocess::interprocess_exception &ex) {
            std::cout << "MQCnn::ReceiveFunc->消息队列收消息失败, er: "
                << ex.what() << std::endl;
            Log::WriteLog(LL_ERROR, "MQCnn::ReceiveFunc->消息队列收消息失败, %s",
                ex.what());
            continue;
        }
    }
}
