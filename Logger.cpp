#include <iostream>

#include"Logger.h"

namespace mymuduo
{
// 获取日志唯一实例
AsyncLogger &AsyncLogger::instance()
{
    static AsyncLogger logger;
    return logger;
}

// 设置日志级别
void AsyncLogger::setLoglevel(int level)
{
    m_logLevel.store(level, std::memory_order_release);
}

//获取当前日志级别
int AsyncLogger::getLogLevel() const
{
    return m_logLevel.load(std::memory_order_acquire);
}

/**
 * @brief 设置输出目标
 * @param filename 空字符串表示输出到控制台，否则写入指定文件
*/
void AsyncLogger::setOutput(const std::string &filename)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(filename.empty())
    {
        //使用自定义删除器，避免关闭cout
        m_output.reset(&std::cout, [](std::ostream *) {});
    }
    else
    {
        m_output.reset(new std::ofstream(filename, std::ios::app));
        if(!m_output->good())
        {
            throw std::runtime_error("Failed to open log file");
        }
    }
}

//添加日志
void AsyncLogger::append(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_running.load(std::memory_order_acquire))
    {
        m_buffer.push(msg);
        m_cond.notify_one();
    }
}

// 启动后台线程
void AsyncLogger::start()
{
    m_running.store(true, std::memory_order_release);
    m_thread = std::thread(&AsyncLogger::threadFunc, this);
}

//停止日志系统
void AsyncLogger::stop()
{
    m_running.store(false, std::memory_order_release);
    //完成剩余日志后关闭
    m_cond.notify_all();
    if(m_thread.joinable())
    {
        m_thread.join();
    }
}

AsyncLogger::AsyncLogger() : m_running(false), m_logLevel(WARN)
{
    setOutput();
}
AsyncLogger::~AsyncLogger()
{
    stop();
}

//后台线程执行函数
void AsyncLogger::threadFunc()
{
    while (m_running.load(std::memory_order_acquire) || !m_buffer.empty())
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait_for(lock, std::chrono::seconds(1), [this]
                        { return !m_buffer.empty() || !m_running; });
        if(!m_buffer.empty())
        {
            auto msg = std::move(m_buffer.front());
            m_buffer.pop();
            lock.unlock();

            if(m_output)
            {
                *m_output << msg << std::endl;
                m_output->flush();
            }
        }
    }
}



Logger::Logger(const char *file, int line, int level) : m_file(file), m_line(line), m_level(level)
{
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_time;
    localtime_r(&t, &tm_time);

    m_stream << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S")
             << " " << levelToString(m_level) << " "
             << basename(m_file) << ":" << m_line << " ";
}
Logger::~Logger()
{
    if(m_level >= AsyncLogger::instance().getLogLevel())
    {
        m_stream << "\n";
        AsyncLogger::instance().append(m_stream.str());
    }
}

//获取日志内容流
std::ostringstream &Logger::stream()
{
    return m_stream;
}
//转换日志级别为字符串
const char *Logger::levelToString(int level)
{
    static const char *levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    return levelStr[level];
}
// 从完整路径提取文件名
const char *Logger::basename(const char *path)
{
    const char *slash = strrchr(path, '/');
#ifdef _WIN32
    if(!slash)
        slash = strrchr(path, '\\');
#endif
    return slash ? slash + 1 : path;
}
} //namespace mymuduo

// int main()
// {
//     // 1. 初始化日志系统
//     mymuduo::AsyncLogger::instance().setLoglevel(mymuduo::DEBUG);  // 设置日志级别
//     mymuduo::AsyncLogger::instance().setOutput();        // 输出到文件
//     mymuduo::AsyncLogger::instance().start();                     // 启动后台线程

//     // 2. 记录不同级别的日志
//     log_info << "Server started on port " << 8080;  // 输出到文件
//     log_debug << "This message will be filtered";   // 被过滤（级别低于INFO）
//     log_error << "Failed to connect to database: " << "Timeout";

//     // 3. 多线程日志测试
//     std::thread t1([]{
//         log_warn << "Thread 1: Resource usage exceeds 80%";
//     });

//     std::thread t2([]{
//         log_fatal << "Thread 2: Critical error detected";
//     });

//     t1.join();
//     t2.join();

//     // 4. 停止日志系统
//     mymuduo::AsyncLogger::instance().stop();
//     return 0;
// }