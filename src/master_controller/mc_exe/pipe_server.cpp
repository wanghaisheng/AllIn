#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/exception/all.hpp>
#include "recver.h"
#include "log.h"
#include "pipe_server.h"
#include "common_definitions.h"

Recver g_recver;

VOID DisconnectAndClose(LPPIPEINST);
BOOL CreateAndConnectInstance(LPOVERLAPPED);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);
VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED);

HANDLE hPipe;

int StartServer()
{
    if (!g_recver.Start()) {
        Log::WriteLog(LL_ERROR, "StartPipe->fails to start recver");
        return -1;
    }

    if (MC::CT_PIPE == g_recver.CnnType()) {
        HANDLE hConnectEvent;
        OVERLAPPED oConnect;
        LPPIPEINST lpPipeInst;
        DWORD dwWait, cbRet;
        BOOL fSuccess, fPendingIO;

        // 用于连接操作的事件对象 
        hConnectEvent = CreateEvent(
            NULL,    // 默认属性
            TRUE,    // 手工reset
            TRUE,    // 初始状态 signaled 
            NULL);   // 未命名

        if (hConnectEvent == NULL)
        {
            printf("CreateEvent failed with %d.\n", GetLastError());
            return 0;
        }
        // OVERLAPPED 事件
        oConnect.hEvent = hConnectEvent;

        // 创建连接实例，等待连接
        fPendingIO = CreateAndConnectInstance(&oConnect);

        while (1) {
            // 等待客户端连接或读写操作完成 
            dwWait = WaitForSingleObjectEx(
                hConnectEvent,  // 等待客户端连接的事件 
                INFINITE,       // 无限等待
                TRUE);

            switch (dwWait)
            {
            case 0:
                // pending
                if (fPendingIO) {
                    // 获取 Overlapped I/O 的结果
                    fSuccess = GetOverlappedResult(
                        hPipe,     // pipe 句柄
                        &oConnect, // OVERLAPPED 结构
                        &cbRet,    // 已经传送的数据量
                        FALSE);    // 不等待
                    if (!fSuccess) {
                        printf("ConnectNamedPipe (%d)\n", GetLastError());
                        return 0;
                    }
                }

                // 分配内存
                lpPipeInst = (LPPIPEINST)HeapAlloc(GetProcessHeap(), 0, sizeof(PIPEINST));
                if (lpPipeInst == NULL) {
                    printf("GlobalAlloc failed (%d)\n", GetLastError());
                    return 0;
                }
                lpPipeInst->hPipeInst = hPipe;

                // 读和写, 注意CompletedWriteRoutine和CompletedReadRoutine的相互调用
                lpPipeInst->cbToWrite = 0;
                CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst);

                // 再创建一个连接实例, 以响应下一个客户端的连接
                fPendingIO = CreateAndConnectInstance(&oConnect);
                break;

                // 读写完成 
            case WAIT_IO_COMPLETION:
                break;

            default: {
                printf("WaitForSingleObjectEx (%d)\n", GetLastError());
                return 0;
            }
            }
        }
    }

    return 0;
}

VOID WINAPI CompletedWriteRoutine(
    DWORD dwErr,
    DWORD cbWritten,
    LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fRead = FALSE;
    // 保存overlap实例
    lpPipeInst = (LPPIPEINST)lpOverLap;

    // 如果没有错误
    if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite)) {
        fRead = ReadFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chRequest,
            CMD_BUF_SIZE * sizeof(TCHAR),
            (LPOVERLAPPED)lpPipeInst,
            // 写读操作完成后, 调用CompletedReadRoutine
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);
    }

    if (!fRead) {
        // 出错, 断开连接
        printf("ReadFileEx fails (%ld)\n", GetLastError());
        DisconnectAndClose(lpPipeInst);
    }
}

/* ************************************
* CompletedReadRoutine
*     读取pipe操作的完成函数
*    接口参见FileIOCompletionRoutine回调函数定义
*
*    当读操作完成时被调用，写入回复
**************************************/
VOID WINAPI CompletedReadRoutine(
    DWORD dwErr,
    DWORD cbBytesRead,
    LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fWrite = FALSE;

    // 保存overlap实例
    lpPipeInst = (LPPIPEINST)lpOverLap;

    // 如果没有错误
    if ((dwErr == 0) && (cbBytesRead != 0))
    {
        // 根据客户端的请求，生成回复
        GetAnswerToRequest(lpPipeInst);

//         DWORD avail = 0;
//         PeekNamedPipe(lpPipeInst->hPipeInst);
        // 将回复写入到pipe
        fWrite = WriteFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chReply,    //将响应写入pipe
            lpPipeInst->cbToWrite,
            (LPOVERLAPPED)lpPipeInst,
            // 写入完成后，调用CompletedWriteRoutine
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
    }

    if (!fWrite)
        // 出错，断开连接
        DisconnectAndClose(lpPipeInst);
}

