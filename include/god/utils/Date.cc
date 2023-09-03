#include "god/utils/Date.h"

#include <ctime>

namespace god
{

Date Date::NowTime(int clockId) noexcept
{
    struct timespec tp;
    ::clock_gettime(clockId, &tp);

    return Date(tp.tv_sec * Date::kNanoPerSecond + tp.tv_nsec);
}

Date Date::SystemTime() noexcept
{
    return NowTime(CLOCK_REALTIME);
}

Date Date::SteadyTime() noexcept
{
    return NowTime(CLOCK_MONOTONIC);
}

std::string Date::toString(const char* fmt) const noexcept
{
    time_t timer = toSeconds();
    struct tm tp;
    ::localtime_r(&timer, &tp);

    char buf[128];
    size_t len = ::strftime(buf, sizeof(buf), fmt, &tp);

    return std::string(buf, len);
}

} // namespace god