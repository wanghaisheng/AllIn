#ifndef CONTROLLER_RECVER_H_
#define CONTROLLER_RECVER_H_

#define WIN32_LEAN_AND_MEAN

#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <windows.h>
#include "syn_queue.h"
#include "pipe_server.h"
#include "seria.h"

struct RecvMsg {
    LPPIPEINST  pipe_inst;
    char        msg[CMD_BUF_SIZE];
};

class Recver {
public:
    ~Recver();

    bool Start();

    MC::ConnType CnnType() const;

    void Insert(const RecvMsg* msg);

    void Signal();

    void Stop();

    bool WriteResp(LPPIPEINST pipe_inst_, char* buf);

private:
    bool ParseConfig();

    void ReceiveFunc();

private:
    void HandleQueryMachine(const RecvMsg* msg);

    void HandleSetMachine(const RecvMsg* msg);

    void HandleInitMachine(const RecvMsg* msg);

    void HandleBindMAC(const RecvMsg* msg);

    void HandleUnbindMAC(const RecvMsg* msg);

    void HandlePrepareStamp(const RecvMsg* msg);

    void HandleQueryPaper(const RecvMsg* msg);

    void HandleSnapshot(const RecvMsg* msg);

    void HandleMergePhoto(const RecvMsg* msg);

    void HandleRecognition(const RecvMsg* msg);

    void HandleElementIdenti(const RecvMsg* msg);

    void HandleOrdinary(const RecvMsg* msg);

    void HandleAuto(const RecvMsg* msg);

    void HandleFinish(const RecvMsg* msg);

    void HandleReleaseStamper(const RecvMsg* msg);

    void HandleGetError(const RecvMsg* msg);

    void HandleCalibrate(const RecvMsg* msg);

    void HandleQueryStampers(const RecvMsg* msg);

    void HandleQuerySafe(const RecvMsg* msg);

    void HandleSafeControl(const RecvMsg* msg);

    void HandleBeepControl(const RecvMsg* msg);

    void HandleQuerySlot(const RecvMsg* msg);

    void HandleAlarmControl(const RecvMsg* msg);

    void HandleQueryMAC(const RecvMsg* msg);

    void HandleHeart(const RecvMsg* msg);

private:
    boost::property_tree::ptree         svr_config_pt_;

    MC::SynQueue<const RecvMsg* >       recver_queue_;
    HANDLE                              recv_msg_ev_;
    volatile bool                       running_;
    boost::thread*                      recver_thread_;

    boost::mutex                        write_resp_mtx_;

    MC::ConnType                        cnn_type_;
    std::string                         recv_mq_name_;
    std::string                         send_mq_name_;
    boost::interprocess::message_queue* recv_mq_;
    boost::interprocess::message_queue* send_mq_;
};

#endif // CONTROLLER_RECVER_H_
