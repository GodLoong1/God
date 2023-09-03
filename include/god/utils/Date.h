#ifndef GOD_UTILS_DATE_H
#define GOD_UTILS_DATE_H

#include <cstdint>
#include <string>

namespace god
{

/// 日期
class Date
{
public:
    static constexpr int64_t kNanoPerSecond = 1000 * 1000 * 1000;

    constexpr explicit Date(int64_t nano = 0) noexcept
    : nano_(nano) { }

    constexpr int64_t count() const noexcept
    {
        return nano_;
    }

    constexpr int64_t toSeconds() const noexcept
    {
        return nano_ / kNanoPerSecond;
    }

    constexpr int64_t toNano() const noexcept
    {
        return nano_ % kNanoPerSecond;
    }

    /**
     * @param Y 年
     * @param m 月
     * @param d 日
     * @param H 时
     * @param M 分
     * @param S 秒
     */
    std::string toString(const char* fmt =
                         "%Y-%m-%d %H:%M:%S") const noexcept;

    static Date NowTime(int clockId) noexcept;
    static Date SystemTime() noexcept;
    static Date SteadyTime() noexcept;

private:
    int64_t nano_;
};

constexpr Date operator-(Date lhs, Date rhs) noexcept
{
    return Date(lhs.count() - rhs.count());
}

constexpr Date operator+(Date lhs, double rhs) noexcept
{
    return Date(lhs.count() + rhs * Date::kNanoPerSecond);
}

constexpr bool operator<(Date lhs, Date rhs) noexcept
{
    return lhs.count() < rhs.count();
}

constexpr bool operator>(Date lhs, Date rhs) noexcept
{
    return lhs.count() > rhs.count();
}

constexpr bool operator==(Date lhs, Date rhs) noexcept
{
    return lhs.count() == rhs.count();
}

} // namespace god

#endif