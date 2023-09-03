#include "god/utils/Logger.h"

constexpr size_t count = 1000000;

void loggerTest()
{
    GOD_LOG = std::make_unique<god::Logger>();
    GOD_LOG->setPlace(std::make_unique<god::LogFileOut>("LoggerTest"));

    for (size_t i = 0; i != count; ++i)
    {
        LOG_TRACE << "this is " << i + 1;
    }
}

void asyncLoggerTest()
{
    GOD_LOG = std::make_unique<god::AsyncLogger>();
    GOD_LOG->setPlace(std::make_unique<god::LogFileOut>("AsyncLoggerTest"));

    for (size_t i = 0; i != count; ++i)
    {
        LOG_TRACE << "this is " << i + 1;
    }
}

int main()
{
    god::Date start = god::Date::SteadyTime();
    loggerTest();
    god::Date end = god::Date::SteadyTime();

    god::Date start2 = god::Date::SteadyTime();
    asyncLoggerTest();
    god::Date end2 = god::Date::SteadyTime();

    GOD_LOG = std::make_unique<god::Logger>();
    GOD_LOG->setPlace(std::make_unique<god::LogPlace>());
    GOD_LOG->setFormat(std::make_unique<god::LogFormat>("%m\n"));

    LOG_INFO << "Logger: " << (end - start).count() / 1000000 << " ms";
    LOG_INFO << "AsyncLogger: " << (end2 - start2).count() / 1000000 << " ms";
}