#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <windows.h>
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
#include "common_definitions.h"
#include "log.h"
#include "SealLog.h"

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
