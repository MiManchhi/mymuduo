#include <iostream>

#include"Logger.h"
#include"Timestamp.h"


// 获取日志唯一实例
mymuduo::Logger &mymuduo::Logger::getLogger()
{
    static mymuduo::Logger log;
    return log;
}

// 设置日志级别
void mymuduo::Logger::setLoglevel(int level)
{
    m_loglevel = level;
}
// 写日志到终端
void mymuduo::Logger::log(std::string msg)
{
    switch (m_loglevel)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case WARN:
        std::cout << "[WARN]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }
    //打印时间和msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}
