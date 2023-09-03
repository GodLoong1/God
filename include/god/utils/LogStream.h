#ifndef GOD_UTILS_LOGSTREAM_H
#define GOD_UTILS_LOGSTREAM_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <charconv>
#include <type_traits>
#include <string>

#include "god/utils/NonCopyable.h"

namespace god
{

/// 日志缓存区
template<size_t N>
class LogBuffer : NonCopyable
{
public:
    LogBuffer() noexcept
    : curr_(data_)
    {
        ::memset(data_, 0, sizeof(data_));
    }

    const char* data() const noexcept
    {
        return data_;
    }

    size_t size() const noexcept
    {
        return curr_ - data_;
    }

    size_t capacity() const noexcept
    {
        return (data_ + N) - curr_;
    }

    void clear() noexcept
    {
        curr_ = data_;
    }

    bool append(const char* buf, size_t len) noexcept
    {
        if (capacity() >= len)
        {
            ::memcpy(curr_, buf, len);
            curr_ += len;
            return true;
        }
        return false;
    }

private:
    char data_[N + 1];
    char* curr_;
};

/// 日志流
class LogStream
{
public:
    static constexpr size_t kInitSize = 1024;

    const char* data() const noexcept
    {
        if (!exBuffer_.empty())
            return exBuffer_.data();
        return buffer_.data();
    }

    size_t size() const noexcept
    {
        if (!exBuffer_.empty())
            return exBuffer_.size();
        return buffer_.size();
    }

    void clear() noexcept
    {
        buffer_.clear();
        exBuffer_.clear();
    }

    void append(const char* buf, size_t len) noexcept
    {
        if (!exBuffer_.empty())
        {
            exBuffer_.append(buf, len); 
        }
        else
        {
            if (!buffer_.append(buf, len))
            {
                exBuffer_.append(buffer_.data(), buffer_.size());
                exBuffer_.append(buf, len);
            }
        }
    }

    LogStream& operator<<(bool val) noexcept
    {
        append(val ? "true" : "false", val ? 4 : 5);
        return *this;
    }

    LogStream& operator<<(char val) noexcept
    {
        append(&val, 1);
        return *this;
    }

    LogStream& operator<<(char* val) noexcept
    {
        return *this << static_cast<const char*>(val);
    }

    LogStream& operator<<(const char* val) noexcept
    {
        if (val)
        {
            append(val, ::strlen(val));
        }
        else
        {
            append("(nullptr)", 9);
        }
        return *this;
    }

    template<typename T, typename = std::enable_if_t<
        std::is_arithmetic_v<T> || std::is_pointer_v<T>>>
    LogStream& operator<<(T val) noexcept
    {
        char buf[64];
        char* p = buf;

        if constexpr (std::is_integral_v<T>)
        {
            p = std::to_chars(p, buf + sizeof(buf), val).ptr;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            p = std::to_chars(p, buf + sizeof(buf), val,
                    std::chars_format::general).ptr;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            p = std::to_chars(p, buf + sizeof(buf),
                    reinterpret_cast<uintptr_t>(val), 16).ptr;
        }

        append(buf, p - buf);
        return *this;
    }

    template<typename T, typename = std::enable_if_t<
        std::is_convertible_v<
            decltype(std::declval<T>().data()), const char*> &&
        std::is_convertible_v<
            decltype(std::declval<T>().size()), size_t>>>
    LogStream& operator<<(const T& val) noexcept
    {
        append(val.data(), val.size());
        return *this;
    }

private:
    LogBuffer<kInitSize> buffer_;
    std::string exBuffer_;
};

} // namespace god

#endif