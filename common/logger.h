/*
 * Author: number
 * Date  : Nov 3, 2015
 */

#ifndef __COMMON_LOGGER_H_H___
#define __COMMON_LOGGER_H_H___

#include <stdarg.h>

#include "shift_writer.h"

namespace ef {

enum LogLevel {
    kLevelFatal    = 0,
    kLevelKey      = 1,
    kLevelErr      = 2,
    kLevelWarn     = 3,
    kLevelInfo     = 4,
    kLevelDebug    = 5,
    kLevelFrame    = 6,
};

class Logger {
public:
    Logger(const char *file, int line);
    virtual ~Logger();

    static bool Initialize(const char * path, const uint32_t max_file_size,
                          const uint32_t max_file_num, const LogLevel level);
    static bool Initialize(const char * path, const uint32_t max_file_size,
                          const uint32_t max_file_num, const char * level);
    static void SetLogLevel(LogLevel log_level);

    static void SetLogFilter(LogLevel filter);
    void Fatal(const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Err  (const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Key  (const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Warn (const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Info (const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Debug(const char * format, ...) __attribute__ ((format (printf, 2, 3)));
    void Frame(const char * format, ...) __attribute__ ((format (printf, 2, 3)));

    static const std::string & GetErrMsg();

private:
    static LogLevel GetLevel(const char * level);
    void Log(const LogLevel level, const char * format, va_list va);

private:
    const char * _file;
    int _line;
};

#define LogFatal(format, ...)   ef::Logger(__FILE__, __LINE__).Fatal(format, ##__VA_ARGS__)
#define LogErr(format, ...)     ef::Logger(__FILE__, __LINE__).Err(format, ##__VA_ARGS__)
#define LogKey(format, ...)     ef::Logger(__FILE__, __LINE__).Key(format, ##__VA_ARGS__)
#define LogWarn(format, ...)    ef::Logger(__FILE__, __LINE__).Warn(format, ##__VA_ARGS__)
#define LogInfo(format, ...)    ef::Logger(__FILE__, __LINE__).Info(format, ##__VA_ARGS__)
#define LogDebug(format, ...)   ef::Logger(__FILE__, __LINE__).Debug(format, ##__VA_ARGS__)
#define LogFrame(format, ...)   ef::Logger(__FILE__, __LINE__).Frame(format, ##__VA_ARGS__)

} /* namespace ef */

#endif /* __COMMON_LOGGER_H__ */
