#include <semaphore.h>

#include "Thread.h"
#include "CurrentThread.h"

namespace mymuduo
{
std::atomic<int> Thread::m_numCreated = 0;
Thread::Thread(ThreadFunc func, const std::string &name)
    : m_running(false)
    , m_joined(false)
    , m_tid(0)
    , m_func(std::move(func))
    , m_name(name)
{
    setDefaultName();
}
Thread::~Thread()
{
    if(m_running)
    {
        if(!m_joined)
        {
            m_thread.detach();
        }
    }
}

void Thread::start()
{
    m_running = true;
    sem_t sem;
    sem_init(&sem, false, 0);
/////////////进入线程//////////////////////
    m_thread = std::thread([&]()
                           {
        m_tid = CurrentThread::tid();
        sem_post(&sem); 
        m_func(); });
/////////////线程结束//////////////////////
    //主线程阻塞等待子线程初始化
    sem_wait(&sem);
    sem_destroy(&sem);
}
void Thread::stop()
{
    m_running = false;
    if(m_thread.joinable())
    {
        if(!m_joined)
        {
            m_thread.join();
            m_joined = true;
        }
    }
}
void Thread::join()
{
    if(m_thread.joinable() && !m_joined) 
    {
        m_thread.join();
        m_joined = true;
    }
}

void Thread::setDefaultName()
{
    int num = ++m_numCreated;
    if(m_name.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        m_name = buf;
    }
}
} // namespace mymuduo