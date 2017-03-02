#include "stdafx.h"
#include "log.h"
#include "stamping_mgr.h"
#include "tool.h"

SharedMem* SharedMem::mem_inst_ = NULL;

MC::Tool* MC::Tool::g_inst = NULL;

MC::Tool* MC::Tool::GetInst()
{
    if (NULL == g_inst)
        g_inst = new (std::nothrow) MC::Tool;

    return g_inst;
}

void MC::Tool::SetStat(const DeviceStat& stat)
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    device_stat_.conn_ = stat.conn_;
    device_stat_.status_ = stat.status_;
    device_stat_.cause_ = stat.cause_;

    Log::WriteLog(LL_DEBUG, "Tool::SetStat->更新设备状态 = (%s, %s, %s)",
        device_stat_.conn_ ? "连接" : "断开",
        MC::ConnDes[device_stat_.status_].c_str(),
        device_stat_.cause_.c_str());
}

// 印控机与PC是否线连接，true --- 表示连接
bool MC::Tool::Connected()
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    return device_stat_.conn_;
}

MC::ConnStatus MC::Tool::GetStatus()
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    return device_stat_.status_;
}

std::string MC::Tool::GetCause()
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    return device_stat_.cause_;
}

// 0--关, 1--开
void MC::Tool::SetTop(int top)
{
    boost::lock_guard<boost::mutex> lk(door_mtx_);
    doors_stat_.top_door_closed_ = top == 0;
    Log::WriteLog(LL_DEBUG, "MC::Tool::SetTop->顶盖门%s", top == 0? "关闭": "开");
}

void MC::Tool::SeteSafe(int safe)
{
    boost::lock_guard<boost::mutex> lk(door_mtx_);
    doors_stat_.safe_door_closed_ = safe == 0;
    Log::WriteLog(LL_DEBUG, "MC::Tool::SeteSafe->安全门%s", safe == 0 ? "关闭" : "开");
}

void MC::Tool::SetPaper(int paper)
{
    boost::lock_guard<boost::mutex> lk(door_mtx_);
    doors_stat_.paper_door_closed_ = paper == 0;
    Log::WriteLog(LL_DEBUG, "MC::Tool::SetPaper->进纸门%s", paper == 0 ? "关闭" : "开");
    if (0 == paper)
        SetEvent(paper_door_ev_);
}

const MC::AllDoorStat& MC::Tool::GetDoors()
{
    boost::lock_guard<boost::mutex> lk(door_mtx_);
    //0x0B, 获取门状态(len = 4)
    //P1:   推纸门状态  0 关闭，1 开启， 2检测错误
    //P2:   电子锁状态，同上
    //P3:   机械锁状态，同上
    //P4:   顶盖状态，同上
    char doors[4] = { 0 };
    int ret = FGetDoorsPresent(doors, 4);
    if (0 == ret) {
        doors_stat_.top_door_closed_ = 0 == doors[3];
        doors_stat_.safe_door_closed_ = 0 == doors[2];
        doors_stat_.paper_door_closed_ = 0 == doors[0];
    }

    return doors_stat_;
}

void MC::Tool::UpdateCams(int which, bool status)
{
    switch (which) {
        case 0:
            paper_cam_ = status;
            break;
        case 1:
            env_cam_ = status;
            break;
        case 2:
            side_cam_ = status;
            break;
        default:
            break;
    }
}

bool MC::Tool::QueryCam(int which)
{
    switch (which) {
        case 0:
            return paper_cam_;
        case 1:
            return env_cam_;
        case 2:
            return side_cam_;
        default:
            return false;
    }
}

int PrepareCamera() 
{
    int ret = FLightCtrl(2, 1);
    if (0 != ret)
        Log::WriteLog(LL_ERROR, "PrepareCamera->打开凭证补光灯失败, er: %d", ret);

    ret = FLightBrightness(2, MC::SvrConfig::GetInst()->brightness_);
    if (0 != ret)
        Log::WriteLog(LL_ERROR, "PrepareCamera->凭证补光灯亮度调节失败, er: %d", ret);

    ret = OpenCamera(PAPERCAMERA);
    if (0 != ret)
        Log::WriteLog(LL_ERROR, "PrepareCamera->打开凭证摄像头失败, er: %d", ret);

    ret = SetResolution(
        PAPERCAMERA,
        MC::SvrConfig::GetInst()->resolution_width_,
        MC::SvrConfig::GetInst()->resolution_height_);
    if (0 != ret)
        Log::WriteLog(LL_ERROR, "PrepareCamera->设置凭证摄像头分辨率失败, er: %d", ret);

    Log::WriteLog(LL_DEBUG, "PrepareCamera->凭证摄像头已准备好, ret: %d", ret);
    return ret;
}

void DisableCamera()
{
    CloseCamera(PAPERCAMERA);
    FLightCtrl(2, 0);
}

/////////////////////////////// 来自印控机的异步回调 /////////////////////////////////

// 连接、断开回调
// 印控机与PC连接成功后，需要打开设备才能进行通信，
// 连接与打开设备是两个不同的动作。
int _stdcall ConnectCallBack(const char* dev_path, unsigned int msg)
{
    SharedMem::GetInst()->WriteSharedMem(msg);

    switch (msg) {
    case 0: { // 断开
        MC::DeviceStat stat;
        stat.conn_ = false;
        stat.status_ = 0 == ::FCloseDev() ? MC::CS_DISCONN_SUC : MC::CS_RECONN_FAIL;
        stat.cause_ = "断开设备";
        MC::Tool::GetInst()->SetStat(stat);

        // DisableCamera();
    }
        break;
    case 1: { // 重连
        MC::DeviceStat stat;
        stat.conn_ = true;
        stat.status_ = 0 == ::FOpenDev(NULL) ? MC::CS_RECON_SUC : MC::CS_RECON_FAIL;
        stat.cause_ = "重连设备";
        MC::Tool::GetInst()->SetStat(stat);

        // PrepareCamera();
    }
        break;
    default:
        break;
    }

    return 0;
}

