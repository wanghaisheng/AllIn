#include "stamping_mgr.h"

StampingMgr* StampingMgr::g_inst = NULL;

int StampingMgr::Wait(int seconds)
{
    DWORD ret = WaitForSingleObject(stamping_ev_, seconds * 1000);
    switch (ret) {
    case WAIT_OBJECT_0:
        return 0;
    case WAIT_TIMEOUT:
        return -1;
    default:
        return -1;
    }
}

void StampingMgr::Signal()
{
    SetEvent(stamping_ev_);
}

void StampingMgr::SetStampingResult(MC::ErrorCode ec)
{
    state_ = ec;
}

MC::ErrorCode StampingMgr::QueryStampingResult()
{
    return state_;
}
