#include "god/http/ListenerManager.h"

namespace god
{

void ListenerManager::addListener(const std::string& ip, uint16_t port)
{
    listeners_.emplace_back(ip, port);
}

void ListenerManager::createListeners(
    const HttpAsyncCallback& httpCallback,
    size_t connectionTimeout,
    const std::vector<EventLoop*>& ioLoops)
{
    for (size_t i = 0; i != ioLoops.size(); ++i)
    {
        for (const ListenerInfo& listener : listeners_)
        {
            const std::string& ip = listener.ip;
            bool isIpV6 = (ip.find(':') != std::string::npos);

            InetAddress listenAddress(ip, listener.port, isIpV6);
            auto serverPtr = std::make_shared<HttpServer>(ioLoops[i],
                                                          listenAddress,
                                                          "GodLoong");
            serverPtr->setHttpCallback(httpCallback);
            serverPtr->setTimeoutOff(connectionTimeout);
            servers_.push_back(serverPtr);
        }
    }
}

void ListenerManager::startListening()
{
    for (auto& server : servers_)
    {
        server->start();
    }
}

void ListenerManager::stopListening()
{
    for (auto& server : servers_)
    {
        server->stop();
    }
}

} // namespace god