#define FULL_MAP_NAME       "Local\\MyFileMappingObject"

// Max size of the file mapping object.  
#define MAP_SIZE            4

bool SharedMem::CreateSharedMem()
{
    // Create the file mapping object.  
    hMapFile_ = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // Use paging file - shared memory  
        NULL,                   // Default security attributes  
        PAGE_READWRITE,         // Allow read and write access  
        0,                      // High-order DWORD of file mapping max size  
        MAP_SIZE,               // Low-order DWORD of file mapping max size  
        FULL_MAP_NAME           // Name of the file mapping object  
        );
    if (hMapFile_ == NULL) {
        Log::WriteLog(LL_ERROR, "CreateSharedMem->CreateFileMapping failed w/err 0x%08lx", 
            GetLastError());
        return false;
    }

    Log::WriteLog(LL_DEBUG, "CreateSharedMem->The file mapping (%s) is created", 
        FULL_MAP_NAME);

    // Map a view of the file mapping into the address space of the current   
    // process.  
    pView_ = MapViewOfFile(
        hMapFile_,               // Handle of the map object  
        FILE_MAP_ALL_ACCESS,    // Read and write access  
        0,                      // High-order DWORD of the file offset   
        0,                      // Low-order DWORD of the file offset   
        MAP_SIZE               // The number of bytes to map to view  
        );
    if (pView_ == NULL) {
        Log::WriteLog(LL_ERROR, "CreateSharedMem->MapViewOfFile failed w/err 0x%08lx",
            GetLastError());
        CloseHandle(hMapFile_);
        return false;
    }

    ClearMem();
    Log::WriteLog(LL_DEBUG, "CreateSharedMem->The file view is mapped");
    return true;
}

void SharedMem::ClearMem()
{
    int buf = -1;
    // clear the view.
    memcpy_s(pView_, MAP_SIZE, &buf, sizeof(buf));
}

void SharedMem::WriteSharedMem(int to_write)
{
    DWORD cbMessage = sizeof(to_write);

    // Write the message to the view.  
    memcpy_s(pView_, MAP_SIZE, &to_write, cbMessage);
    SetEvent(syn_ev_);

    Log::WriteLog(LL_DEBUG, "WriteSharedMem->This value is written to the view:\"%d\"", 
        to_write);
}

void SharedMem::ReleaseSharedMem()
{
    if (hMapFile_) {
        if (pView_)
        {
            // Unmap the file view.  
            UnmapViewOfFile(pView_);
            pView_ = NULL;
        }
        // Close the file mapping object.  
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
    }
}

// 状态回调
int _stdcall DevMsgCallBack(
    unsigned int uMsg,
    unsigned int wParam,
    long lParam,
    unsigned char* data,
    unsigned char len)
{
    switch (uMsg) {
    case 0xA0: // 盖章过程中下压
        SharedMem::GetInst()->WriteSharedMem(uMsg);
        break;
    case 0xA1: // 盖章过程中机械手臂回到印油线
        SharedMem::GetInst()->WriteSharedMem(uMsg);
        break;
    case 0xA2: { // 盖章完成通知
        if (0 == wParam) { // success
            SharedMem::GetInst()->WriteSharedMem(0xA2);
            StampingMgr::GetInst()->SetStampingResult(MC::EC_SUCC);
        } else {   // failure
            SharedMem::GetInst()->WriteSharedMem(0xA3);
            StampingMgr::GetInst()->SetStampingResult(MC::EC_FAIL);
        }
        StampingMgr::GetInst()->Signal();
    }
        break;
    case 0xA3:  { // 盖章过程中印章掉落
        SharedMem::GetInst()->WriteSharedMem(0xA4);

        StampingMgr::GetInst()->SetStampingResult(MC::EC_STAMPER_DROP);
        StampingMgr::GetInst()->Signal();

        // 印章掉落通知
        MC::DeviceStat stat;
        stat.conn_ = true;
        stat.status_ = MC::CS_STAMPER_DROP;
        stat.cause_ = "印章掉落";
        MC::Tool::GetInst()->SetStat(stat);
    }
        break;
    case 0xA4: // 纸门信号
        SharedMem::GetInst()->WriteSharedMem(0xA5);

        MC::Tool::GetInst()->SetPaper(wParam);
        break;
    case 0xA5: // 盖章错误
        SharedMem::GetInst()->WriteSharedMem(0xA6);

        StampingMgr::GetInst()->SetStampingResult(MC::EC_FAIL);
        StampingMgr::GetInst()->Signal();
        break;
    case 0xA6: // 侧门关闭通知
        if (0 == wParam) { // closed
            SharedMem::GetInst()->WriteSharedMem(0xA7);
        } else { // open
            SharedMem::GetInst()->WriteSharedMem(0xA8);
        }

        MC::Tool::GetInst()->SeteSafe(wParam);
        break;
    case 0xA7: // 顶盖门关闭
        SharedMem::GetInst()->WriteSharedMem(0xA9);

        MC::Tool::GetInst()->SetTop(wParam);
        break;
    case 0xA8: // 电子锁上锁通知
    {
        if (0 == wParam)
            SharedMem::GetInst()->WriteSharedMem(0xAA);
        else
            SharedMem::GetInst()->WriteSharedMem(0xAB);
    }
        break;
    default:
        break;
    }

    return 0;
}
