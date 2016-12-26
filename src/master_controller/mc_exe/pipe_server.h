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

int StartServer();

#endif // CONTROLLER_PIPE_SERVER_H_
