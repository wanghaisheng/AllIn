#ifndef CONTROLLER_LOG_H_
#define CONTROLLER_LOG_H_

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
    const static int MAX_LOG_LEN = 2048;
};

#endif // CONTROLLER_LOG_H_
