#ifndef API_DEMO_PARSE_H_
#define API_DEMO_PARSE_H_

#include <string>

class ClientConfig {
public:
    static ClientConfig* GetInst() {
        if (NULL == config_inst)
            config_inst = new ClientConfig;

        return config_inst;
    }

    bool Parse();

private:
    ClientConfig() {

    }

private:
    static ClientConfig* config_inst;

public:
    std::string     ori_path_;
    std::string     cut_path_;
};

#endif // API_DEMO_PARSE_H_
