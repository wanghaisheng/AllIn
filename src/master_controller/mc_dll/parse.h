#ifndef MASTER_CONTROLLER_PARSE_H_
#define MASTER_CONTROLLER_PARSE_H_

#include <windows.h>

namespace MC {

struct Point {
    Point() : x(0), y(0) {}

    Point(int _x, int _y) : x(_x), y(_y) {

    }

    int x;
    int y;
};

class SvrConfig {
public:
    static SvrConfig* GetInst() {
        if (NULL == config_inst)
            config_inst = new SvrConfig;

        return config_inst;
    }

    bool Parse();

private:
    SvrConfig(): 
        img_format_(0), 
        resolution_width_(2048), 
        resolution_height_(1536),
        brightness_(25) {

    }

private:
    static SvrConfig* config_inst;

public:
    int wait_time_;         // 章到位等待时间

    MC::Point check_pt0_;   // 校准点
    MC::Point check_pt1_;
    MC::Point check_pt2_;
    MC::Point check_pt3_;
    MC::Point check_pt4_;

    // 凭证摄像头图片格式, 默认jpg, 值为0
    int img_format_;

    // 凭证摄像头分辨率, 默认300W
    int resolution_width_;
    int resolution_height_;

    // 凭证摄像头补光灯亮度
    int brightness_;
};

}

#endif // MASTER_CONTROLLER_PARSE_H_
