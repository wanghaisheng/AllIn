#ifndef AGENT_CMD_H_
#define AGENT_CMD_H_

#include <string>
#include <windows.h>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include "common_definitions.h"
#include "seria.h"

#define TASK_ID_SIZE        48
#define VOUCHER_TYPE_SIZE   16
#define KEY_SIZE            64
#define MAC_SIZE            64
#define ERROR_SIZE          64
#define SEND_TIME_SIZE      37  // null terminated.
#define SN_SIZE             30
#define MAX_STAMPER_NUM     6

enum CmdType {
    CT_INIT_MACHINE = 1,    // 初始化
    CT_BIND_MAC,            // 绑定MAC
    CT_UNBIND_MAC,          // 解绑MAC
    CT_PREPARE_STAMP,       // 准备用印
    CT_PAPER_DOOR,          // 查询进纸门状态
    CT_SNAPSHOT,            // 拍照
    CT_PHOTO_SYNTHESIS,     // 照片合成
    CT_RECOGNITION,         // 版面验证码识别
    CT_ELEMENT_IDENTI,      // 要素识别
    CT_ORDINARY_STAMP,      // 普通用印
    CT_AUTO_STAMP,          // 自动用印
    CT_FINISH_STAMP,        // 结束用印
    CT_RELEASE_STAMPER,     // 释放用印机
    CT_GET_ERROR,           // 获取错误信息
    CT_HEART_BEAT,          // 心跳命令
    CT_QUERY_MACHINE,       // 获取印控机编号
    CT_SET_MACHINE,         // 设置印控机编号
    CT_CALIBRATION,         // 校准印章
    CT_QUERY_STAMPERS,      // 印章状态查询
    CT_QUERY_SAFE,          // 安全门状态查询
    CT_SAFE_CTL,            // 开关安全门
    CT_BEEP_CTL,            // 蜂鸣器控制
    CT_QUERY_SLOT,          // 卡槽数量查询
    CT_ALARM_CTL,           // 报警器控制
    CT_QUERY_MAC,           // 查询已绑定MAC地址
};

static std::string cmd_des[] = 
{
    "保留", 
    "初始化印控机", 
    "绑定MAC", 
    "解绑MAC",
    "准备用印",
    "查询进纸门状态",
    "拍照",
    "照片合成",
    "版面验证码识别",
    "要素识别",
    "普通用印",
    "自动用印",
    "结束用印",
    "释放用印机",
    "获取错误信息",
    "心跳命令",
    "获取印控机编号",
    "设置印控机编号",
    "校准印章",
    "印章状态查询",
    "安全门状态查询",
    "开关安全门",
    "蜂鸣器控制",
    "卡槽数量查询",
    "报警器控制",
    "查询已绑定MAC地址"
};

class BaseCmd {
public:
    BaseCmd() {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        std::string uuid_str = boost::lexical_cast<std::string>(uuid);
        strcpy(send_time_, uuid_str.c_str());
    }

    virtual void Ser();
    virtual void Unser();

public:
    enum        CmdType ct_;               //新增数据成员放在ct_后, 序列化时也最先序列号ct_
    char        send_time_[SEND_TIME_SIZE];
    XSpace      xs_;
};

