#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include "log.h"
#include "parse.h"
#include "recver.h"
#include "pipe_server.h"

extern HANDLE hPipe;

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
    std::string name = MC::SvrConfig::GetInst()->name_;
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
    Recver::Insert(msg);
    Recver::Signal();
}
