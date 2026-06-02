#ifndef MINI_MUDUO_BASE_LOGGER_H
#define MINI_MUDUO_BASE_LOGGER_H

#include <string>

namespace mini_muduo {

// 日志级别。当前阶段只提供同步控制台输出，后续再扩展异步日志。
enum LogLevel {
    DEBUG,
    INFO,
    ERROR,
    FATAL
};

class Logger {
public:
    // 输出一条日志。file 和 line 由日志宏自动传入，方便定位调用位置。
    static void log(LogLevel level, const char* file, int line, const std::string& message);
};

}  // namespace mini_muduo

// 日志宏：自动记录调用日志的位置。
#define LOG_DEBUG(message) mini_muduo::Logger::log(mini_muduo::DEBUG, __FILE__, __LINE__, (message))
#define LOG_INFO(message) mini_muduo::Logger::log(mini_muduo::INFO, __FILE__, __LINE__, (message))
#define LOG_ERROR(message) mini_muduo::Logger::log(mini_muduo::ERROR, __FILE__, __LINE__, (message))
#define LOG_FATAL(message) mini_muduo::Logger::log(mini_muduo::FATAL, __FILE__, __LINE__, (message))

#endif  // MINI_MUDUO_BASE_LOGGER_H
