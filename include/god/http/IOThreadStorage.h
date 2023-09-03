#ifndef GOD_HTTP_IOTHREADSTORAGE_H
#define GOD_HTTP_IOTHREADSTORAGE_H

#include "god/utils/NonCopyable.h"
#include "god/http/HttpAppFramework.h"

#include <cassert>
#include <memory>
#include <vector>
#include <functional>

namespace god
{

/// 每个EventLoop都有一份数据以实现线程安全
template <typename T>
class IOThreadStorage : public NonCopyable
{
public:
    using ValueType = T;
    using InitCallback = std::function<void(ValueType& val, size_t index)>;

    template<typename... Args>
    explicit IOThreadStorage(Args&&... args) noexcept
    {
        static_assert(std::is_constructible_v<T, Args&&...>,
                      "Unable to construct storage with given signature");

        size_t numThreads = app().getThreadNum();
        assert(numThreads > 0 && numThreads != EventLoop::nindex);

        storage_.reserve(numThreads + 1);
        for (size_t i = 0; i <= numThreads; ++i)
        {
            storage_.emplace_back(std::forward<Args>(args)...);
        }
    }

    void init(const InitCallback& initCB) noexcept
    {
        for (size_t i = 0; i < storage_.size(); ++i)
        {
            initCB(storage_[i], i);
        }
    }

    ValueType &getThreadData() noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        return storage_[idx];
    }

    const ValueType &getThreadData() const noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        return storage_[idx];
    }

    void setThreadData(const ValueType& newData) noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        storage_[idx] = newData;
    }

    void setThreadData(ValueType&& newData) noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        storage_[idx] = std::move(newData);
    }

    ValueType* operator->() noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        return &storage_[idx];
    }

    const ValueType* operator->() const noexcept
    {
        size_t idx = app().getCurrentThreadIndex();
        assert(idx < storage_.size());
        return &storage_[idx];
    }

    ValueType& operator*() noexcept
    {
        return getThreadData();
    }

    const ValueType& operator*() const noexcept
    {
        return getThreadData();
    }

private:
    std::vector<ValueType> storage_;
};

}  // namespace god

#endif