#include "listen.h"
#include "log.h"
#include "api.h"

Listen* Listen::listen_inst_ = NULL;

#define FULL_MAP_NAME       "Local\\MyFileMappingObject"

// The number of bytes of a file mapping to map to the view. All bytes of the   
// view must be within the maximum size of the file mapping object. If   
// VIEW_SIZE is 0, the mapping extends from the offset (VIEW_OFFSET) to the   
// end of the file mapping.  
#define VIEW_SIZE           4

bool Listen::Start()
{
    if (running_)
        return true;

    // Try to open the named file mapping identified by the map name.  
    map_file_ = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,    // Read access  
        FALSE,                  // Do not inherit the name  
        FULL_MAP_NAME           // File mapping name   
        );
    if (map_file_ == NULL) {
        Log::WriteLog(LL_ERROR, "Listen::start->OpenFileMapping failed w/err 0x%08lx",
            GetLastError());
        return false;
    }

    Log::WriteLog(LL_DEBUG, "Listen::start->The file mapping (%s) is opened", FULL_MAP_NAME);

    running_ = true;
    work_thread_ =
        new (std::nothrow) boost::thread(boost::bind(&Listen::ListenFunc, this));
    return true;
}

int Listen::GetNotifyValue()
{
    map_view_ = MapViewOfFile(
        map_file_,               // Handle of the map object  
        FILE_MAP_ALL_ACCESS,     // Read access  
        0,                      // High-order DWORD of the file offset   
        0,                      // Low-order DWORD of the file offset  
        VIEW_SIZE               // The number of bytes to map to view  
        );
    if (map_view_ == NULL) {
        Log::WriteLog(LL_ERROR, "Listen::GetNotifyValue->MapViewOfFile failed w/err 0x%08lx",
            GetLastError());
        return -1;
    }

    // Read and display the content in view.  
    int msg = *((int*)map_view_);

    // reset the view.
    int buf = -1;
    memcpy_s(map_view_, VIEW_SIZE, &buf, sizeof(buf));	return msg;

    return msg;
}

void Listen::ListenFunc()
{
    while (running_) {
        WaitForSingleObject(syn_ev_, INFINITE);
        ResetEvent(syn_ev_);

        int msg = GetNotifyValue();
        // 断开、重连callback
        if (0 == msg || 1 == msg) {
            Log::WriteLog(LL_DEBUG, "Listen::ListenFunc->Read from the file mapping:\"%0x\"",
                "callback size: %d",
                msg,
                connect_callback_vec_.size());

            for (int i = 0; i < connect_callback_vec_.size(); ++i) {
                ConnectCallback cb = (ConnectCallback)(connect_callback_vec_.at(i));
                cb(msg);
            }

            continue;
        }

        // not within pre-defined values
        if (msg < 0xA0 || msg > 0xAB)
            continue;

        Log::WriteLog(LL_DEBUG, "Listen::ListenFunc->Read from the file mapping:\"%0x\"",
            msg);

        // 其他消息回掉
        for (int i = 0; i < msg_callback_vec_.size(); ++i) {
            EventCallback cb = (EventCallback)(msg_callback_vec_.at(i));
            if (NULL == cb)
                continue;
            
            switch (msg) {
            case 0xA0:
                cb(0xA0, 0);
                break;
            case 0xA1:
                cb(0xA1, 0);
                break;
            case 0xA2:
                cb(0xA2, 0);
                break;
            case 0xA3:
                cb(0xA2, 1);
                break;
            case 0xA4:
                cb(0xA3, 0);
                break;
            case 0xA5:
                cb(0xA4, 0);
                break;
            case 0xA6:
                cb(0xA5, 0);
                break;
            case 0xA7:
                cb(0xA6, 0);
                break;
            case 0xA8:
                cb(0xA6, 1);
                break;
            case 0xA9:
                cb(0xA7, 0);
                break;
            case 0xAA:
                cb(0xA8, 0);
                break;
            case 0xAB:
                cb(0xA8, 1);
                break;
            default:
                break;
            }
        }
    }
}
