#ifndef MASTER_CONTROLLER_PARSE_H_
#define MASTER_CONTROLLER_PARSE_H_

#include <windows.h>
#include "PSBC_STAMP.h"

class PSBCConfig {
public:
    static PSBCConfig* GetInst() {
        if (NULL == config_inst)
            config_inst = new PSBCConfig;

        return config_inst;
    }

    bool Parse();

private:
    PSBCConfig(): 
        img_format_(0), 
        dpi_(96),
        resolution_width_(2048), 
        resolution_height_(1536),
        brightness_(25) {

    }

private:
    static PSBCConfig* config_inst;

public:
    int wait_time_;         // 章到位等待时间

    Point check_pt0_;   // 校准点
    Point check_pt1_;
    Point check_pt2_;
    Point check_pt3_;
    Point check_pt4_;

    // 凭证摄像头图片格式, 默认jpg, 值为0
    int img_format_;

    int dpi_;

    // 凭证摄像头分辨率, 默认300W
    int resolution_width_;
    int resolution_height_;

    // 凭证摄像头补光灯亮度
    int brightness_;
};

bool GetMoudulePath(std::string& path);

#endif // MASTER_CONTROLLER_PARSE_H_
