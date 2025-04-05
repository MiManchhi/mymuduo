#include "Poller.h"
#include "Channel.h"

namespace mymuduo
{
Poller::Poller(Eventloop *loop) : m_ownerLoop(loop)
{
}

// 判断参数channel是否在当前的Poller中
bool Poller::hasChannel(Channel *channel) const
{
    auto iter = m_channelMap.find(channel->fd());
    return iter != m_channelMap.end() && iter->first == channel->fd();
}

} //namespace mymuduo