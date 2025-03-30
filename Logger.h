#pragma once

#include <string>

#include"noncopyable.h"

namespace mymuduo
{
#define LOG_INFO(logmsgFormat, ...)                   \
do                                                    \
{                                                     \
    Logger &logger = Logger::getLogger();             \
    logger.setLoglevel(INFO);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
}while (0)

#define LOG_WARN(logmsgFormat, ...)                   \
do                                                    \
{                                                     \
    Logger &logger = Logger::getLogger();             \
    logger.setLoglevel(WARN);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
}while (0)

#define LOG_ERROR(logmsgFormat, ...)                  \
do                                                    \
{                                                     \
    Logger &logger = Logger::getLogger();             \
    logger.setLoglevel(ERROR);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
}while (0)

#define LOG_FATAL(logmsgFormat, ...)                  \
do                                                    \
{                                                     \
    Logger &logger = Logger::getLogger();             \
    logger.setLoglevel(FATAL);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
    exit(-1);                                         \
}while (0)

#ifdef MYDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                  \
do                                                    \
{                                                     \
    Logger &logger = Logger::getLogger();             \
    logger.setLoglevel(DEBUG);                        \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
} while (0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif


// 日志级别
enum Loglevel
{
    INFO,  // 普通信息
    WARN,  // 警告
    ERROR, // 错误
    FATAL, // 致命错误
    DEBUG  // 调试
};

class Logger : noncopyable
{
public:
    // 获取日志唯一实例
    static Logger &getLogger();
    // 设置日志级别
    void setLoglevel(int level = ERROR);
    // 写日志到终端
    void log(std::string msg);

private:
    int m_loglevel;
};
}