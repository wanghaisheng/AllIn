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
    CT_PHOTO_SYNTHESIS,     // 照片合成
    CT_RECOGNITION,         // 版面验证码识别
    CT_SEARCH_STAMP,        // 查找用印点
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
    CT_ALARM_CTL,           // 开关报警器(门报警、振动报警)
    CT_QUERY_ALARM,         // 获取报警器状态
    CT_QUERY_MAC,           // 查询已绑定MAC地址
    CT_LOCK,                // 锁定印控仪
    CT_UNLOCK,              // 解锁印控仪
    CT_LOCK_STATUS,         // 锁定状态
    CT_OPEN_CON,            // 打开连接
    CT_CLOSE_CON,           // 关闭连接
    CT_QUERY_CON,           // 连接状态
    CT_SIDE_DOOR_ALARM,     // 安全门报警器设置
    CT_QUERY_DEV_MODEL,     // 获取设备型号
    CT_OPEN_PAPER,          // 开进纸门
    CT_LED_CTL,             // 补光灯控制
    CT_CHECK_PARAM,         // 用印参数合法性检查
    CT_GET_RFID,            // 根据卡槽号获取RFID
    CT_RECOG_MODEL,         // 模板及用印点查找
    CT_GET_DEV_STATUS,      // 获取设备状态
    CT_GET_SEAL_COORD,      // 原图转设备坐标
    CT_WRITE_CVT_RATIO,     // 写图像转换倍率
    CT_READ_CVT_RATIO,      // 读图像转换倍率
    CT_WRITE_CALIBRATION,   // 写较准点
    CT_READ_CALIBRATION,    // 读较准点
    CT_QUERY_TOP,           // 顶盖门状态查询
    CT_EXIT_MAIN,           // 退出维护模式
    CT_FACTORY_CTRL,        // 工厂模式控制
    CT_RESET,               // 复位
    CT_RESTART,             // 重启主板

    // 摄像头接口
    CT_START_PREVIEW,       // 开始预览
    CT_STOP_PREVIEW,        // 停止预览
    CT_OPEN_CAMERA,         // 打开摄像头
    CT_CLOSE_CAMERA,        // 关闭摄像头
    CT_CAMERA_STATUS,       // 摄像头状态
    CT_SET_RESOLUTION,      // 设置分辨率
    CT_SET_DPI,             // 设置DPI值
    CT_SET_PROPERTY,        // 设置属性
    CT_SNAPSHOT,            // 拍照
    CT_RECORD,              // 录像
    CT_STOP_RECORD,         // 停止录像
};

