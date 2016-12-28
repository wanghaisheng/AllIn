/*
**	@file	SealLog.h
**	用于日志输出
**	@version 1.0 
**	@date  
**	@author 
**
**
*/
#ifndef _SEAL_LOG_H
#define _SEAL_LOG_H

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef  USBSEALLOG_EXPORTS
#define USBSEALLOG_API __declspec(dllexport)
#else
#define USBSEALLOG_API __declspec(dllimport)
#endif

/**
* 函数:WriteSealLog	
* 功能:写日志文件
* 
* 参数:	
* @parm int level 日志级别(0不写 1提示 2警告 3错误 4调试)
* @parm onst char* szLog 日志内容
* 返回值   无 		
*/
USBSEALLOG_API  void WriteSealLog(int level, const char* szLog);
/**
* 函数:SetLogRoot
* 功能:设置日志的根目录

* 参数
* @param const char* szLogPath 日志文件的根目录
* @return 无
*/
USBSEALLOG_API  void SetLogRoot(const char* szLogPath);
/**
* 函数:SetLogLevel
* 功能:设置日志级别

* 参数
* @param int level(0不写 1提示 2警告 3错误 4调试)
* @return 无
*/
USBSEALLOG_API  void SetLogLevel(int level);
#ifdef __cplusplus
}
#endif
#endif