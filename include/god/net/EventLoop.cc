#include "god/net/EventLoop.h"

#include <unistd.h>
#include <sys/eventfd.h>

#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/Channel.h"
#include "god/net/Poller.h"
#include "god/net/TimerHeap.h"

namespace god
{

static thread_local EventLoop* eventLoopPtr{nullptr};

EventLoop::EventLoop() noexcept
: tid_(getThreadId()),
  looping_(false),
  poller_(new Poller(this)),
  wakeFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
  wakeing_(false),
  wakeChannel_(new Channel(this, wakeFd_)),
  timerHeap_(new TimerHeap(this))
{
    LOG_TRACE << getThreadName() << ": fd: " << wakeFd_;

    assert(!eventLoopPtr);
    eventLoopPtr = this;

    wakeChannel_->setReadCallback([this] { handleRead(); });
    wakeChannel_->enableReading();
}

EventLoop::~EventLoop() noexcept
{
    LOG_TRACE << getThreadName() << ": fd: " << wakeFd_;

    eventLoopPtr = nullptr;
    wakeChannel_->disableAll();
}

void EventLoop::loop() noexcept
{
    this->assertInLoop();

    looping_.store(true, std::memory_order_release);
    while (looping_.load(std::memory_order_acquire))
    {
        poller_->wait(readyChannels_);

        for (Channel* channel : readyChannels_)
        {
            // LOG_TRACE << "fd: " << channel->fd() << ", event: "
            //           << channel->eventString() << ", revent: "
            //           << channel->reventString();
            channel->handleEvent();
        }
        readyChannels_.clear();

        {
            std::lock_guard<std::mutex> lock(wakeMutex_);
            wakeTemp_.swap(wakeCallbacks_);
        }
        for (Func& cb : wakeTemp_)
        {
            cb();
        }
        wakeTemp_.clear();
    }
}

void EventLoop::quit() noexcept
{
    looping_.store(false, std::memory_order_release);
    handleWrite();
}

void EventLoop::change(Channel* channel) noexcept
{
    this->assertInLoop();
    assert(channel->getLoop() == this);

    poller_->change(channel);
}

void EventLoop::runInLoop(Func&& cb) noexcept
{
    if (isInLoop())
    {
        cb();
    }
    else
    {
        addInLoop(std::move(cb));
    }
}

void EventLoop::addInLoop(Func&& cb) noexcept
{
    {
        std::lock_guard<std::mutex> lock(wakeMutex_);
        wakeCallbacks_.emplace_back(std::move(cb));
    }
    handleWrite();
}

TimerId EventLoop::runAt(Date when, TimerCallback&& cb) noexcept
{
    return timerHeap_->addTimer(std::move(cb), when, 0.0);
}

TimerId EventLoop::runOnce(double delay, TimerCallback&& cb) noexcept
{
    Date when(Date::SteadyTime() + delay);
    return runAt(when, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback&& cb) noexcept
{
    Date when(Date::SteadyTime() + interval);
    return timerHeap_->addTimer(std::move(cb), when, interval);
}

void EventLoop::cancelTimer(TimerId timerId) noexcept
{
    timerHeap_->cancelTimer(timerId);
}

bool EventLoop::isInLoop() const noexcept
{
    return tid_ == getThreadId();
}

void EventLoop::assertInLoop() const noexcept
{
    if (!isInLoop())
    {
        LOG_FATAL << "create thread: " << tid_
                  << ", current thread: " << getThreadId();
    }
}

EventLoop* EventLoop::GetLoop() noexcept
{
    assert(eventLoopPtr);
    return eventLoopPtr;
}

void EventLoop::handleRead() noexcept
{
    this->assertInLoop();

    uint64_t val = 0;
    if (::read(wakeFd_, &val, sizeof(val)) != sizeof(val))
    {
        LOG_FATAL << strerr();
    }
    wakeing_.clear(std::memory_order_release);
}

void EventLoop::handleWrite() noexcept
{
    if (!wakeing_.test_and_set(std::memory_order_acquire))
    {
        uint64_t val = 1;
        if (::write(wakeFd_, &val, sizeof(val)) != sizeof(val))
        {
            LOG_FATAL << strerr();
        }
    }
}

} // namespace god