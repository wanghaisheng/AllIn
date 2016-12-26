#ifndef  CONTROLLER_LOG_H_
#define CONTROLLER_LOG_H_

#include <boost/thread/thread.hpp>

enum LogLevel {
    LL_DEBUG,       //调试
    LL_MSG,         //消息
    LL_WARN,        //警告
    LL_ERROR        //错误
};

class Log {
public:
    static void WriteLog(LogLevel level, const char * fmt, ...);

private:
    static void init();

private:
    const static int MAX_LOG_LEN = 2048;

    static boost::mutex mtx_;
    static bool g_init;
};

#endif // CONTROLLER_LOG_H_
