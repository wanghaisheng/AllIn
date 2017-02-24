#ifndef MC_AGENT_PARSE_H_
#define MC_AGENT_PARSE_H_

#include <string>
#include "common_definitions.h"

namespace MC {

class AgentConfig {
public:
    static AgentConfig* GetInst() {
        if (NULL == config_inst)
            config_inst = new (std::nothrow) AgentConfig;

        return config_inst;
    }

    bool Parse();

private:
    AgentConfig() : conn_type_(CT_MAX) {

    }

private:
    static AgentConfig* config_inst;

public:
    MC::ConnType    conn_type_;  // 是否是管道连接通信

    std::string     pipe_name_;
    std::string     send_mq_name_;
    std::string     recv_mq_name_;
}; // class AgentConfig

} // namespace MC

#endif // MC_AGENT_PARSE_H_
