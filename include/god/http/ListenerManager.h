#ifndef GOD_HTTP_LISTENERMANAGER_H
#define GOD_HTTP_LISTENERMANAGER_H

#include <cstdint>
#include <vector>
#include <memory>

#include "god/utils/NonCopyable.h"
#include "god/http/HttpServer.h"

namespace god
{

/// 监听器管理
class ListenerManager : NonCopyable
{
public:
    void addListener(const std::string& ip, uint16_t port);

    void createListeners(const HttpAsyncCallback& httpCallback,
                         size_t connectionTimeout,
                         const std::vector<EventLoop*>& ioLoops);
    
    void startListening();
    void stopListening();

private:
    struct ListenerInfo
    {
        ListenerInfo(const std::string& ip, uint16_t port)
        : ip(ip), port(port) { }

        std::string ip;
        uint16_t port;
    };

    std::vector<ListenerInfo> listeners_;
    std::vector<std::shared_ptr<HttpServer>> servers_;
};

} // namespace god

#endif