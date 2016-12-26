#pragma once
#include <direct.h>
#include <time.h>
#include <assert.h>
#include <io.h>

#ifdef _DEBUG
#include <base/base_pub.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#endif

// class Lock  
// {  
// private:         
// 	CriticalSection m_cs;  
// public:  
// 	Lock(CriticalSection  cs) : m_cs(cs)  
// 	{  
// 		m_cs.Lock();  		
// 	}  
// 	~Lock()  
// 	{  
// 		m_cs.Unlock();  
// 	}  
// }; 

class CLog
{
private:
	CLog(void){
		m_bLog = false;
		m_strDirPath.clear();
	}
	
	class CGarbo   //它的唯一工作就是在析构函数中删除CSingleton的实例  
	{  
	public:  
		~CGarbo()  
		{  
			if(CLog::m_pInstance)  
				delete CLog::m_pInstance;  
		}  
	};  
	static CGarbo Garbo;
private:
	bool m_bLog;
	string m_strDirPath;
	static CLog* m_pInstance;	
public:	
	static CLog* sharedLog(void);	

	string GetCurrentModulePath();
	bool CreateMultiLevelDirectory(const char *strFilePathName);
	void DeleteHistoryLog(const char* filepath);

	//日志记录
	void WriteLog(const char* szLogFileName, const char* szLog);

    void WriteLog(int level, const char* szLog);

	// 日志调试记录
	void WriteDebugLog(const char *fmt, ...);

	// 记录一般日志
	void WriteNormalLog(const char *fmt, ...);

	//记录USB调试信息
	void CLog::WriteUSBDebugLog(unsigned char *pData, int length);

	void WriteUSBdata(const char *szMsg, unsigned char *pData, int length );

	// 记录上层程序日志
	void WriteAppLog(const char* szLogFileName, const char* szLog);
};

