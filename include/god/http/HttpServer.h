#ifndef GOD_HTTP_HTTPSERVER_H
#define GOD_HTTP_HTTPSERVER_H

#include "god/http/HttpRequestParser.h"
#include "god/net/EventLoop.h"
#include "god/net/TcpConnection.h"
#include "god/net/TcpServer.h"
#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"
#include "god/http/HttpTypes.h"
#include "god/utils/NonCopyable.h"

namespace god
{

using HttpAsyncCallback =
    std::function<void(const HttpRequestPtr& req,
                       HttpResponseHandler&& respcb)>;

/// Http服务器
class HttpServer : NonCopyable
{
public:
    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string_view& name);
    ~HttpServer();

    void start();
    void stop();

    EventLoop* getLoop() const
    {
        return server_->getLoop();
    }

    void setIoLoopNum(int numThreads) noexcept
    {
        server_->setIoLoopNum(numThreads);
    }

    void setTimeoutOff(size_t timeout) noexcept
    {
        server_->setTimeoutOff(timeout);
    }

    void setHttpCallback(const HttpAsyncCallback& cb)
    {
        httpAsyncCallback_ = cb;
    }

private:
    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, TcpBuffer& buf);

    void onRequest(const TcpConnectionPtr& conn,
                   const HttpRequestParserPtr& parser,
                   const HttpRequestPtr& req);
    
    void onResponse(const TcpConnectionPtr& conn,
                    const HttpRequestParserPtr& parser,
                    const HttpRequestPtr& req,
                    const HttpResponsePtr& resp);

    std::unique_ptr<TcpServer> server_;
    HttpAsyncCallback httpAsyncCallback_;
};

} // namespace god

#endif
