#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <string>
#include <unistd.h>

#include "noncopyable.h"

namespace mymuduo
{
class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void stop();
    void join();

    bool started() const { return m_running; }
    pid_t tid() const { return m_tid; }
    const std::string &name() const { return m_name; }

    static int numCreated() { return m_numCreated; }

private:
    void setDefaultName();

    std::atomic<bool> m_running;
    std::atomic<bool> m_joined;
    std::thread m_thread;
    pid_t m_tid;
    ThreadFunc m_func;
    std::string m_name;
    static std::atomic<int> m_numCreated;
};
} // namespace mymuduo