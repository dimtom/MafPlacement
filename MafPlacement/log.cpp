#include "log.h"

Log::LogLevel Log::_log_level = LogLevel::All;

void Log::setLogLevel(LogLevel level)
{
    _log_level = level;
}

void Log::trace(const char* msg)
{
    log(LogLevel::Trace, msg);
}

void Log::info(const char* msg)
{
    log(LogLevel::Info, msg);
}

void Log::warning(const char* msg)
{
    log(LogLevel::Info, msg);
}

void Log::error(const char* msg)
{
    log(LogLevel::Error, msg);
}

void Log::log(LogLevel log_level, const char* msg)
{
    if (log_level > _log_level)
        return;

    printf("%s\n", msg);
}


