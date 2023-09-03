#include "god/http/CacheMap.h"
#include "god/net/EventLoop.h"
#include "god/net/EventLoopThread.h"
#include "god/utils/Logger.h"
#include <unistd.h>

using namespace god;

int main()
{
    EventLoopThread thLoop("ioLoop");
    thLoop.start();

    CacheMap<std::string, std::string> map(thLoop.getLoop());

    for (int i = 0; i != 100; ++i)
    {
        map.insert("xiao is " + std::to_string(i), "long", i);
    }

    thLoop.getLoop()->runOnce(10, []{
        EventLoop::GetLoop()->quit();
    });

    sleep(100);
}