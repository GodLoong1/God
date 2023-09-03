#include "god/net/EventLoopThread.h"

#include <emmintrin.h>
#include <sys/prctl.h>

#include <cassert>

namespace god
{

EventLoopThread::~EventLoopThread() noexcept
{
    EventLoop* loopPtr = loop_.load(std::memory_order_acquire);
    if (loopPtr)
    {
        loopPtr->quit();
    }
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void EventLoopThread::start() noexcept
{
    assert(!thread_.joinable());
    thread_ = std::thread([this] {
        ::prctl(PR_SET_NAME, name_.data());

        EventLoop loop;
        loop_.store(&loop, std::memory_order_release);

        loop.loop();
        loop_.store(nullptr, std::memory_order_release);
    });

    while (!loop_.load(std::memory_order_acquire))
    {
        ::_mm_pause();
    }
}

EventLoop* EventLoopThread::getLoop() noexcept
{
    assert(loop_);
    return loop_.load(std::memory_order_acquire);
}

} // namespace god