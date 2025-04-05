#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

namespace mymuduo
{

//channel未被添加到poller中
constexpr int kNew = -1;      //channel成员m_index就是这个标志，默认-1
//channel已经被添加到poller中
constexpr int kAdded = 1;
//channel从poller中删除
constexpr int kDeleted = 2;

//epoll_cretae
EpollPoller::EpollPoller(Eventloop *loop)
    : Poller(loop)
    , m_epollfd(::epoll_create1(EPOLL_CLOEXEC))
    , m_events(kInitEventListSize)
{
    if(m_epollfd < 0)
    {
        log_fatal << "epoll_create1 error : " << errno;
    }
}
EpollPoller::~EpollPoller()
{
    ::close(m_epollfd);
}

// 重写Poller的纯虚函数
//epoll_wait
Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    log_info << "fd total counts = " << m_channelMap.size();
    int numEvent = epoll_wait(m_epollfd, &*m_events.begin(), static_cast<int>(m_events.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvent > 0)
    {
        log_info << numEvent << " events happened";
        fillActiveChannels(numEvent, activeChannels);
        if(numEvent == m_events.size()) //扩容
        {
            m_events.resize(m_events.size() * 2);
        }
    }
    else if(numEvent == 0)
    {
        log_info << "timeout! nothing happen";
    }
    else //numEvent < 0 ----Error!
    {
        errno = savedErrno;
        log_error << "EpollPoller::poll() error !";
    }
    return now;
}
//epoll_ctl
void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    log_info << "fd = " << channel->fd() << " events = " << channel->events()
             << "index = " << index;
    if(index == kNew || index == kDeleted) //未被注册或已被标记为删除
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            m_channelMap[fd] = channel;
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else //已经被注册
    {
        int fd = channel->fd();
        if(channel->isNoneEvent()) //没有关注的事件了，将其移除
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
//epoll_ctl
void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    m_channelMap.erase(fd);

    log_info << "fd = " << fd << "was delete";

    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kDeleted);
}

//填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvent, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvent; ++i)
    {
        Channel *channel = static_cast<Channel *>(m_events[i].data.ptr);
        channel->setRevents(m_events[i].events);
        activeChannels->push_back(channel);
    }
}
// 更新channel  epoll_ctl/add/mod/del
void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    bzero(&event, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events();  //关注的事件
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(m_epollfd,operation,fd,&event) < 0) //失败
    {
        if(operation == EPOLL_CTL_DEL)
        {
            log_error << "epoll_ctl delete error : " << errno;
        }
        else
        {
            log_fatal << "epoll_ctl add/mod error : " << errno;
        }
    }
}
} //namespace mymuduo