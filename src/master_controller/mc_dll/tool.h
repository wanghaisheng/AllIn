﻿#ifndef CONTROLLER_TOOL_H_
#define CONTROLLER_TOOL_H_

#include <string>
#include <windows.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <windows.h>    // 需要放在头文件RZCamera.h前面, 否则编译不过
#include "RZCamera.h"
#include "log.h"
#include "parse.h"
#include "USBControlF60.h"
#include "common_definitions.h"

int _stdcall ConnectCallBack(const char* dev_path, unsigned int msg);

int _stdcall DevMsgCallBack(
    unsigned int        uMsg, 
    unsigned int        wParam, 
    long                lParam,
    unsigned char*      data, 
    unsigned char       len);


class SharedMem {
public:
    static SharedMem* GetInst() {
        if (mem_inst_ == NULL)
            mem_inst_ = new SharedMem;

        return mem_inst_;
    }

    ~SharedMem() {
        ReleaseSharedMem();
    }

    bool CreateSharedMem();
    void WriteSharedMem(int to_write);
    void ReleaseSharedMem();
    void ClearMem();

private:
    SharedMem() {
        syn_ev_ = CreateEvent(NULL, TRUE, FALSE, "Local\\MySynEvent");
    }

private:
    static SharedMem* mem_inst_;

    PVOID pView_;
    HANDLE hMapFile_;

    HANDLE syn_ev_;
};

int PrepareCamera();
void DisableCamera();

namespace MC {

class Tool {
public:
    static Tool* GetInst();

    void SetStat(const DeviceStat& stat);

    bool Connected();

    ConnStatus GetStatus();

    std::string GetCause();

    void SetTop(int top);

    void SeteSafe(int safe);

    void SetPaper(int paper);

    const AllDoorStat& GetDoors();

    void UpdateCams(int which, bool status);

    bool QueryCam(int which);

private:
    Tool() {
        paper_cam_ = false;
        env_cam_ = false;
        side_cam_ = false;

        paper_door_ev_ = CreateEvent(
            NULL,       // 默认属性
            TRUE,       // 手动reset
            FALSE,      // 初始状态 non-signaled 
            NULL);      // 未命名
    }

private:
    static Tool* g_inst;

    boost::mutex mutex_;
    DeviceStat device_stat_;

    boost::mutex door_mtx_;
    AllDoorStat doors_stat_;

    bool paper_cam_;
    bool env_cam_;
    bool side_cam_;

public:
    HANDLE      paper_door_ev_;     // 进纸门关闭事件
}; // class Tool

} // namespace MC

#endif // CONTROLLER_TOOL_H_
