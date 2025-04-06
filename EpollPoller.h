#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "Timestamp.h"

namespace mymuduo
{
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    // 重写Poller的纯虚函数
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static constexpr int kInitEventListSize = 16; //vector<channel*>初始元素数量

    //填写活跃的连接
    void fillActiveChannels(int numEvent, ChannelList *activeChannels) const;
    // 更新channel
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int m_epollfd;         //epoll实例
    EventList m_events;    //活跃事件列表  通过epoll_wait返回
};
} // namespace mymuduo

/**
 * epoll的使用  
 * epoll_create
 * epoll_ctl   add/mod/del
 * epoll_wait
 */ 