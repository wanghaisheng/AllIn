#ifndef MC_AGENT_PARSE_H_
#define MC_AGENT_PARSE_H_

#include <windows.h>
#include "common_definitions.h"

namespace MC {

class Config {
public:
    static Config* GetInst() {
        if (NULL == config_inst)
            config_inst = new Config;

        return config_inst;
    }

    bool Parse();

private:
    Config() : conn_type_(CT_MAX) {

    }

private:
    static Config* config_inst;

public:
    MC::ConnType    conn_type_;  // 是否是管道连接通信

    std::string     pipe_name_;
    std::string     send_mq_name_;
    std::string     recv_mq_name_;
};
}

#endif // MC_AGENT_PARSE_H_
