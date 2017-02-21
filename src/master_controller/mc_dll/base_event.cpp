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
    int ret = FOpenDev(NULL);
    unsigned char dev_status[24] = { 0 };
    ret = ::FGetDevStatus(dev_status, sizeof(dev_status));
    if (-2 == ret) {
        Log::WriteLog(LL_ERROR, "BaseEvent::Execute->查询设备状态失败, er: %d", ret);
        exception_ = MC::EC_DEV_DISCONN;
        goto NOR;
    }

    if (0 != ret) {
        Log::WriteLog(LL_ERROR, "BaseEvent::Execute->查询设备状态失败, er: %d", ret);
        exception_ = MC::EC_QUERY_DEV_STATUS_FAIL;
        goto NOR;
    }

    // 设备状态机描述
    // char* status_des[] = {
    //     "未初始化",
    //     "启动自检",
    //     "检测章",
    //     "空闲状态",
    //     "测试模式",
    //     "故障模式",
    //     "盖章模式",
    //     "维护模式"
    // };
    switch (dev_status[1]) {
    case 0:
        exception_ = MC::EC_UN_INITIATION;
        break;
    case 1:
        exception_ = MC::EC_SELF_EXAMIN;
        break;
    case 2:
        exception_ = MC::EC_EXAMIN_STAMPER;
        break;
    case 3:
        exception_ = MC::EC_SUCC;
        break;
    case 4:
        exception_ = MC::EC_IN_TEST;
        break;
    case 5:
        exception_ = MC::EC_IN_BREAK_DOWN;
        break;
    case 6:
        exception_ = MC::EC_IN_STAMPING;
        break;
    case 7:
        exception_ = MC::EC_IN_MAINTAIN;
        break;
    default:
        exception_ = MC::EC_FAIL;
        break;
    }

    if (MC::EC_SUCC != exception_)
        goto NOR;

    // 异常处理
    bool conn = Tool::GetInst()->Connected();
    if (!conn && 0 != ret) {
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
