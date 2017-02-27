#ifndef CONTROLLER_SYN_QUEUE_H_
#define CONTROLLER_SYN_QUEUE_H_

#include <list>
#include <windows.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace MC { // MC for Master Controller

//同步线程安全队列
template <typename T>
class SynQueue {

public:
    SynQueue() : vista_better_(false) {

    }

    ~SynQueue() {
        queue_list_.clear();
    }

    // 阻塞等待有数据读出.
    // 返回值:
    //      0 --- 有数据
    //      1 --- 无数据
    //      -1 --- 超时
    int WaitForRead(unsigned long milliseconds) {
        boost::unique_lock<boost::mutex> lk(mutex_);

        if (WAIT_TIMEOUT == WaitForSingleObject(cv_, milliseconds))
            return -1;

        if (queue_list_.empty())
            return 1;

        return 0;
    }

    bool Push(const T& val) {
        boost::lock_guard<boost::mutex> lk(mutex_);
        queue_list_.push_back(val);
        SetEvent(cv_);
        return true;
    }

    bool Pop(T& val) {
        boost::lock_guard<boost::mutex> lk(mutex_);
        if (queue_list_.empty())
            return false;

        val = queue_list_.front();
        queue_list_.pop_front();
        return true;
    }

    unsigned int size() {
        boost::lock_guard<boost::mutex> lk(mutex_);
        return queue_list_.size();
    }

private:
    std::list<T> queue_list_;
    boost::mutex mutex_;

    bool vista_better_;
    HANDLE cv_;
};

} //namespace MC

#endif //CONTROLLER_SYN_QUEUE_H_
