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

bool Log::g_init = false;
boost::mutex Log::mtx_;

void Log::WriteLog(LogLevel level, const char * fmt, ...)
{
/*    init();*/
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

void Log::init()
{
    boost::lock_guard<boost::mutex> lk(mtx_);
    if (!g_init) {
        /* init boost log
        * 1. Add common attributes
        * 2. set log filter to trace
        */
        boost::log::add_common_attributes();
        boost::log::core::get()->add_global_attribute(
            "Scope",
            boost::log::attributes::named_scope());
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
        // 得到所在进程名
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        std::string::size_type last_backslash = std::string(path).find_last_of("\\");
        std::string process_name = std::string(path).substr(last_backslash + 1, std::string::npos);

        std::string prefix;
        std::string log_file = process_name + ".%Y-%m-%d_%H-%M-%S.log";
        bool err = MC::GetMoudulePath(prefix);
        auto fsSink = boost::log::add_file_log(
            boost::log::keywords::file_name = err ? prefix + log_file : log_file,
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::min_free_space = 30 * 1024 * 1024,
            boost::log::keywords::open_mode = std::ios_base::app);
        fsSink->set_formatter(logFmt);
        fsSink->locked_backend()->auto_flush(true);

        g_init = true;
    }
}