static std::string cmd_des[] = 
{
    "保留", 
    "初始化印控机", 
    "绑定MAC", 
    "解绑MAC",
    "准备用印",
    "查询进纸门状态",
    "照片合成",
    "版面验证码识别",
    "查找用印点",
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
    "获取报警器状态",
    "查询已绑定MAC地址",
    "锁定印控仪",
    "解锁印控仪",
    "锁定状态",
    "打开连接",
    "关闭连接",
    "连接状态",
    "安全门报警器设置",
    "获取设备型号",
    "开进纸门",
    "补光灯控制",
    "用印参数合法性检查",
    "根据卡槽号获取RFID",
    "模板及用印点查找",
    "获取设备状态",
    "写图像转换倍率",
    "读图像转换倍率",
    "写较准点",
    "读较准点",
    "顶盖门状态查询",
    "退出维护模式",
    "工厂模式控制",
    "复位",
    "重启主板",

    "打开摄像头",
    "关闭摄像头",
    "摄像头状态",
    "设置分辨率",
    "设置DPI值",
    "设置属性",
    "拍照",
    "开始录像",
    "停止录像"
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
    int             which_;
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

class SearchStampPointCmd: public BaseCmd {
public:
    SearchStampPointCmd(): ret_(MC::EC_SUCC) {
        in_x_ = 0;
        in_y_ = 0;
        in_angle_ = 0.0;
        ct_ = CT_SEARCH_STAMP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char    src_[MAX_PATH];
    int     in_x_;
    int     in_y_;
    double  in_angle_;
    int     out_x_;
    int     out_y_;
    double  out_angle_;

    MC::ErrorCode   ret_;
};

class RecognitionCmd : public BaseCmd {
public:
    RecognitionCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RECOGNITION;
        memset(template_id_, 0x0, sizeof(template_id_));
        memset(trace_num_, 0x0, sizeof(trace_num_));
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            path_[MAX_PATH];       // 待识别图片路径

    char            template_id_[32];       // 模板ID
    char            trace_num_[32];        // 识别出的验证码

    MC::ErrorCode   ret_;
};

class IdentifyElementCmd : public BaseCmd {
public:
    IdentifyElementCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_ELEMENT_IDENTI;
        memset(content_str_, 0x0, sizeof(content_str_));
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

class RecoModelTypeEtcCmd: public BaseCmd {
public:
    RecoModelTypeEtcCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RECOG_MODEL;
        memset(model_result_, 0x0, sizeof(model_result_));
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            path_[MAX_PATH];

    char            model_result_[128];
    double          angle_;
    int             x_;
    int             y_;

    MC::ErrorCode   ret_;
};

// 普通用印
class OridinaryStampCmd : public BaseCmd {
public:
    OridinaryStampCmd() : ret_(MC::EC_SUCC), stamper_num_(-1),
        x_(-1), y_(-1), angle_(0), seal_type_(0) {
        ct_ = CT_ORDINARY_STAMP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            task_id_[TASK_ID_SIZE];     // 任务ID
    char            type_[VOUCHER_TYPE_SIZE];   // 凭证类型
    int             stamper_num_;               // 印章卡槽号
    int             ink_;                       // 是否蘸印油, 0-否, 1-是
    int             x_;                         // 印章位置X(物理坐标)
    int             y_;                         // 印章位置Y(物理坐标)
    int             angle_;                     // 章旋转角度(顺时针: 0, 90, 180, 270)
    int             seal_type_;                 // 0 - 普通用印, 1 - 骑缝章

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
        memset(stamper_status_, 0x0, sizeof(char) * MAX_STAMPER_NUM + 1);
    }

    virtual void Ser();
    virtual void Unser();

public:
    char            stamper_status_[MAX_STAMPER_NUM + 1];
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
    int             type_;
    int             interval_;
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

class QueryAlarmCmd: public BaseCmd {
public:
    QueryAlarmCmd(): ret_(MC::EC_SUCC), door_(-1), vibration_(-1) {
        ct_ = CT_QUERY_ALARM;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             door_;
    int             vibration_;
    
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

// lock machine
class LockCmd: public BaseCmd {
public:
    LockCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_LOCK;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

// unlock machine
class UnlockCmd: public BaseCmd {
public:
    UnlockCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_UNLOCK;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

// query lock status
class QueryLockCmd: public BaseCmd {
public:
    QueryLockCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_LOCK_STATUS;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int status_;            // 0 -- unlocking, 1 -- locking

    MC::ErrorCode ret_;
};

class OpenCnnCmd: public BaseCmd {
public:
    OpenCnnCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_OPEN_CON;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

class CloseCnnCmd: public BaseCmd {
public:
    CloseCnnCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_CLOSE_CON;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

class QueryCnnCmd: public BaseCmd {
public:
    QueryCnnCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_QUERY_CON;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int status_;            // 0-disconnected, 1-connected

    MC::ErrorCode ret_;
};

class SideDoorAlarmCmd: public BaseCmd {
public:
    SideDoorAlarmCmd(): ret_(MC::EC_SUCC), keep_open_(0), timeout_(0) {
        ct_ = CT_SIDE_DOOR_ALARM;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int keep_open_;
    int timeout_;

    MC::ErrorCode ret_;
};

class GetDevModelCmd: public BaseCmd {
public:
    GetDevModelCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_QUERY_DEV_MODEL;
        memset(model_, 0, sizeof(model_));
    }

    virtual void Ser();
    virtual void Unser();

public:
    char model_[128];

    MC::ErrorCode ret_;
};

class OpenPaperCmd: public BaseCmd {
public:
    OpenPaperCmd(): ret_(MC::EC_SUCC), timeout_(0) {
        ct_ = CT_OPEN_PAPER;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int timeout_;
    MC::ErrorCode ret_;
};

class CtrlLEDCmd: public BaseCmd {
public:
    CtrlLEDCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_LED_CTL;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;     // control which led
    int switch_;    // operation code, 0--close, 1--open
    int value_;     // only when operation code is '1', this value is valid

    MC::ErrorCode ret_;
};

class CheckParamCmd: public BaseCmd {
public:
    CheckParamCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_CHECK_PARAM;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int x_;
    int y_;
    int angle_;

    MC::ErrorCode ret_;
};

class OpenCameraCmd: public BaseCmd {
public:
    OpenCameraCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_OPEN_CAMERA;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;

    MC::ErrorCode ret_;
};

class CloseCameraCmd: public BaseCmd {
public:
    CloseCameraCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_CLOSE_CAMERA;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;

    MC::ErrorCode ret_;
};

class QueryCameraStatusCmd: public BaseCmd {
public:
    QueryCameraStatusCmd() : ret_(MC::EC_SUCC), which_(-1), status_(-1) {
        ct_ = CT_CAMERA_STATUS;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    int status_;

    MC::ErrorCode ret_;
};

class SetResolutionCmd: public BaseCmd {
public:
    SetResolutionCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_SET_RESOLUTION;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    int x_;
    int y_;

    MC::ErrorCode ret_;
};

class SetDPICmd: public BaseCmd {
public:
    SetDPICmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_SET_DPI;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    int dpi_x_;
    int dpi_y_;

    MC::ErrorCode ret_;
};

class SetPropertyCmd: public BaseCmd {
public:
    SetPropertyCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_SET_PROPERTY;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    int hue_;
    int saturation_;
    int value_;

    MC::ErrorCode ret_;
};

class RecordVideoCmd: public BaseCmd {
public:
    RecordVideoCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_RECORD;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    char path_[MAX_PATH];

    MC::ErrorCode ret_;
};

class StopRecordVideoCmd: public BaseCmd {
public:
    StopRecordVideoCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_STOP_RECORD;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    char path_[MAX_PATH];

    MC::ErrorCode ret_;
};

class GetRFIDCmd: public BaseCmd {
public:
    GetRFIDCmd(): ret_(MC::EC_SUCC), slot_(0), rfid_(0) {
        ct_ = CT_GET_RFID;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int slot_;      // 卡槽号，从1开始
    int rfid_;      // 对应卡槽号的RFID

    MC::ErrorCode ret_;
};

class GetDevStatusCmd : public BaseCmd {
public:
    GetDevStatusCmd() : ret_(MC::EC_SUCC), status_code_(-1) {
        ct_ = CT_GET_DEV_STATUS;
    }

    virtual void Ser();
    virtual void Unser();

public:
    // 0 ---- "未初始化"
    // 1 ---- "启动自检"
    // 2 ---  "检测章"
    // 3 ---- "空闲状态"
    // 4 ---- "测试模式"
    // 5 ---- "故障模式"
    // 6 ---- "盖章模式"
    // 7 ---- "维护模式"
    int status_code_;

    MC::ErrorCode ret_;
};

class CoordCvtCmd : public BaseCmd {
public:
    CoordCvtCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_GET_SEAL_COORD;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int x_img_;
    int y_img_;

    int x_dev_;
    int y_dev_;

    MC::ErrorCode ret_;
};

class WriteRatioCmd : public BaseCmd {
public:
    WriteRatioCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_WRITE_CVT_RATIO;
    }

    virtual void Ser();
    virtual void Unser();

public:
    float x_;
    float y_;

    MC::ErrorCode ret_;
};

class ReadRatioCmd : public BaseCmd {
public:
    ReadRatioCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_READ_CVT_RATIO;
    }

    virtual void Ser();
    virtual void Unser();

public:
    float x_;
    float y_;

    MC::ErrorCode ret_;
};

class WriteCaliPtsCmd : public BaseCmd {
public:
    WriteCaliPtsCmd(): len_(10), ret_(MC::EC_SUCC) {
        ct_ = CT_WRITE_CALIBRATION;
        memset(pts_, 0, len_ * sizeof(unsigned short));
    }

    virtual void Ser();
    virtual void Unser();

public:
    unsigned short pts_[10];
    unsigned short len_;

    MC::ErrorCode ret_;
};

class ReadCaliPtsCmd : public BaseCmd {
public:
    ReadCaliPtsCmd(): len_(10), ret_(MC::EC_SUCC) {
        ct_ = CT_READ_CALIBRATION;
        memset(pts_, 0, len_ * sizeof(unsigned short));
    }

    virtual void Ser();
    virtual void Unser();

public:
    unsigned short pts_[10];
    unsigned short len_;

    MC::ErrorCode ret_;
};

class QueryTopCmd : public BaseCmd {
public:
    QueryTopCmd() : ret_(MC::EC_SUCC), status_(-1) {
        ct_ = CT_QUERY_TOP;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int             status_;            //顶盖门状态, 0-开, 1-关, -1--未成功获取到门状态
    MC::ErrorCode   ret_;
};

class ExitMaintainCmd : public BaseCmd {
public:
    ExitMaintainCmd(): ret_(MC::EC_SUCC) {
        ct_ = CT_EXIT_MAIN;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

class StartPreviewCmd : public BaseCmd {
public:
    StartPreviewCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_START_PREVIEW;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;
    int width_;
    int height_;
    int hwnd_;

    MC::ErrorCode ret_;
};

class StopPreviewCmd : public BaseCmd {
public:
    StopPreviewCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_STOP_PREVIEW;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int which_;

    MC::ErrorCode ret_;
};

class CtrlFactoryCmd : public BaseCmd {
public:
    CtrlFactoryCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_FACTORY_CTRL;
    }

    virtual void Ser();
    virtual void Unser();

public:
    int ctrl_;  // 0 --- enable, 1 --- disable

    MC::ErrorCode ret_;
};

class ResetCmd : public BaseCmd {
public:
    ResetCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RESET;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

class RestartCmd : public BaseCmd {
public:
    RestartCmd() : ret_(MC::EC_SUCC) {
        ct_ = CT_RESTART;
    }

    virtual void Ser();
    virtual void Unser();

public:
    MC::ErrorCode ret_;
};

#endif // AGENT_CMD_H_
