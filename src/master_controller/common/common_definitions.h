#ifndef CONTROLLER_COMMON_DEFINITIONS_H_
#define CONTROLLER_COMMON_DEFINITIONS_H_

#include <string>
#include "err_code.h"

#define PIPE_MAX_BUF    4096
#define PIPE_BUSY_WAIT  2000
#define CMD_BUF_SIZE    1024
#define MAX_MAC_SIZE    17
#define STAMPING_WAIT_TIME  30  // 用印等待完成时间(从发起盖章到等待盖章完成通知)
#define PREVIEW_WIDTH   640
#define PREVIEW_HEIGHT  480

namespace MC {

enum ConnType {
    CT_PIPE,
    CT_MQ,
    CT_MAX
};

enum ConnStatus {
    CS_FREE         = 0,    // 成功打开设备并空闲
    CS_OPEN_FAIL    = 1,    // 打开设备失败
    CS_CLOSE_SUC    = 2,    // 成功关闭设备
    CS_CLOSE_FAIL   = 3,    // 关闭设备失败
    CS_RECON_SUC    = 4,    // 重连设备成功
    CS_RECON_FAIL   = 5,    // 重连设备失败
    CS_DISCONN_SUC  = 6,    // 设备断开并关闭成功
    CS_RECONN_FAIL  = 7,    // 设备断开并关闭设备失败
    CS_STAMPER_DROP = 8,    // 印章掉落,进入故障模式
};

static std::string ConnDes[] = 
{
    "成功打开设备并空闲",
    "打开设备失败",
    "成功关闭设备",
    "关闭设备失败",
    "重连设备成功",
    "重连设备失败",
    "设备断开并关闭成功",
    "设备断开并关闭设备失败",
    "印章掉落,进入故障模式"
};

// 印控机与PC连接状态描述
struct DeviceStat {
    bool        conn_;             //PC与印控机是否成功连接
    ConnStatus  status_;
    std::string cause_;
};

// 印控机门状态描述
struct AllDoorStat {
    bool top_door_closed_;      // 顶盖门是否关闭
    bool safe_door_closed_;     // 机械锁是否关闭
    bool elec_door_closed_;     // 电子锁是否关闭
    bool paper_door_closed_;    // 进纸门是否关闭
};

// 任务号状态
enum TaskState {
    TS_ALIVE,   // 生成任务号，未使用
    TS_USED,    // 任务号被使用
    TS_DEAD,    // 结束用印, 删除任务号
    TS_NON_EXIST
};

#ifndef PATHSPLIT_CHAR
#define PATHSPLIT_CHAR      '\\'
#endif

bool GetMoudulePath(std::string& path);

void KillProcessByName(const char *filename);

bool LookupProcessByName(const char* filename);

#ifdef _DEBUG
static const std::string SERVER_NAME = "mc_exed.exe";
#else
static const std::string SERVER_NAME = "mc_exe.exe";
#endif

static const int MQ_MAX_MSG_NUM = 512;
static const int MQ_MAX_MSG_SIZE = 1024;    // 1K

} // namespace MC

#endif // CONTROLLER_COMMON_DEFINITIONS_H_
