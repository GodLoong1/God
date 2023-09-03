#ifndef GOD_NET_EVENTLOOPTHREADPOOL_H
#define GOD_NET_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>

#include "god/utils/NonCopyable.h"
#include "god/net/EventLoopThread.h"

namespace god
{

/// 事件循环线程池
class EventLoopThreadPool : NonCopyable
{
public:
    EventLoopThreadPool(size_t threadNum,
                        const std::string& threadName) noexcept;

    void start() noexcept;
    std::vector<EventLoop*> getLoops() noexcept;

private:
    std::vector<std::unique_ptr<EventLoopThread>> loopVec_;
};

} // namespace god

#endif