#include "base/Logger.h"

#include <cstdlib>
#include <iostream>

namespace mini_muduo {

namespace {

const char* logLevelName(LogLevel level)
{
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

}  // namespace

void Logger::log(LogLevel level, const char* file, int line, const std::string& message)
{
    // ERROR/FATAL 输出到标准错误，其余级别输出到标准输出。
    std::ostream& stream = (level == ERROR || level == FATAL) ? std::cerr : std::cout;
    stream << "[" << logLevelName(level) << "] "
           << file << ":" << line << " "
           << message << std::endl;

    // FATAL 表示不可恢复错误，输出日志后立即终止进程。
    if (level == FATAL) {
        std::abort();
    }
}

}  // namespace mini_muduo
