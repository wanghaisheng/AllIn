#ifndef API_DEMO_PARSE_H_
#define API_DEMO_PARSE_H_

#include <windows.h>

class Config {
public:
    static Config* GetInst() {
        if (NULL == config_inst)
            config_inst = new Config;

        return config_inst;
    }

    bool Parse();

private:
    Config() {

    }

private:
    static Config* config_inst;

public:
    std::string     ori_path_;
    std::string     cut_path_;
};

#endif // API_DEMO_PARSE_H_