/* ************************************
* VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
* 功能    断开一个连接的实例
* 参数    lpPipeInst，断开并关闭的实例句柄
**************************************/
VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
    // 关闭连接实例
    if (!DisconnectNamedPipe(lpPipeInst->hPipeInst))
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }
    // 关闭 pipe 实例的句柄 
    CloseHandle(lpPipeInst->hPipeInst);
    // 释放
    if (lpPipeInst != NULL)
        HeapFree(GetProcessHeap(), 0, lpPipeInst);
}

/* ************************************
* BOOL CreateAndConnectInstance(LPOVERLAPPED lpoOverlap)
* 功能    建立连接实例
* 参数    lpoOverlap，用于overlapped IO的结构
* 返回值    是否成功
**************************************/
BOOL CreateAndConnectInstance(LPOVERLAPPED lpoOverlap)
{
    std::string path;
    if (!MC::GetMoudulePath(path))
        return FALSE;

    std::string xml_path = path + "server.xml";
    std::string type;
    std::string name;
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::xml_parser::read_xml(xml_path, pt);
        type = pt.get<std::string>("con.type");
        name = pt.get<std::string>("con.name");
    } catch (...) {
        boost::exception_ptr e = boost::current_exception();
        std::cout << boost::current_exception_diagnostic_information();
        //printf("read_xml fails\n");
        system("pause");
        return FALSE;
    }

    char cnn_name[1024] = { 0 };
    sprintf_s(cnn_name, "\\\\.\\pipe\\%s", name.c_str());
    LPTSTR lpszPipename = TEXT(cnn_name);
    // 创建named pipe     
    hPipe = CreateNamedPipe(
        lpszPipename,             // pipe名 
        PIPE_ACCESS_DUPLEX |      // 可读可写
        FILE_FLAG_OVERLAPPED,     // overlapped 模式
        // pipe模式
        PIPE_TYPE_BYTE |       // 消息类型pipe
        PIPE_READMODE_BYTE |   // 消息读模式
        PIPE_WAIT,                // 阻塞模式
        PIPE_UNLIMITED_INSTANCES, // 无限制实例
        PIPE_MAX_BUF * sizeof(TCHAR),  // 输出缓存大小
        PIPE_MAX_BUF * sizeof(TCHAR),  // 输入缓存大小
        PIPE_TIMEOUT,             // 客户端超时
        NULL);                    // 默认安全属性
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    Log::WriteLog(LL_DEBUG, "CreateAndConnectInstance->CreateNamedPipe, hPipe: %d, err: %d",
        (int)hPipe,
        GetLastError());

    // 连接到新的客户端
    return ConnectToNewClient(hPipe, lpoOverlap);
}

/* ************************************
* BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
* 功能    建立连接实例
* 参数    lpoOverlap，用于overlapped IO的结构
* 返回值    是否成功
**************************************/
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // 开始一个 overlapped 连接, 由于lpo非空, 调用后会立即返回;
    // 此时, 如管道尚未连接, 客户同管道连接时就会触发lpOverlapped结构中的事件对象.
    // 随后, 可用一个等待函数来监视连接.
    fConnected = ConnectNamedPipe(hPipe, lpo);
    if (fConnected) {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    printf("成功接受客户端连接, err:%d\n", GetLastError());
    switch (GetLastError()) {
        // overlapped连接进行中.
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;
        // 已经连接，因此Event未置位 
    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;
        // error
    default:
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }

    return fPendingIO;
}

// TODO根据客户端的请求，给出响应
VOID GetAnswerToRequest(LPPIPEINST pipe)
{
/*    _tprintf(TEXT("[%d] %s\n"), pipe->hPipeInst, pipe->chRequest);*/
    RecvMsg* msg = new (std::nothrow) RecvMsg();
    if (NULL == msg)
        return;

    msg->pipe_inst = pipe;
    memcpy(msg->msg, pipe->chRequest, sizeof(msg->msg));
    g_recver.Insert(msg);
    g_recver.Signal();
}
