#ifndef GOD_UTILS_NONCOPYABLE_H
#define GOD_UTILS_NONCOPYABLE_H

namespace god
{

/// 禁止拷贝
class NonCopyable
{
public:
    NonCopyable() noexcept = default;
    ~NonCopyable() noexcept = default;

    NonCopyable(const NonCopyable&) noexcept = delete;
    NonCopyable& operator=(const NonCopyable&) noexcept = delete;
};

} // namespace god

#endif