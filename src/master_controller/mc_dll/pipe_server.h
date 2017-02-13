#ifndef CONTROLLER_PIPE_SERVER_H_
#define CONTROLLER_PIPE_SERVER_H_

#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include "common_definitions.h"

#define PIPE_TIMEOUT 5000

typedef struct
{
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
    TCHAR chRequest[CMD_BUF_SIZE];
    DWORD cbRead;
    TCHAR chReply[CMD_BUF_SIZE];
    DWORD cbToWrite;
} PIPEINST, *LPPIPEINST;

struct RecvMsg {
    RecvMsg() {
        memset(msg, 0, CMD_BUF_SIZE);
    }

    LPPIPEINST  pipe_inst;
    char        msg[CMD_BUF_SIZE];
};

VOID DisconnectAndClose(LPPIPEINST);
BOOL CreateAndConnectInstance(LPOVERLAPPED);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);
VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED);

#endif // CONTROLLER_PIPE_SERVER_H_
