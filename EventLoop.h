#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

namespace mymuduo
{
class Channel;
class Poller;
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();                            //开启事件循环
    void quit();                            //停止事件循环

    Timestamp pollReturnTime() const { return m_pollReturnTime; }

    void runInLoop(Functor cb);             //在当前loop执行cb
    void queueInLoop(Functor cb);           //将cb放入队列，唤醒loop所在的线程执行cb

    void wakeup();                          //唤醒loop所在的线程

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return m_threadID == CurrentThread::tid(); } //当前loop是否在自己的线程

private:
    void handleRead();                      //weak up 消费，读取eventfd数据
    void doPendingFunctors();               //执行回调

    using ChannelList = std::vector<Channel *>;

    std::atomic<bool> m_looping;            //事件循环开始的标志
    std::atomic<bool> m_quit;               //事件循环停止的标志

    const pid_t m_threadID;                 //记录当前loop所属的线程

    Timestamp m_pollReturnTime;             //Poller返回发生事件的channels的时间
    std::unique_ptr<Poller> m_poller;       //eventloop所管理的Poller

    int m_wakeFd;                           //用于主loop和子loop之间的通信
    /*
    * 当主loop获取新用户channel，通过轮询选择一个子loop，
    * 通过该成员来唤醒子loop来处理channel  (eventfd)
    */
    std::unique_ptr<Channel> m_wakeupChannel;

    ChannelList m_activeChannels;           //活跃channel列表

    std::atomic<bool> m_callingPendingFunctors;  //标识loop是否正在执行回调操作
    std::vector<Functor> m_pendingFunctors;      //存出loop需要执行的所有回调操作
    std::mutex m_mutex;                          //保护m_pendingFunctors的线程安全
};
} // namespace mymuduo