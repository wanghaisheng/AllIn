#include "stdafx.h"
#include "log.h"
#include "event_cpu.h"

MC::EventCPUCore* MC::EventCPUCore::g_instance = NULL;

MC::EventCPUCore* MC::EventCPUCore::GetInstance()
{
    if (NULL == g_instance)
        g_instance = new (std::nothrow) EventCPUCore();

    return g_instance;
}

MC::EventCPUCore::EventCPUCore() : running_(false)
{

}

bool MC::EventCPUCore::Start()
{
    if (running_)
        return true;

    running_ = true;
    worker_thread_ = 
        new (std::nothrow) boost::thread(boost::bind(&EventCPUCore::WorkFunc, this));
    if (NULL == worker_thread_)
        return false;

    Log::WriteLog(LL_DEBUG, "EventCPUCore::Start->事件处理器启动成功");
    return true;
}

void MC::EventCPUCore::Stop()
{
    running_ = false;
    worker_thread_->join();
    Log::WriteLog(LL_DEBUG, "EventCPUCore::Start->停止事件处理器成功");
    Log::WriteLog(LL_DEBUG, "EventCPUCore::Stop->停止事件处理器后事件队列大小:%d", 
        event_queue_.size());
}

bool MC::EventCPUCore::PostEvent(BaseEvent* ev)
{
    return event_queue_.Push(ev);
}

void MC::EventCPUCore::WorkFunc()
{
    BaseEvent* ev = NULL;
    while (running_) {
        if (0 == event_queue_.WaitForRead(EVENT_CPU_WAIT)) {
            event_queue_.Pop(ev);
            ev->Execute();
        }
    }
}
