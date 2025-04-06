#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "Logger.h"

namespace mymuduo
{
//防止一个线程创建多个eventloop
__thread EventLoop *t_loopInThisThread = nullptr;
//默认Poller IO复用接口超时时间
constexpr int kPollTimeMs = 10000;
//创建wakeupfd ，用来唤醒子loop处理新的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        log_fatal << "eventfd error : " << errno;
    }
    return evtfd;
}

EventLoop::EventLoop()
    : m_looping(false)
    , m_quit(false)
    , m_threadID(CurrentThread::tid())
    , m_poller(Poller::newDefaultPoller(this))
    , m_wakeFd(createEventfd())
    , m_wakeupChannel(new Channel(this,m_wakeFd))
    , m_callingPendingFunctors(false)
{
    log_debug << "EventLoop create :" << this << " in thread :" << m_threadID;
    if(t_loopInThisThread)
    {
        log_fatal << "Another EventLoop " << t_loopInThisThread
                  << "exits in this thread " << m_threadID;
    }
    else
    {
        t_loopInThisThread = this;
    }
    //设置wakeupfd的事件类型，以及发生事件后的回调操作
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupchannel的EPOLLIN事件,将wakeupChannel注册到Poller
    m_wakeupChannel->enableReading();
}
EventLoop::~EventLoop()
{
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;
    log_info << "EventLoop " << this << " start looping";
    while (!m_quit)
    {
        m_activeChannels.clear();
        //Poller 监听两类fd，一个是clientfd ，一个是wakefd
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
        //处理活跃IO事件
        for(Channel* channel : m_activeChannels)
        {
            channel->handleEvent(m_pollReturnTime);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        /**
         * IO线程 mainLoop accept fd《=channel subloop
         * mainLoop 事先注册一个回调cb（需要subloop来执行）    wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb操作
         */
        doPendingFunctors();
    }
    log_info << "EventLoop " << this << "stop looping";
    m_looping = false;
}
void EventLoop::quit()
{
    m_quit = true;
    // 如果是在其它线程中，调用的quit   在一个subloop(woker)中，
    // 调用了mainLoop(IO)的quit
    if(!isInLoopThread())
    {
        wakeup(); // 通过 eventfd 写入数据，触发可读事件
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()) //在当前线程直接执行
    {
        cb();
    }
    else  //在其他线程，则加入到任务队列，待其他线程执行cb
    {
        queueInLoop(cb);
    }
}       
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(cb);
    }
    //唤醒相应的需要执行上面的回调操作的loop线程
    if(!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeup(); //唤醒loop所在的线程
    }
}        

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(m_wakeFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        log_error << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    m_poller->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    m_poller->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel *channel)
{
    return m_poller->hasChannel(channel);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(m_wakeFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        log_error << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    for(const auto& functor : functors)
    {
        functor();
    }
    m_callingPendingFunctors = false;
}
} // namespace mymuduo