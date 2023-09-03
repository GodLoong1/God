#include "god/utils/Logger.h"
#include <memory>

using namespace god;

int count = 1000000;

void test1()
{
    GOD_LOG = std::make_unique<Logger>();
    GOD_LOG->setPlace(std::make_unique<LogFileOut>("test1"));

    for (int i = 0; i != count; ++i)
    {
        LOG_INFO << "this is " << i;
    }
}

void test2()
{
    GOD_LOG = std::make_unique<AsyncLogger>();
    GOD_LOG->setPlace(std::make_unique<LogFileOut>("test2"));

    for (int i = 0; i != count; ++i)
    {
        LOG_INFO << "this is " << i;
    }
}

void test3()
{
    GOD_LOG = std::make_unique<AsyncLogger>();
    GOD_LOG->setPlace(std::make_unique<LogFileOut>("test3"));

    for (int i = 0; i != count; ++i)
    {
        LOG_INFO << "this is " << i;
        if (i == count / 2)
        {
            LOG_FATAL << "gg";
        }
    }
}

void test4()
{
    count = 10000;
    GOD_LOG = std::make_unique<AsyncLogger>();
    GOD_LOG->setPlace(std::make_unique<LogFileOut>("test4"));

    std::thread th1([] {
        for (int i = 0; i != count; ++i)
        {
            LOG_DEBUG << "th1: " << i;
        }
    });

    std::thread th2([] {
        for (int i = 0; i != count; ++i)
        {
            LOG_WARN << "th2: " << i;
        }
    });

    for (int i = 0; i != count; ++i)
    {
        LOG_INFO << "this is " << i;
        if (i == count / 2)
        {
            LOG_FATAL << "gg";
        }
    }
    
    th1.join();
    th2.join();
}

int main()
{
    // test4();

    auto start1 = Date::SteadyTime();
    test1();
    auto end1 = Date::SteadyTime();

    auto start2 = Date::SteadyTime();
    test2();
    auto end2 = Date::SteadyTime();

    GOD_LOG = std::make_unique<Logger>();
    GOD_LOG->setFormat(std::make_unique<LogFormat>("[%m]\n"));

    LOG_INFO << "Logger time " << (end1 - start1).count() / 1000000 << " ms";
    LOG_INFO << "AsyncLogger time " << (end2 - start2).count() / 1000000 << " ms";

    // test3();
}
