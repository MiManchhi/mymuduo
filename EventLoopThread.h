#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "noncopyable.h"
#include "Thread.h"

namespace mymuduo
{
class EventLoop;

//one loop per thread
//EventLoopThread 组合了 EventLoop 和 Thread 将其绑定
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    //开始事件循环
    EventLoop *startLoop();

private:
    void threadFunc();                  //这里执行Poller->poll,此函数绑定到线程函数

    EventLoop *m_loop;                  //事件循环核心
    Thread m_thread;                    //线程类
    bool m_exiting;                     //线程是否退出
    std::mutex m_mutex;                 
    std::condition_variable m_cond;     //条件变量，同步主线程和eventloop线程
    ThreadInitCallback m_callback;      //初始事件回调
};
} // namespace mymuduo