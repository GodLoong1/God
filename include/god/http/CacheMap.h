#ifndef GOD_HTTP_CACHEMAP_H
#define GOD_HTTP_CACHEMAP_H

#include <mutex>
#include <unordered_set>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <cassert>

#include "god/utils/Logger.h"
#include "god/net/EventLoop.h"
#include "god/net/Timer.h"

namespace god
{

/// 缓存键值对
template <typename T1, typename T2>
class CacheMap
{
public:
    class CallbackEntry
    {
    public:
        CallbackEntry(std::function<void()>&& cb)
        : cb_(std::move(cb)) { }

        ~CallbackEntry()
        {
            cb_();
        }

    private:
        std::function<void()> cb_;
    };

    using CallbackEntryPtr = std::shared_ptr<CallbackEntry>;
    using WeakCallbackEntryPtr = std::weak_ptr<CallbackEntry>;

    using CallbackBucket = std::unordered_set<CallbackEntryPtr>;
    using CallbackBucketQueue = std::deque<CallbackBucket>;

    struct MapValue
    {
        explicit MapValue(const T2& value = T2{}, size_t timeout = 0)
        : value(value),
          timeout(timeout)
        { }

        T2 value;
        size_t timeout;
        WeakCallbackEntryPtr weakEntryPtr;
    };

    CacheMap(EventLoop* loop,
             double tickInterval = 1.0,
             size_t wheelsNum = 4,
             size_t bucketsNum = 200)
    : loop_(loop),
      tickInterval_(tickInterval),
      wheelsNum_(wheelsNum),
      bucketsNum_(bucketsNum)
    {
        assert(tickInterval_ > 0);
        assert(wheelsNum_ > 0);
        assert(bucketsNum_ > 0);

        wheels_.resize(wheelsNum_);
        for (CallbackBucketQueue& wheel : wheels_)
        {
            wheel.resize(bucketsNum);
        }

        timerId_ = loop_->runEvery(tickInterval_, [this] {
            size_t t = ++ticksCounter_;
            size_t pow = 1;
            for (CallbackBucketQueue& buckets : wheels_)
            {
                if ((t % pow) == 0)
                {
                    CallbackBucket tmp;
                    {
                        std::lock_guard<std::mutex> lock(wheelsMutex_);
                        buckets.front().swap(tmp);
                        buckets.pop_front();
                        buckets.emplace_back();
                    }
                }
                pow = pow * bucketsNum_;
            }
        });

        LOG_TRACE << getThreadName() << ": CacheMap: interval: "
                  << tickInterval_ << ", wheelsNum: " << wheelsNum_
                  << ", bucketNum: " << bucketsNum_;
    }

    ~CacheMap()
    {
        loop_->cancelTimer(timerId_);
        map_.clear();
        for (auto iter = wheels_.rbegin(); iter != wheels_.rend(); ++iter)
        {
            iter->clear();
        }
        LOG_TRACE << getThreadName() << ": ~CacheMap";
    }

    EventLoop* getLoop()
    {
        return loop_;
    }

    void insert(const T1& key,
                const T2& value,
                size_t timeout = 0)
    {
        MapValue v(value, timeout);
        std::lock_guard<std::mutex> lock(mapMutex_);

        map_.emplace(key, std::move(v));
        if (timeout > 0)
        {
            eraseAfter(timeout, key);
        }
        LOG_TRACE << "!!!!!!!!!!!!!!!! Insert " << key;
    }

    void erase(const T1& key)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        map_.erase(key);
    }

    std::optional<T2> find(const T1& key)
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (auto iter = map_.find(key); iter != map_.end())
        {
            size_t timeout = iter->second.timeout;
            if (timeout > 0)
                eraseAfter(timeout, key);
            return iter->second.value;
        }
        return std::nullopt;
    }

private:
    void eraseAfter(size_t delay, const T1& key)
    {
        assert(delay > 0);
        assert(map_.find(key) != map_.end());

        CallbackEntryPtr entryPtr = map_[key].weakEntryPtr.lock();
        if (entryPtr)
        {
            LOG_TRACE << "!!!!!!!!!!!!!!!! Update " << key;
            insertEntry(delay, entryPtr);
        }
        else
        {
            entryPtr = std::make_shared<CallbackEntry>([this, key] {
                std::lock_guard<std::mutex> lock(mapMutex_);
                if (auto iter = map_.find(key); iter != map_.end())
                {
                    map_.erase(key);
                    LOG_TRACE << "!!!!!!!!!!!!!!!! Erase " << key;
                }
            });

            map_[key].weakEntryPtr = entryPtr;
            insertEntry(delay, entryPtr);
        }
    }

    void insertEntry(size_t delay, CallbackEntryPtr entryPtr)
    {
        std::lock_guard<std::mutex> lock(wheelsMutex_);

        assert(delay > 0);

        delay = static_cast<size_t>(delay / tickInterval_ + 1);
        size_t t = ticksCounter_;

        for (size_t i = 0; i != wheelsNum_; ++i)
        {
            if (delay <= bucketsNum_)
            {
                wheels_[i][delay - 1].insert(entryPtr);
                break;
            }
            if (i < (wheelsNum_ - 1))
            {
                entryPtr = std::make_shared<CallbackEntry>(
                    [this, delay, i, t, entryPtr] {
                        assert(delay > 0);
                        std::lock_guard<std::mutex> lock(wheelsMutex_);
                        wheels_[i][(delay + (t % bucketsNum_) - 1) %
                                   bucketsNum_].insert(entryPtr);
                });
            }
            else
            {
                wheels_[i][bucketsNum_ - 1].insert(entryPtr);
            }

            delay = (delay + (t % bucketsNum_) - 1) / bucketsNum_;
            t = t / bucketsNum_;
        }
    }

private:
    EventLoop* loop_;
    double tickInterval_;
    size_t wheelsNum_;
    size_t bucketsNum_;

    std::vector<CallbackBucketQueue> wheels_;
    std::mutex wheelsMutex_;

    std::unordered_map<T1, MapValue> map_;
    std::mutex mapMutex_;

    TimerId timerId_{0};
    std::atomic<size_t> ticksCounter_{0};
};

} // namespace god

#endif