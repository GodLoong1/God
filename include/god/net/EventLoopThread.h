#ifndef GOD_NET_EVENTLOOPTHREAD_H
#define GOD_NET_EVENTLOOPTHREAD_H

#include <thread>
#include <atomic>
#include <string>

#include "god/utils/NonCopyable.h"
#include "god/net/EventLoop.h"

namespace god
{

/// 事件循环线程
class EventLoopThread : NonCopyable
{
public:
    explicit EventLoopThread(const std::string& name) noexcept
    : name_(name) { }

    // 必须在父线程中析构
    ~EventLoopThread() noexcept;

    void start() noexcept;
    EventLoop* getLoop() noexcept;

private:
    std::atomic<EventLoop*> loop_{nullptr};
    std::thread thread_;
    std::string name_;
};

} // namespace god

#endif