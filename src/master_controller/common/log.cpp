#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include "common_definitions.h"
#include "SealLog.h"
#include "log.h"

void Log::WriteLog(LogLevel level, const char * fmt, ...)
{
    char buf[MAX_LOG_LEN] = { 0 };
    va_list val;
    va_start(val, fmt);
    _vsnprintf_s(buf, MAX_LOG_LEN, fmt, val);
    va_end(val);

    switch (level) {
    case LL_DEBUG:
        WriteSealLog(4, buf);
        break;
    case LL_MSG:
        WriteSealLog(1, buf);
        break;
    case LL_WARN:
        WriteSealLog(2, buf);
        break;
    case LL_ERROR:
        WriteSealLog(3, buf);
        break;
    default:
        WriteSealLog(4, buf);
        break;
    }
}
