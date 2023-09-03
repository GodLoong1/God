#include "god/net/EventLoopThreadPool.h"

namespace god
{

EventLoopThreadPool::EventLoopThreadPool(
    size_t threadNum,
    const std::string& threadName) noexcept
{
    loopVec_.reserve(threadNum);
    for (size_t i = 0; i != threadNum; ++i)
    {
        loopVec_.emplace_back(new EventLoopThread(threadName));
    }
}

void EventLoopThreadPool::start() noexcept
{
    for (auto& th : loopVec_)
    {
        th->start();
    }
}

std::vector<EventLoop*> EventLoopThreadPool::getLoops() noexcept
{
    std::vector<EventLoop*> ret;
    ret.reserve(loopVec_.size());

    for (const auto& th : loopVec_)
    {
        ret.emplace_back(th->getLoop());
    }
    return ret;
}

} // namespace god