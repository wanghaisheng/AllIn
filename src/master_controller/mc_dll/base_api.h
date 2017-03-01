#ifndef CONTROLLER_BASE_API_H_
#define CONTROLLER_BASE_API_H_

#include <string>
#include "log.h"
#include "tool.h"
#include "parse.h"
#include "USBControlF60.h"

namespace MC {

class BaseAPI {
public:
    BaseAPI(std::string des) : des_(des) {
        int ret = FOpenDev(NULL);
        MC::DeviceStat stat;
        stat.conn_ = ret == 0;
        stat.status_ = 0 == ret ? MC::CS_FREE : MC::CS_OPEN_FAIL;
        stat.cause_ = "初始化打开设备";
        MC::Tool::GetInst()->SetStat(stat);
    }

    virtual ~BaseAPI() {
        int ret = FCloseDev();
        MC::DeviceStat stat;
        stat.conn_ = !(ret == 0);
        stat.status_ = 0 == ret ? MC::CS_CLOSE_SUC : MC::CS_CLOSE_FAIL;
        stat.cause_ = "正常断开设备";
        MC::Tool::GetInst()->SetStat(stat);
    }

private:
    std::string des_;
};

} // namespace MC

#endif // CONTROLLER_BASE_API_H_
