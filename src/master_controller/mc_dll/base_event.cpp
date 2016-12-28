#include "stdafx.h"
#include "tool.h"
#include "log.h"
#include "base_event.h"

MC::BaseEvent::BaseEvent(std::string des) : des_(des), exception_(EC_SUCC)
{
}

MC::BaseEvent::~BaseEvent()
{

}

void MC::BaseEvent::Execute()
{
    // 异常处理
    bool conn = Tool::GetInst()->Connected();
    if (!conn) {
        Log::WriteLog(LL_DEBUG, "BaseEvent::Execute->设备未连接");
        exception_ = EC_DEV_DISCONN;
        goto NOR;
    }

    MC::ConnStatus status = Tool::GetInst()->GetStatus();
    if (status == CS_FREE || status == CS_RECON_SUC) {
        FOpenDev(NULL);
        goto NOR;
    }

    if (CS_OPEN_FAIL == status) {
        Log::WriteLog(LL_DEBUG, "BaseEvent::Execute->连接--打开设备失败");
        exception_ = EC_CONN_OPEN_FAIL;
        goto NOR;
    }

    if (CS_RECON_FAIL == status) {
        Log::WriteLog(LL_DEBUG, "BaseEvent::Execute->重连设备失败");
        exception_ = EC_RECONN_FAIL;
        goto NOR;
    }

    if (CS_STAMPER_DROP == status) {
        Log::WriteLog(LL_DEBUG, "BaseEvent::Execute->连接--印章掉落, 印控机处故障状态");
        exception_ = EC_STAMPER_DROP;
        goto NOR;
    }

NOR:
    SpecificExecute();
}
