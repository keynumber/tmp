/*
 * Author: number
 * Date  : Nov 3, 2015
 */

#include "logger.h"

#include <stdio.h>
#include <string.h>

#include <sys/time.h>

namespace ef {

static LogLevel _log_filter = kLevelDebug;
static ShiftWriter _writer;

static const int kMaxLogSize = 2048;
static const int kMaxPathLength = 256;
static const int kBufSize = kMaxLogSize + kMaxPathLength + 100;

Logger::Logger(const char* file, int line)
    : _file(file)
    , _line(line)
{
}

Logger::~Logger()
{
}

bool Logger::Initialize(const char* path, const int max_file_size,
                       const int max_file_num, const LogLevel level)
{
    _log_filter = level;
    return _writer.Initialize(path, max_file_size, max_file_num, ".log");
}

const std::string & Logger::GetErrMsg()
{
    return _writer.GetErrMsg();
}

bool Logger::Initialize(const char * path, const int max_file_size,
               const int max_file_num, const char * str_level)
{
    LogLevel level = GetLevel(str_level);
    return Initialize(path, max_file_size, max_file_num, level);
}

LogLevel Logger::GetLevel(const char * str_level)
{
    if (std::string("fatal") == str_level) {
        return kLevelFatal;
    } else if (std::string("key") == str_level) {
        return kLevelKey;
    } else if (std::string("error") == str_level) {
        return kLevelErr;
    } else if (std::string("warn") == str_level) {
        return kLevelWarn;
    } else if (std::string("info") == str_level) {
        return kLevelInfo;
    } else if (std::string("debug") == str_level) {
        return kLevelDebug;
    } else if (std::string("Frame") == str_level) {
        return kLevelFrame;
    }
    return kLevelInfo;
}

void Logger::SetLogLevel(LogLevel log_level)
{
    _log_filter = log_level;
}

void Logger::SetLogFilter(LogLevel filter)
{
    _log_filter = filter;
}

void Logger::Log(const LogLevel level, const char* format, va_list va)
{
    if (_log_filter < level) {
        return;
    }

    char buf[kBufSize];

    char * ptr = buf;

    timeval now;
    gettimeofday(&now, nullptr);
    struct tm tm;
    localtime_r(&now.tv_sec, &tm);
    int write_size = snprintf(ptr, static_cast<size_t>(kBufSize-(ptr-buf)), "%02d-%02d %02d:%02d:%02d.%06ld",
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            static_cast<long>(now.tv_usec));
    ptr += write_size;
    // replace 0 with ' '
    *ptr = ' ';
    ++ ptr;

    switch (level) {
        case kLevelFatal: memcpy(ptr, "FATAL ", 6); ptr += 6; break;
        case kLevelKey  : memcpy(ptr, "Key   ", 6); ptr += 6; break;
        case kLevelErr  : memcpy(ptr, "ERROR ", 6); ptr += 6; break;
        case kLevelWarn : memcpy(ptr, "WARN  ", 6); ptr += 6; break;
        case kLevelInfo : memcpy(ptr, "INFO  ", 6); ptr += 6; break;
        case kLevelDebug: memcpy(ptr, "DEBUG ", 6); ptr += 6; break;
        case kLevelFrame: memcpy(ptr, "FRAME ", 6); ptr += 6; break;
        default         : memcpy(ptr, "OTHER ", 6); ptr += 6; break;
    }

    write_size = snprintf(ptr, static_cast<size_t>(kBufSize-(ptr-buf)), "[%s:%d] ", _file, _line);
    if (write_size > kMaxPathLength) {
        write_size = kMaxPathLength;
    }
    ptr += write_size;

    write_size = vsnprintf(ptr, kMaxLogSize, format, va);
    if (write_size > kMaxLogSize) {
        write_size = kMaxLogSize;
    }
    ptr += write_size;
    *ptr = 0;

    _writer.Write(buf, ptr-buf);
}

#define LOGGER_DEF(name)                            \
void Logger::name(const char * format, ...)         \
{                                                   \
    va_list va;                                     \
    va_start(va, format);                           \
    Log(kLevel##name, format, va);                  \
}


LOGGER_DEF(Fatal)
LOGGER_DEF(Key  )
LOGGER_DEF(Err  )
LOGGER_DEF(Warn )
LOGGER_DEF(Info )
LOGGER_DEF(Debug)
LOGGER_DEF(Frame)

} /* namespace ef */
