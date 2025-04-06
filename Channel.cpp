#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

namespace mymuduo
{
//EventLoop : ChannelList Poller
Channel::Channel(EventLoop *loop, int fd)
    : m_loop(loop), m_fd(fd), m_revents(0), m_index(-1), m_tied(false)
{
}
Channel::~Channel()
{
}

//fd得到poller通知以后，处理事件
void Channel::handleEvent(Timestamp receiveTime)
{
    if(m_tied)
    {
        std::shared_ptr<void> guard = m_tie.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 防止当channel被手动remove掉，channel还在执行回调操作
void Channel::tie(const std::shared_ptr<void> &ptr)
{
    m_tie = ptr;
    m_tied = true;
}

//通过channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove()
{
    m_loop->removeChannel(this);
}
void Channel::update()
{
    m_loop->updateChannel(this);
}
//根据poller通知的channel发生的具体事件，有channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    log_info << "channel handleEvent revent" << m_revents;
    if((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN))
    {
        if(m_closeCallback)
        {
            m_closeCallback();
        }
    }

    if(m_revents & EPOLLERR)
    {
        if(m_errorCallback)
        {
            m_errorCallback();
        }
    }

    if(m_revents & (EPOLLIN | EPOLLPRI))
    {
        if(m_readCallback)
        {
            m_readCallback(receiveTime);
        }
    }

    if(m_revents & EPOLLOUT)
    {
        if(m_writeCallback)
        {
            m_writeCallback();
        }
    }
}

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;
} // namespace mymuduo