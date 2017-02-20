#ifndef MC_STAMPING_MGR_H_
#define MC_STAMPING_MGR_H_

#include <windows.h>
#include "err_code.h"

class StampingMgr {
public:
    static StampingMgr* GetInst() {
        if (NULL == g_inst)
            g_inst = new StampingMgr;

        return g_inst;
    }

    int Wait(int seconds);

    void Signal();                        
                                                                 
    void SetStampingResult(MC::ErrorCode ec);

    MC::ErrorCode QueryStampingResult();

private:
    StampingMgr(): state_(MC::EC_SUCC) {                                                        
        stamping_ev_ = CreateEvent(
            NULL,       // 默认属性
            FALSE,      // AUTO-reset
            FALSE,      // 初始状态 non-signaled 
            NULL);      // 未命名
    }

private:
    static StampingMgr* g_inst;

    HANDLE stamping_ev_;
    MC::ErrorCode state_;
};

#endif // MC_STAMPING_MGR_H_
