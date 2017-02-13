#ifndef MC_AGENT_API_H_
#define MC_AGENT_API_H_

#include <string>

#ifdef MASTERCTRL_AGENT_EXPORTS
#define MASTERCTRL_AGENT_API _declspec(dllexport)
#else
#define MASTERCTRL_AGENT_API _declspec(dllimport)
#ifdef _DEBUG
#pragma comment(lib, "agentd.lib")
#else
#pragma comment(lib, "agent.lib")
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

// 该接口文件包含以下4类接口：
// 1. 印控机接口;
// 2. 图像处理接口;
// 3. 摄像头接口;
// 4. 查询错误信息接口；

/////////////////////// 1. 印控机API //////////////////////////////

// 获取印控机与PC连接状态
// cnn      --- 0: 断开连接
//          --- 1: 连接
MASTERCTRL_AGENT_API int ST_QueryCnn(
    int& cnn);

// 打开设备连接
MASTERCTRL_AGENT_API int ST_Open();

// 关闭设备连接
MASTERCTRL_AGENT_API int ST_Close();

// 获取印控仪编号
MASTERCTRL_AGENT_API int ST_QueryMachine(
    std::string& sn);

// 设置印控机编号
MASTERCTRL_AGENT_API int ST_SetMachine(
    const std::string& sn);

// 查询MAC地址, 一台印控机最多支持绑定2个MAC地址
// mac1         --- mac地址1
// mac2         --- mac地址2
MASTERCTRL_AGENT_API int ST_QueryMAC(
    std::string& mac1, 
    std::string& mac2);

// 绑定MAC地址
// mac     --- 待绑定MAC地址
MASTERCTRL_AGENT_API int ST_BindMAC(
    const std::string& mac);

// 解绑MAC地址
// mac     --- 待解绑MAC地址
MASTERCTRL_AGENT_API int ST_UnbindMAC(
    const std::string& mac);

// 根据卡槽号获取对应的RFID
// slot         --- 卡槽号，从1开始
// rfid         --- 对应卡槽号的rfid
MASTERCTRL_AGENT_API int ST_GetRFID(
        int     slot,
        int&    rfid);

// 准备用印
// stamp_num        --- 章槽号, 从1开始
// timeout          --- 进纸门超时未关闭时间, 单位秒
// task_id          --- 任务号, 仅能使用一次
MASTERCTRL_AGENT_API int ST_PrepareStamp(
    char            slot,
    int             timeout, 
    std::string&    task_id);

// 普通用印
MASTERCTRL_AGENT_API int ST_OrdinaryStamp(
    const std::string&  task,       // 任务号
    int                 slot,       // 印章卡槽号(1-6)
    int                 ink,        // 0 - 不蘸印油， 1 - 蘸印油
    int                 x,          // 盖章位置x坐标, 相对于印控机坐标系
    int                 y,          // 盖章位置y坐标, 相对于印控机坐标系
    int                 angle,      // 印章旋转角度, 大于等于0且小于360度
    int                 type = 0);  // 0 - 普通用印, 1 - 骑缝章

// 结束用印. 完成以下操作：
// 1. 解锁印控仪;
// 2. 删除任务号;
// 3. 弹出纸门;
// 
// task         --- 准备用印时获得的用印任务号
MASTERCTRL_AGENT_API int ST_FinishStamp(
    const std::string& task);

// 印章校准
// slot     --- 卡槽号, 从1开始
MASTERCTRL_AGENT_API int ST_Calibrate(
    int slot);

// 查询印章状态
// status   --- 对应章槽上是否有章
//              0 - 表示无章, 1 - 表示有章. 如"001101"
MASTERCTRL_AGENT_API int ST_QueryStampers(
    int* status);

// 查询进纸门状态
// status        --- 0-关, 1-开
MASTERCTRL_AGENT_API int ST_QueryPaper(
    int& status);

// 打开进纸门
// timeout      --- 纸门超时未关闭时间，单位秒
MASTERCTRL_AGENT_API int ST_OpenPaper(
    int timeout = 30);

// 安全门状态查询
// status   --- 0-关,1-开
MASTERCTRL_AGENT_API int ST_QuerySafe(
    int& status);

// 开关安全门
// ctrl     --- 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ST_ControlSafe(
    int ctrl);

// 蜂鸣器开关
// ctrl     --- 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ST_ControlBeep(
    int ctrl);

