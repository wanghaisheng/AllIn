#include "stdafx.h"
#include "Log.h"
#include <winbase.h>
#include "SealLog.h"

time_t systime_to_timet(const SYSTEMTIME& st)
{
	struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth-1, st.wYear-1900, st.wDayOfWeek, 0, 0};
	return mktime(&gm);
}

inline HMODULE ModuleHandleByAddr(const void* ptrAddr)                            
{                                                                                 
    MEMORY_BASIC_INFORMATION info;                                                
    ::VirtualQuery(ptrAddr, &info, sizeof(info));                                 
    return (HMODULE)info.AllocationBase;                                          
}                                                                                 
/*                                                                                
     获取当前模块句柄                                                                  
*/                                                                                
inline HMODULE ThisModuleHandle()                                                 
{                                                                                 
    static HMODULE sInstance = ModuleHandleByAddr((void*)&ThisModuleHandle);      
    return sInstance;                                                             
}

//得到当前DLL目录
string CLog::GetCurrentModulePath()
{
  if(m_strDirPath.empty())
  {
    TCHAR szFileName[FILENAME_MAX] = { 0 };
    ::GetModuleFileName(ThisModuleHandle(), szFileName, FILENAME_MAX);
    string strPath = szFileName;

    size_t stPathPos = strPath.find_last_of('\\');
    m_strDirPath = strPath.substr(0, ++stPathPos);
  }

  return m_strDirPath;
}

bool CLog::CreateMultiLevelDirectory(const char *strFilePathName)
{
	char strFilePath[260];
	char *s, *p;
	strcpy_s(strFilePath, sizeof(strFilePath), strFilePathName);
	s = strFilePath;
	// if strFilePathName is network path, skip the ip address/host name
	// Modified on 20090623
	if(0 == strncmp(s, "\\\\", 2))
	{
		s += 2;
		s = strchr(s, '\\');
		if(!s)
		{
			return false;
		}
		else
		{
			s += 1;
		}
	}

	do
	{
		p = strchr(s, '\\');
		if(p)
		{
			*p = '\0';
		}
		s = strFilePath;
		// directory doesn't exist
		if(-1 == _access(s, 0))
		{
			// failed to create directory.
			if(-1 == _mkdir(s))
			{
				return false;
			}
		}

		if(p)
		{
			*p = '\\';
			s = p + 1;
		}
	} while(p);

	return true;
}

void CLog::DeleteHistoryLog(const char* filepath)
{
	char szFileName[MAX_PATH]="";//查找的根目录
	strcpy_s(szFileName,filepath);
	strcat_s(szFileName, "*.*");
	WIN32_FIND_DATA findData;
	HANDLE hFindFile;
	FILETIME ftime;
	SYSTEMTIME stime;
	time_t ltime;

	hFindFile = ::FindFirstFile(szFileName, &findData);
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{			
			ftime=findData.ftCreationTime;
			//FileTimeToLocalFileTime(&ftime,&ltime);			
			FileTimeToSystemTime(&ftime,&stime);
			ltime=systime_to_timet(stime);
			time_t curtime=time(NULL);
			int dvalue = (int)difftime(curtime,ltime);

			char temp[200]="";
			char temp1[100]="";			
			sprintf_s(temp,sizeof(temp),"%s",findData.cFileName);
			sprintf_s(temp1,sizeof(temp1),"%d-%d-%d %d:%d:%d",stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond);
			if (dvalue>=60*60*24*30)
			{
				char  filename[MAX_PATH]="";
				strcpy_s(filename, MAX_PATH, filepath);
				strcat_s(filename,temp);
				DeleteFile(filename);

			}
			if(findData.cFileName[0]=='.')
				continue;
			if(findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				printf("%s\n",findData.cFileName);
		}while(::FindNextFile(hFindFile,&findData));

		FindClose(hFindFile);
	}
}

CLog* CLog::m_pInstance = NULL;	

