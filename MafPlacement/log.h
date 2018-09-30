#pragma once
#include <string>

class Log
{
public:
    enum class LogLevel : uint8_t {
        None        = 0,
        Error       = 10,
        Warning     = 20,
        Info        = 30,
        Trace       = 40,
        All         = 0xff,
    };
    static void setLogLevel(LogLevel level);

public:
    static void trace(const char* msg);
    static void info(const char* msg);
    static void warning(const char* msg);
    static void error(const char* msg);
    static void log(LogLevel log_level, const char* msg);

private:
    static LogLevel _log_level;

};

#define LOG(level, ...) \
    do { \
        char buf[4096]; \
        sprintf_s(buf, __VA_ARGS__); \
        Log::log(level, buf); \
    } while (false)

#define TRACE(...)  LOG(Log::LogLevel::Trace, __VA_ARGS__)
#define INFO(...)   LOG(Log::LogLevel::Info, __VA_ARGS__)
#define WARN(...)   LOG(Log::LogLevel::Warn, __VA_ARGS__)
#define ERROR(...)  LOG(Log::LogLevel::Error, __VA_ARGS__)
