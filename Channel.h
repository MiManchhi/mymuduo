#include <memory>
#include <functional>

#include "noncopyable.h"
#include "Timestamp.h"

namespace mymuduo
{
class EventLoop;
/**
 * EventLoop里有多个Channel（ChannelList），和一个Poller
 * Channel理解为通道，封装了sockfd和其感兴趣的事件，如EPOLLIN、EPOLLOUT
 * 还绑定了poller返回的具体事件
 */
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    explicit Channel(EventLoop *loop, int fd);
    ~Channel();

    //fd得到poller通知以后，处理事件
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { m_readCallback = std::move(cb); }
    void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }
    void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }
    void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

    // 防止当channel被手动remove掉，channel还在执行回调操作
    //一个TcpConnection新连接创建的时候TcpConnection => channel
    void tie(const std::shared_ptr<void> &ptr);

    int fd() const { return m_fd; }
    int events() const { return m_events; }
    void setRevents(int revt) { m_revents = revt; }

    // 设置fd相应的事件状态
    void enableReading() { m_events |= kReadEvent;
        update();
    }
    void disableReading() { m_events & ~kReadEvent;
        update();
    }
    void enableWriting() { m_events |= kWriteEvent;
        update();
    }
    void disableWriting() { m_events &= ~kWriteEvent;
        update();
    }
    void disableAll() { m_events = kNoneEvent;
        update();
    }

    //返回fd当前的事件状态
    bool isNoneEvent() const { return m_events == kNoneEvent; }
    bool isWriting() const { return m_events & kWriteEvent; }
    bool isReading() const { return m_events & kReadEvent; }

    int index() { return m_index; }
    void setIndex(int idx) { m_index = idx; }

    EventLoop *ownerLoop() { return m_loop; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_loop;     // 事件循环
    const int m_fd;        // fd，poller监听的对象
    int m_events;          // 注册fd感兴趣的事件
    int m_revents;         // poller返回的具体发生的事件
    int m_index;           // 是否绑定到POller的标志位

    std::weak_ptr<void> m_tie;
    bool m_tied;

    //因为channel能够获知fd最终发生的具体的事件，所以它负责调用具体的回调操作
    ReadEventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;
};

} //namespace mymuduo