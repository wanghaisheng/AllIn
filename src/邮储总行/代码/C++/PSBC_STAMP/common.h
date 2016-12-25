#ifndef PSBC_STAMP_COMMON_H_
#define PSBC_STAMP_COMMON_H_

#include <string>

enum ErrorCode {
    EC_SUC,                     // 成功
    EC_FAIL,                    // 普通失败
    EC_OPEN_FAIL,               // 打开设备失败
    EC_QUERY_STAMP_FAIL,        // 检测章失败
    EC_API_FAIL,                // 调用驱动接口失败
    EC_MACHINE_MISMATCH,        // 设备编号不匹配
    EC_INVALID_PARAMETER,       // 参数非法
    EC_UPTO_MAX_EXCEPTION,      // 最大记录异常开锁信息条目(4条)
    EC_INTO_MAINTAIN_FAIL,      // 进入维护模式失败
    EC_QUIT_MAINTAIN_FAIL,      // 退出维护模式失败
    EC_QUERY_DEVICE_FAIL,       // 查询系统状态失败
    EC_NOT_INIT,                // 未初始化
    EC_STARTUP_EXAM,            // 启动自检
    EC_FREE,                    // 空闲状态
    EC_TEST,                    // 测试模式
    EC_BREAKDOWN,               // 故障模式
    EC_STAMPING,                // 盖章模式
    EC_MAINTAIN,                // 维护模式
    EC_FILE_NOT_EXIST,          // 文件不存在
    EC_OPEN_VIDEO_FAIL,         // 打开摄像头失败
    EC_CLOSE_VIDEO_FAIL,        // 关闭摄像头失败
    EC_SET_CAMERA_PARAM_FAIL,   // 设置摄像头属性失败
    EC_OPEN_CAMERA_LED_FAIL,    // 开启补光灯失败
    EC_ADJUST_LED_FAIL,         // 补光灯亮度调节失败
    EC_CLOSE_CAMERA_LED_FAIL,   // 关闭补光灯失败
    EC_DUP_SLOT_NUM,            // 相同的槽位号
    EC_CAPTURE_FAIL,            // 拍照失败
    EC_IMG_PROCESS_FAIL,        // 纠偏去黑边失败
    EC_CUT_TO_ORI_FAIL,         // 切图转原图坐标失败
    EC_PAPER_OPEN,              // 进纸门未关闭
    EC_SAFE_OPEN,               // 安全门未关闭
    EC_GET_RFID_FAIL,           // 获取印章RFID失败
    EC_NO_SEAL,                 // 无印章
    EC_MAX
};

static std::string ec_des[EC_MAX] = 
{
    "成功",
    "普通失败",
    "打开设备失败",
    "检测章失败",
    "调用驱动接口失败",
    "设备编号不匹配",
    "参数非法", 
    "最大记录异常开锁信息条目(4条)",
    "进入维护模式失败",
    "退出维护模式失败",
    "查询系统状态失败",
    "未初始化",
    "启动自检",
    "空闲状态",
    "测试模式",
    "故障模式",
    "盖章模式",
    "维护模式",
    "文件不存在",
    "打开摄像头失败",
    "关闭摄像头失败",
    "设置摄像头属性失败",
    "开启补光灯失败",
    "补光灯亮度调节失败",
    "关闭补光灯失败",
    "相同的槽位号",
    "拍照失败",
    "纠偏去黑边失败",
    "切图转原图坐标失败",
    "进纸门未关闭",
    "安全门未关闭",
    "获取印章RFID失败",
    "无印章"
};

#endif // PSBC_STAMP_COMMON_H_
