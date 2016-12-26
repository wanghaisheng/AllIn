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

// 获取印控仪编号
MASTERCTRL_AGENT_API int QueryMachine(
    std::string& sn);

// 设置印控机编号
MASTERCTRL_AGENT_API int SetMachine(
    const std::string& sn);

// 初始化印控机
MASTERCTRL_AGENT_API int InitMachine(
    const std::string& key);

// 查询MAC地址
MASTERCTRL_AGENT_API int QueryMAC(
    std::string& mac1, 
    std::string& mac2);

// 功能:     绑定MAC地址
//
// 输入参数:
//		const std::string & mac     --- 待绑定MAC地址
//
// 输出参数:
//		
// 返回值:
MASTERCTRL_AGENT_API int BindMAC(
    const std::string& mac);

// 功能:     解绑MAC地址
//
// 输入参数:
//		const std::string & mac     --- 待解绑MAC地址
//
// 输出参数:
//		
// 返回值:
MASTERCTRL_AGENT_API int UnbindMAC(
    const std::string& mac);

// 功能:     准备用印
//
// 输入参数:
//		char stamp_num          --- 章槽号, 从1开始
//		int timeout             --- 进纸门超时未关闭时间, 单位秒
//
// 输出参数:
//		std::string & task_id   --- 任务号, 仅能使用一次
//
// 返回值:
MASTERCTRL_AGENT_API int PrepareStamp(
    char            stamp_num, 
    int             timeout, 
    std::string&    task_id);

// 功能:     查询进纸门状态
//
// 输入参数:
//
// 输出参数:
//		int & status        --- 0-关, 1-开
//
// 返回值:
MASTERCTRL_AGENT_API int QueryPaper(
    int& status);

// 拍照
MASTERCTRL_AGENT_API int Snapshot(
    int                 ori_dpi, 
    int                 cut_dpi, 
    const std::string&  ori_path, 
    const std::string&  cut_path);

// 合成照片
MASTERCTRL_AGENT_API int MergePhoto(
    const std::string& p1, 
    const std::string& p2,
    const std::string& merged);

// 版面、验证码识别
// path         --- 切图路径
// template_id  --- 模板ID
// trace_num    --- 追溯码
MASTERCTRL_AGENT_API int RecognizeImage(
    const std::string&  path, 
    std::string&        template_id, 
    std::string&        trace_num);

// 要素识别
// path         --- 切图路径
MASTERCTRL_AGENT_API int IdentifyElement(
    const           std::string& path, 
    int             x, 
    int             y, 
    int             width, 
    int             height,
    int             angle, 
    std::string&    result);

// 普通用印
MASTERCTRL_AGENT_API int OrdinaryStamp(
    const std::string&  task,
    const std::string&  voucher,
    int                 num, 
    int                 x,          // 盖章位置x坐标, 原始图片中的像素
    int                 y,          // 盖章位置y坐标, 原始图片中的像素
    int                 angle);     // 印章旋转角度, 大于等于0且小于360度

// 自动用印
MASTERCTRL_AGENT_API int AutoStamp(
    const std::string&  task,
    const std::string&  voucher, 
    int                 num);

// 结束用印
MASTERCTRL_AGENT_API int FinishStamp(
    const std::string& task);

// 释放印控机
MASTERCTRL_AGENT_API int ReleaseStamp(
    const std::string& machine);

// 印章校准
// slot     --- 卡槽号, 从1开始
MASTERCTRL_AGENT_API int Calibrate(
    int slot);

// 查询印章状态
// status   --- 0-无章, 1-有章, 如"001101"
MASTERCTRL_AGENT_API int QueryStampers(
    int* status);

// 安全门状态
// status   --- 0-关,1-开
MASTERCTRL_AGENT_API int QuerySafe(
    int& status);

// 开关安全门
// ctrl     --- 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ControlSafe(
    int ctrl);

// 蜂鸣器开关
// ctrl     --- 0 - 关, 1 - 开
MASTERCTRL_AGENT_API int ControlBeep(
    int ctrl);

// 报警器开关
//alarm     --- 0(开门报警器)
//              1(振动报警器)
//switches  --- 报警器开关
//              1(开启);
//              0(关闭)
MASTERCTRL_AGENT_API int ControlAlarm(
    int alarm, 
    int switches);

// 卡槽数量查询
// num      --- 实际印章数
MASTERCTRL_AGENT_API int QuerySlot(
    int& num);

// 获取详细错误信息
MASTERCTRL_AGENT_API int GetError(
    int             err_code,
    std::string&    err_msg,
    std::string&    err_resolver);

#ifdef __cplusplus
}
#endif

#endif // MC_AGENT_API_H_
