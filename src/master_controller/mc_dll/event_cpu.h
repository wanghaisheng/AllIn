#ifndef CONTROLLER_EVENT_CPU_H_
#define CONTROLLER_EVENT_CPU_H_

#include "syn_queue.h"
#include "base_event.h"
#include <boost/thread/thread.hpp>

namespace MC {

class EventCPUCore {

public:
    static EventCPUCore* GetInstance();

public:
    bool Start();

    void Stop();

    bool PostEvent(BaseEvent* ev);

private:
    EventCPUCore();

    void WorkFunc();

private:
    volatile bool running_;
    SynQueue<BaseEvent*> event_queue_;
    boost::thread* worker_thread_;

    static EventCPUCore* g_instance;
    static const int EVENT_CPU_WAIT = 3000; // millisecond
};

} // namespace MC

#endif // CONTROLLER_EVENT_CPU_H_
