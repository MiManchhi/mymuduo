#include <stdlib.h>

#include "Poller.h"
#include "EpollPoller.h"

namespace mymuduo
{
//EventLoop可以通过该接口获取默认的IO复用的具体实现
Poller *Poller::newDefaultPoller(Eventloop *loop)
{
    if(::getenv("MYMUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll实例
    }
    else
    {
        return nullptr; // 生成epoll实例
    }
}
} //namespace mymuduo