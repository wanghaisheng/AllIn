#ifndef CONTROLLER_ERROR_CODE_H_
#define CONTROLLER_ERROR_CODE_H_

#include <string>

namespace MC {

enum ErrorCode {
    EC_SUCC,                    // 成功
    EC_FAIL,                    // 失败
    EC_TIMEOUT,                 // 超时
    EC_PAPER_TIMEOUT,           // 进纸门超时未关闭
    EC_DRIVER_FAIL,             // 调用驱动接口失败
    EC_CONNECTED,               // 设备已连接
    EC_CON_DISCONN,             // 通信连接断开
    EC_DEV_DISCONN,             // 设备已断开
    EC_CONN_OPEN_FAIL,          // 成功连接, 打开设备失败
    EC_RECONN_FAIL,             // 重连设备失败
    EC_STAMPER_DROP,            // 印章掉落
    EC_ALREADY_BOUND,           // MAC地址已绑定
    EC_MAC_MAX,                 // 已绑定最多2个MAC
    EC_INVALID_PARAMETER,       // 参数非法
    EC_ALLOCATE_FAILURE,        // 分配内存失败
    EC_CODE_MISMATCH,           // 认证码不匹配
    EC_SUC_CALL_BIND,           // 成功, 后续只能先调用绑定MAC地址
    EC_NOT_BOUND,               // 该MAC地址未绑定
    EC_MERGE_FAIL,              // 合成照片失败
    EC_MODEL_TYPE_FAIL,         // 模板类型、角度识别失败
    EC_RECOG_FAIL,              // 版面、验证码识别失败
    EC_ELEMENT_FAIL,            // 要素识别失败
    EC_QUERY_DOORS_FAIL,        // 查询门状态失败
    EC_SAFE_CLOSED,             // 安全门已关闭
    EC_SAFE_OPENED,             // 安全门已打开
    EC_OPEN_CAMERA_FAIL,        // 打开凭证摄像头失败
    EC_CAPTURE_FAIL,            // 拍照失败
    EC_PROCESS_IMG_FAIL,        // 纠偏去黑边失败
    EC_TASK_CONSUMED,           // 任务号已被使用
    EC_TASK_NON_EXIST,          // 任务号不存在
    EC_LOCK_MACHINE_FAIL,       // 锁定印控仪失败
    EC_MACHINE_UNLOCKED,        // 印控仪未锁定
    EC_MACHINE_LOCKED,          // 印控仪处于锁定状态
    EC_LOCK_FAILURE,            // 解锁印控仪失败, 重试
    EC_TOP_DOOR_OPEN,           // 顶盖门打开
    EC_UPDATE_STAMP_FAIL,       // 更新章映射失败
    EC_STAMP_NOT_EXIST,         // 章槽无章
    EC_SET_RESO_FAIL,           // 设置分辨率失败
    EC_SET_DPI_FAIL,            // 设置摄像头DPI失败
    EC_OPEN_CAM_FAIL,           // 打开摄像头失败
    EC_CLOSE_CAM_FAIL,          // 关闭摄像头失败
    EC_OPEN_FAIL,               // 打开设备失败
    EC_START_RECORD_FAIL,       // 开启录像失败
    EC_STOP_RECORD_FAIL,        // 停止录像失败
    EC_INVALID_X_PARAM,         // x坐标非法
    EC_INVALID_Y_PARAM,         // y坐标非法
    EC_INVALID_ANGLE,           // 章旋转角度非法
    EC_SEARCH_STAMP_FAIL,       // 原图用印点查找失败
    EC_MAX
};

static std::string ErrorMsg[] = {
    "成功",
    "失败",
    "超时错误",
    "进纸门超时未关闭",
    "调用驱动接口失败",
    "设备已连接",
    "通信连接断开",
    "设备已断开",
    "成功连接, 打开设备失败",
    "重连设备失败",
    "印章掉落",
    "该MAC地址已绑定",
    "已绑定最多2个MAC地址",
    "参数非法",
    "分配内存失败",
    "认证码不匹配",
    "成功, 后续只能先调用绑定MAC地址",
    "该MAC地址未绑定",
    "合成照片失败",
    "模板类型、角度识别失败",
    "版面、验证码识别失败",
    "要素识别失败",
    "查询门状态失败",
    "安全门已关闭",
    "安全门已打开",
    "打开凭证摄像头失败",
    "拍照失败",
    "纠偏去黑边失败",
    "任务号已被使用",
    "任务号不存在",
    "锁定印控仪失败",
    "印控仪未锁定",
    "印控仪处于锁定状态",
    "解锁印控仪失败, 重试",
    "顶盖门打开",
    "更新章映射失败",
    "章槽无章",
    "设置分辨率失败",
    "设置摄像头DPI失败",
    "打开摄像头失败",
    "关闭摄像头失败",
    "打开设备失败",
    "开启录像失败",
    "停止录像失败",
    "x坐标非法",
    "y坐标非法",
    "章旋转角度非法",
    "原图用印点查找失败"
};

static std::string GetErrMsg(enum ErrorCode err)
{
    return ErrorMsg[err];
}

static std::string ErrorResolver[] = {
    "成功",
    "失败",
    "超时错误",
    "进纸门超时未关闭",
    "调用驱动接口失败",
    "通信连接断开",
    "设备已断开",
    "成功连接, 打开设备失败",
    "重连设备失败",
    "印章掉落",
    "该MAC地址已绑定",
    "已绑定最多2个MAC地址",
    "参数非法",
    "分配内存失败",
    "认证码不匹配",
    "成功, 后续只能先调用绑定MAC地址",
    "该MAC地址未绑定",
    "合成照片失败",
    "查询门状态失败",
    "安全门已关闭",
    "安全门已打开",
    "打开凭证摄像头失败",
    "拍照失败",
    "纠偏去黑边失败",
    "任务号已被使用",
    "任务号不存在",
    "锁定印控仪失败",
    "印控仪未锁定",
    "印控仪处于锁定状态",
    "解锁印控仪失败, 重试",
    "先关闭顶盖门",
    "更新章映射失败"
    "章槽无章",
    "设置分辨率失败",
    "设置摄像头DPI失败",
    "打开摄像头失败",
    "关闭摄像头失败",
    "打开设备失败",
    "开启录像失败",
    "停止录像失败",
    "x坐标非法",
    "y坐标非法",
    "章旋转角度在0~360°",
    "原图用印点查找失败"
};

static std::string GetErrResolver(enum ErrorCode err)
{
    return ErrorResolver[err];
}

}   // namespace MC

#endif // CONTROLLER_ERROR_CODE_H_
