#ifndef AGENT_LISTEN_H_
#define AGENT_LISTEN_H_

#include <stdio.h>
#include <windows.h>
#include <vector>
#include <boost/thread/thread.hpp>

class Listen {
public:
    ~Listen() {
        running_ = false;
        work_thread_->join();

        if (map_file_) {
            if (map_view_)
            {
                UnmapViewOfFile(map_view_);
                map_view_ = NULL;
            }

            CloseHandle(map_file_);
            map_file_ = NULL;
        }
    }

    static Listen* GetInst() {
        if (NULL == listen_inst_)
            listen_inst_ = new Listen;

        return listen_inst_;
    }

    bool Start();

    void RegisterConnCallback(PVOID func) {
        connect_callback_vec_.push_back(func);
    }

    void RegisterMsgCallback(PVOID func) {
        msg_callback_vec_.push_back(func);
    }

private:
    Listen():map_file_(NULL), map_view_(NULL) {
        running_ = false;
        syn_ev_ = OpenEvent(EVENT_ALL_ACCESS, FALSE, "Local\\MySynEvent");
    }

    void ListenFunc();

    int GetNotifyValue();

private:
    static Listen* listen_inst_;

    HANDLE syn_ev_;

    HANDLE map_file_;
    PVOID map_view_;

    bool running_;
    boost::thread*          work_thread_;

    std::vector<PVOID> connect_callback_vec_;
    std::vector<PVOID> msg_callback_vec_;
};

#endif
