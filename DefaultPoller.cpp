#include <stdlib.h>

#include "Poller.h"
#include "EpollPoller.h"

namespace mymuduo
{
//EventLoop可以通过该接口获取默认的IO复用的具体实现
Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MYMUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll实例
    }
    else
    {
        return new EpollPoller(loop); // 生成epoll实例
    }
}
} //namespace mymuduo