CLog* CLog::sharedLog(void)
{	
    if (m_pInstance == NULL) {
#ifdef _DEBUG
        boost::log::add_common_attributes();
        boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::trace);

        /* log formatter:
        * [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
        */
        auto fmtTimeStamp = boost::log::expressions::
            format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
        auto fmtThreadId = boost::log::expressions::
            attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");
        auto fmtSeverity = boost::log::expressions::
            attr<boost::log::trivial::severity_level>("Severity");
        auto fmtScope = boost::log::expressions::format_named_scope("Scope",
            boost::log::keywords::format = "%n(%f:%l)",
            boost::log::keywords::iteration = boost::log::expressions::reverse,
            boost::log::keywords::depth = 2);
        boost::log::formatter logFmt =
            boost::log::expressions::format("[%1%] (%2%) [%3%] [%4%] %5%")
            % fmtTimeStamp % fmtThreadId % fmtSeverity % fmtScope
            % boost::log::expressions::smessage;

        /* fs sink */
        std::string prefix;
        std::string log_file = "FLog/USBControlF60_%Y-%m-%d_%H-%M-%S.%N.log";
        bool err = base::FSUtility::GetMoudulePath(prefix);
        auto fsSink = boost::log::add_file_log(
            boost::log::keywords::file_name = err ? prefix + log_file : log_file,
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::min_free_space = 30 * 1024 * 1024,
            boost::log::keywords::open_mode = std::ios_base::app);
        fsSink->set_formatter(logFmt);
        fsSink->locked_backend()->auto_flush(true);
#endif

        //Lock lock(cs);
        if (m_pInstance == NULL)
        {
            m_pInstance = new CLog();
        }
    }

    return m_pInstance;
}

void CLog::WriteLog(const char* szLogFileName, const char* szLog)
{
    return WriteSealLog(4, szLog);


	/*if (!m_bLog)
	{
	return;
	}*/

	FILE *logfile = NULL;
	char szTemp[MAX_PATH] = {0};

	SYSTEMTIME sys;
	::GetLocalTime( &sys );

	string strPath(GetCurrentModulePath());
	strPath += "FLog";

	// 文件名 = szLogFileName+年+月+日
	sprintf_s(szTemp, sizeof(szTemp), "\\%s%04d%02d%02d.log", szLogFileName, sys.wYear, sys.wMonth, sys.wDay);  
	strPath += szTemp;

	sprintf_s(szTemp, sizeof(szTemp), "%02d:%02d:%02d.%03d ", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);  

	errno_t err = fopen_s(&logfile, strPath.c_str(), "a+");
	if(err == 0)
	{
		string strLog(szTemp);
		strLog += szLog;

		if (!strLog.empty())
		{
			fwrite(strLog.c_str(), sizeof(char), strLog.length(), logfile);
			fwrite("\n", sizeof(char), 1, logfile);
		}

		fclose(logfile);
	}
}

void CLog::WriteLog(int level, const char* szLog)
{
    WriteSealLog(level, szLog);
}

void CLog::WriteDebugLog(const char *fmt, ...)
{
	char buf[1000] = {0};
	char* p = buf;
	va_list args;

	va_start(args, fmt);
	vsprintf_s(p, _countof(buf), fmt, args);
	va_end(args);

	WriteLog(("ControlStamperDebug"), p);
}

void CLog::WriteNormalLog(const char *fmt, ...)
{
	char buf[1000] = {0};
	char* p = buf;
	va_list args;

	va_start(args, fmt);
	vsprintf_s(p, _countof(buf), fmt, args);
	va_end(args);

	WriteLog(("ControlStamperNormal"), p);
}

void CLog::WriteUSBDebugLog(unsigned char *pData, int length)
{	
	WriteLog(("USBDebug"), (char*)pData);
}

void CLog::WriteUSBdata(const char *szMsg, unsigned char *pData, int length )
{
	string strLogT;
	char szTmp[10] = {0};
	for(int i = 0; i < length; ++i)
	{
		sprintf_s(szTmp, _T("%02X "), pData[i]);
		strLogT += szTmp;
	}

	string strLog(szMsg);
	strLog += _T(":");
	strLog += strLogT;

	WriteLog(("ControlStamperUSBdata"), strLog.c_str());
}

void CLog::WriteAppLog(const char* szLogFileName, const char* szLog)
{
	WriteLog(szLogFileName, szLog);
}