// 报警器开关
//alarm     --- 0(开门报警器)
//              1(振动报警器)
//switches  --- 报警器开关
//              1(开启);
//              0(关闭)
MASTERCTRL_AGENT_API int ST_ControlAlarm(
    int alarm, 
    int switches);

// 卡槽数量查询
// num      --- 实际印章数
MASTERCTRL_AGENT_API int ST_QuerySlots(
    int &num);

// 锁定印控仪
MASTERCTRL_AGENT_API int ST_Lock();

// 解锁印控仪
MASTERCTRL_AGENT_API int ST_Unlock();

// 印控仪锁定状态
// lock     --- 0: 已锁定
//              1：未锁定
MASTERCTRL_AGENT_API int ST_QueryLock(
    int& lock);

// 设置安全门
MASTERCTRL_AGENT_API int ST_SetSideDoor(
    int keep,
    int timeout);

//  获取设备型号
MASTERCTRL_AGENT_API int ST_GetDevModel(
    std::string& model);

// 补光灯控制
// which    --- 补光灯类型
//              1 -- 安全门的补光灯; 
//              2 -- 凭证摄像头的补光灯
// ctrl     --- 控制开关
//              0 -- 关; 1 -- 开
// value    --- 打开补光灯时同步设置补光灯的亮度值
MASTERCTRL_AGENT_API int ST_ControlLed(
    int which,
    int ctrl,
    int value);

// 用印参数检查(针对印控机的物理坐标)
// x        --- x坐标值
// y        --- y坐标值
// angle    --- 章旋转角度值(0 ~ 360)
MASTERCTRL_AGENT_API int ST_CheckParam(
    int x,
    int y,
    int angle);

/////////////////////// 2. 图像处理API //////////////////////////////

// 合成照片
// p1       --- 图片1路径
// p2       --- 图片2路径
// merged   --- 合成图片路径
MASTERCTRL_AGENT_API int ST_MergePhoto(
    const std::string& p1, 
    const std::string& p2,
    const std::string& merged);

// 版面、验证码识别
// path         --- 切图路径
// template_id  --- 模板ID
// trace_num    --- 追溯码
MASTERCTRL_AGENT_API int ST_RecognizeImage(
    const std::string&  path, 
    std::string&        template_id, 
    std::string&        trace_num);

// 要素识别
// path         --- 切图路径
MASTERCTRL_AGENT_API int ST_IdentifyElement(
    const           std::string& path, 
    int             x, 
    int             y, 
    int             width, 
    int             height,
    int             angle, 
    std::string&    result);

/////////////////////// 3. 摄像头API //////////////////////////////

// 打开摄像头
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
MASTERCTRL_AGENT_API int ST_OpenCamera(
    int which);

// 关闭摄像头
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
MASTERCTRL_AGENT_API int ST_CloseCamera(
     int which);

// 查看摄像头状态
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// status   --- 摄像头开关状态
//              0--关，1--开
MASTERCTRL_AGENT_API int ST_QueryCamera(
    int which, 
    int& status);

// 拍照
// which        --- 0：凭证摄像头
//                  1：环境摄像头
//                  2：侧门摄像头
// ori_path     --- 存放原图路径
// cut_path     --- 存放切图路径
MASTERCTRL_AGENT_API int ST_Snapshot(
    int                 which, 
    const std::string&  ori_path, 
    const std::string&  cut_path);

// 设置摄像头分辨率
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// x        --- 宽度
// y        --- 高度
MASTERCTRL_AGENT_API int ST_SetResolution(
     int which,
     int x,
     int y);

// 开始录制视频
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// path     --- 视频存放路径, 包含扩展名的全路径(.avi格式)
MASTERCTRL_AGENT_API int ST_StartRecordVideo(
    int which, 
    const std::string& path);

// 停止录制视频
// which    --- 0：凭证摄像头
//              1：环境摄像头
//              2：侧门摄像头
// path     --- 视频存放路径, 包含扩展名的全路径(.avi格式)
MASTERCTRL_AGENT_API int ST_StopRecordVideo(
    int which,
    const std::string& path);

/////////////////////// 4. 查询错误信息 API //////////////////////////////

// 根据错误码获取错误信息
MASTERCTRL_AGENT_API int ST_GetError(
    int             err_code,
    std::string&    err_msg,
    std::string&    err_resolver);

#ifdef __cplusplus
}
#endif

#endif // MC_AGENT_API_H_
