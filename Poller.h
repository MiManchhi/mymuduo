#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

/**
 * 
 *一个EventLoop有一个Poller和多个Channel
 *一个Poller是一个epoll实例
 *Poller里使用ChannelMap管理被注册的文件描述符
 */

namespace mymuduo
{
class Channel;
class EventLoop;
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    //给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    // 判断参数channel是否在当前的Poller中
    bool hasChannel(Channel *channel) const;

    //EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    //map的key是sockfd  value是所属的channel通道类型   用于快速查找sockfd对应的channel
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap m_channelMap;

private:
    EventLoop *m_ownerLoop;   //Poller所属的事件循环
};
} // namespace mymuduo