class InitMachineCmd : public BaseCmd {
public:
    InitMachineCmd(): ret_(MC::EC_SUCC)
    {
        ct_ = CT_INIT_MACHINE;
        memset(key_, 0, KEY_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            key_[KEY_SIZE];
    MC::ErrorCode   ret_;
};

class BindMACCmd : public BaseCmd {
public:
    BindMACCmd() : ret_(MC::EC_SUCC)
    {
        ct_ = CT_BIND_MAC;
        memset(mac_, 0, MAC_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            mac_[MAC_SIZE];
    MC::ErrorCode   ret_;
};

class UnbindCmd : public BaseCmd {
public:
    UnbindCmd() : ret_(MC::EC_SUCC)
    {
        ct_ = CT_UNBIND_MAC;
        memset(mac_, 0, MAC_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            mac_[MAC_SIZE];
    MC::ErrorCode   ret_;
};

class PrepareStampCmd : public BaseCmd {
public:
    PrepareStampCmd() : ret_(MC::EC_SUCC), stamper_id_(-1), timeout_(0) {
        ct_ = CT_PREPARE_STAMP;
        memset(task_id_, 0x0, TASK_ID_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            stamper_id_;                //印章卡槽编号(1-6)
    int             timeout_;                   //超时时间, 单位: 秒
    char            task_id_[TASK_ID_SIZE];     //用印任务ID
    MC::ErrorCode ret_;
};

class ViewPaperCmd : public BaseCmd {
public:
    ViewPaperCmd() : ret_(MC::EC_SUCC), status_(-1) {
        ct_ = CT_PAPER_DOOR;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             status_;            //进纸门状态, 0-开, 1-关, -1--未获取到进纸门状态
    MC::ErrorCode   ret_;
};

class SnapshotCmd : public BaseCmd {
public:
    SnapshotCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_SNAPSHOT;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             original_dpi_;              // 拍照的分辨率
    int             cut_dpi_;

    char            original_path_[MAX_PATH];  // 拍照原始图像路径
    char            cut_path_[MAX_PATH];       // 剪切纠偏后图像路径

    MC::ErrorCode   ret_;
};

class SynthesizePhotoCmd : public BaseCmd {
public:
    SynthesizePhotoCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_PHOTO_SYNTHESIS;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            photo1_[MAX_PATH];
    char            photo2_[MAX_PATH];
    char            merged_[MAX_PATH];

    MC::ErrorCode   ret_;
};

class RecognitionCmd : public BaseCmd {
public:
    RecognitionCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RECOGNITION;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            path_[MAX_PATH];       // 待识别图片路径

    char            template_id_[4];       // 模板ID
    char            trace_num_[20];        // 识别出的验证码

    MC::ErrorCode   ret_;
};

class IdentifyElementCmd : public BaseCmd {
public:
    IdentifyElementCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_ELEMENT_IDENTI;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            path_[MAX_PATH];        // 图片路径
    int             x_;                     // 识别区域坐标x(像素)
    int             y_;                     // 识别区域坐标y(像素)
    int             width_;                 // 识别区域宽度(像素)
    int             height_;                // 识别区域高度(像素)
    int             angle_;                 // 旋转角度(0, 90, 180, 270)

    char            content_str_[64];       // 识别结果

    MC::ErrorCode   ret_;
};

// 普通用印
class OridinaryStampCmd : public BaseCmd {
public:
    OridinaryStampCmd() : ret_(MC::EC_SUCC), stamper_num_(-1),
        x_(-1), y_(-1), angle_(0) {
        ct_ = CT_ORDINARY_STAMP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            task_id_[TASK_ID_SIZE];     // 任务ID
    char            type_[VOUCHER_TYPE_SIZE];   // 凭证类型
    int             stamper_num_;               // 印章卡槽号
    int             ink_;                       // 是否蘸印油, 0-否, 1-是
    int             x_;                         // 印章位置X(像素)
    int             y_;                         // 印章位置Y(像素)
    int             angle_;                     // 章旋转角度(顺时针: 0, 90, 180, 270)

    MC::ErrorCode   ret_;
};

class AutoStampCmd : public BaseCmd {
public:
    AutoStampCmd() : ret_(MC::EC_SUCC), stamper_num_(-1) {
        ct_ = CT_AUTO_STAMP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            task_id_[TASK_ID_SIZE];    // 任务ID
    char            type_[VOUCHER_TYPE_SIZE];  // 凭证类型
    int             stamper_num_;              // 印章卡槽号

    MC::ErrorCode   ret_;
};

class FinishStampCmd : public BaseCmd {
public:
    FinishStampCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_FINISH_STAMP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            task_id_[TASK_ID_SIZE];    // 任务ID
    MC::ErrorCode   ret_;
};

class ReleaseStamperCmd : public BaseCmd {
public:
    ReleaseStamperCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RELEASE_STAMPER;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            stamp_id_[KEY_SIZE];       // 用印机唯一编号
    MC::ErrorCode   ret_;
};

class GetErrorCmd : public BaseCmd {
public:
    GetErrorCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_GET_ERROR;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             err_;
    char            err_msg_[ERROR_SIZE];
    char            err_resolver_[ERROR_SIZE];

    MC::ErrorCode   ret_;
};

class HeartCmd : public BaseCmd {
public:
    HeartCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_HEART_BEAT;
    }

    HeartCmd(char* buf) {
        memcpy(xs_.buf_, buf, CMD_BUF_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

class QueryMachineCmd : public BaseCmd {
public:
    QueryMachineCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_QUERY_MACHINE;
        memset(sn_, 0x0, SN_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            sn_[SN_SIZE];
    MC::ErrorCode   ret_;
};

class SetMachineCmd : public BaseCmd {
public:
    SetMachineCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_SET_MACHINE;
//        memset(sn_, 0x0, SN_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            sn_[SN_SIZE];
    MC::ErrorCode   ret_;
};

class CalibrateCmd : public BaseCmd {
public:
    CalibrateCmd() : ret_(MC::EC_SUCC), slot_(-1) {
        ct_ = CT_CALIBRATION;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             slot_;      // 印章号
    MC::ErrorCode   ret_;
};

class QueryStampersCmd : public BaseCmd {
public:
    QueryStampersCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_QUERY_STAMPERS;
        memset(stamper_status_, 0x0, sizeof(int) * MAX_STAMPER_NUM);
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             stamper_status_[MAX_STAMPER_NUM];
    MC::ErrorCode   ret_;
};

class QuerySafeCmd : public BaseCmd {
public:
    QuerySafeCmd() : ret_(MC::EC_SUCC), status_(-1) {
        ct_ = CT_QUERY_SAFE;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             status_;      // 0-关, 1-开
    MC::ErrorCode   ret_;
};

class SafeCtrlCmd : public BaseCmd {
public:
    SafeCtrlCmd() : ret_(MC::EC_SUCC), ctrl_(-1), timeout_(0) {
        ct_ = CT_SAFE_CTL;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             ctrl_;      // 0-关, 1-开
    int             timeout_;   // 超时未关闭时间, 单位: 秒
    MC::ErrorCode   ret_;
};

class BeepCtrlCmd : public BaseCmd {
public:
    BeepCtrlCmd(): ret_(MC::EC_SUCC), ctrl_(-1) {
        ct_ = CT_BEEP_CTL;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             ctrl_;      // 0-关, 1-开
    MC::ErrorCode   ret_;
};

class QuerySlotCmd : public BaseCmd {
public:
    QuerySlotCmd() : ret_(MC::EC_SUCC), num_(0) {
        ct_ = CT_QUERY_SLOT;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             num_;
    MC::ErrorCode   ret_;
};

class AlarmCtrlCmd : public BaseCmd {
public:
    AlarmCtrlCmd() : ret_(MC::EC_SUCC), alarm_(-1), ctrl_(-1) {
        ct_ = CT_ALARM_CTL;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             alarm_; // 报警器类型, 0-开门报警, 1-振动报警
    int             ctrl_;  // 控制开关, 0-关, 1-开
    MC::ErrorCode   ret_;
};

class QueryMACCmd : public BaseCmd {
public:
    QueryMACCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_QUERY_MAC;
        memset(mac1_, 0, MAC_SIZE);
        memset(mac2_, 0, MAC_SIZE);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char mac1_[MAC_SIZE];
    char mac2_[MAC_SIZE];

    MC::ErrorCode ret_;
};

#endif // AGENT_CMD_H_
