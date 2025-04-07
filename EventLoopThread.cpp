#include "EventLoopThread.h"
#include "EventLoop.h"

namespace mymuduo
{
EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : m_loop(nullptr)
    , m_thread(std::bind(&EventLoopThread::threadFunc, this), name)
    , m_exiting(false)
    , m_mutex()
    , m_cond()
    , m_callback(cb)
{
}
EventLoopThread::~EventLoopThread()
{
    m_exiting = true;
    if(m_loop != nullptr)
    {
        m_loop->quit();   //退出事件循环
        m_thread.join();  //退出线程
    }
}

//开始事件循环
EventLoop *EventLoopThread::startLoop()
{
    //这里调用了std::bind(&EventLoopThread::threadFunc, this)
    //既EventLoopThread::threadFunc()  （非此线程，需要线程同步）这个函数里在栈上创建了一个eventloop
    //此线程需要等待m_thread线程eventloop创建完成，并保存到m_loop
    m_thread.start(); //线程继续执行，不会阻塞 
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]
                    { return m_loop != nullptr; });
        loop = m_loop;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    //创建一个独立的eventloop，和上面的线程一一对应（one loop per thread）
    EventLoop loop; 

    //在开启事件循环前先执行默认回调
    if(m_callback)
    {
        m_callback(&loop);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }
    loop.loop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}
} // namespace mymuduo