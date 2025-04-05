#pragma once

#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <fstream>
#include <sstream>
#include <memory>
#include <condition_variable>
#include <queue>

#include"noncopyable.h"

namespace mymuduo
{
// 日志级别
enum Loglevel
{
    DEBUG, // 调试信息
    INFO,  // 常规运行信息
    WARN,  // 警告事件（不影响系统正常运行）
    ERROR, // 错误事件（影响单个操作，但系统然可以继续运行）
    FATAL, // 致命错误（系统不可用）
    LEVEL_COUNT //用于内部统计的辅助值
};

/**
 * @brief 异步日志系统核心类（单例模式）
 * 
 * 负责管理日志输出队列和后台写入线程
 * 实现线程安全的日志消息缓冲机制
 */
class AsyncLogger : noncopyable
{
public:
    // 获取日志唯一实例
    static AsyncLogger &instance();

    // 设置日志级别
    void setLoglevel(int level = WARN);

    //获取当前日志级别
    int getLogLevel() const;

    /**
     * @brief 设置输出目标
     * @param filename 空字符串表示输出到控制台，否则写入指定文件
    */
    void setOutput(const std::string &filename = "");

    //添加日志
    void append(const std::string &msg);

    // 启动后台线程
    void start();

    //停止日志系统
    void stop();
private:
    AsyncLogger();
    ~AsyncLogger();

    //后台线程执行函数
    void threadFunc();

private:
    std::atomic<bool> m_running;           //线程运行标志
    std::atomic<int> m_logLevel;           //当前日志级别
    std::queue<std::string> m_buffer;      //日志消息缓冲区
    std::mutex m_mutex;                    //队列访问互斥锁
    std::condition_variable m_cond;        //线程同步条件变量
    std::thread m_thread;                  //后台工作线程
    std::shared_ptr<std::ostream> m_output;//输出流（文件或cout）
};

/**
 * @brief 日志消息构造类（RAII方式自动提交日志）
 * 
 * 在构造函数中收集上下文信息（时间、文件、行号）
 * 析构时根据级别决定是否提交到异步队列
 */
class Logger : noncopyable
{
public:
    Logger(const char *file, int line, int level);
    ~Logger();

    //获取日志内容流
    std::ostringstream &stream();
private:
    //转换日志级别为字符串
    static const char *levelToString(int level);
    // 从完整路径提取文件名
    static const char *basename(const char *path);

private:
    const char *m_file;           //源文件路径
    int m_line;                   //代码行号
    int m_level;             //日志级别
    std::ostringstream m_stream; //消息内容流
};

/*******************************用户接口宏****************************** */
/*
 * 自动捕获__FILE__和__LINE__
 * log_info << "Received " << n << " bytes";
*/
#define log_debug \
if(mymuduo::Loglevel::DEBUG >= mymuduo::AsyncLogger::instance().getLogLevel()) \
    mymuduo::Logger(__FILE__,__LINE__,mymuduo::DEBUG).stream()                                                                                              

#define log_info \
if(mymuduo::Loglevel::INFO >= mymuduo::AsyncLogger::instance().getLogLevel()) \
    mymuduo::Logger(__FILE__,__LINE__,mymuduo::INFO).stream()                                                                                          

#define log_warn \
mymuduo::Logger(__FILE__,__LINE__,mymuduo::WARN).stream()                       

#define log_error \
mymuduo::Logger(__FILE__,__LINE__,mymuduo::ERROR).stream()                      

#define log_fatal \
mymuduo::Logger(__FILE__,__LINE__,mymuduo::FATAL).stream()                      

} // namespace mymuduo