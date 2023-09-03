#include "god/http/HttpServer.h"

#include <cassert>

#include "god/http/HttpRequestParser.h"
#include "god/http/HttpResponse.h"
#include "god/http/HttpTypes.h"
#include "god/net/InetAddress.h"
#include "god/net/TcpBuffer.h"
#include "god/utils/Logger.h"

namespace god
{

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const std::string_view& name)
  : server_(new TcpServer(loop, listenAddr, name))
{
    LOG_TRACE << getThreadName() << ": HttpServer: " << server_->getName();

    server_->setConnectionCallback(
        [this](const TcpConnectionPtr& conn) {
            onConnection(conn);
    });
    server_->setMessageCallback(
        [this](const TcpConnectionPtr& conn, TcpBuffer& buf) {
            onMessage(conn, buf);
    });
}

HttpServer::~HttpServer()
{
    LOG_TRACE << getThreadName() << ": ~HttpServer: " << server_->getName();
}

void HttpServer::start()
{
    assert(httpAsyncCallback_);
    server_->start();
}

void HttpServer::stop()
{
    server_->stop();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->isConnected())
    {
        LOG_TRACE << "HttpServer::onConnection connected conn "
                  << conn->fd();

        conn->setContext(std::make_shared<HttpRequestParser>(conn));
    }
    else if (conn->isDisconnected())
    {
        LOG_TRACE << "HttpServer::onConnection disconnected conn "
                  << conn->fd();

        conn->setContext(nullptr);
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, TcpBuffer& buf)
{
    LOG_TRACE << "HttpServer::onMessage conn " << conn->fd();

    auto parser = conn->getContext<HttpRequestParser>();

    if (!parser->parseRequest(buf))
    {
        conn->forceClose();
        return;
    }

    onRequest(conn, parser, parser->getRequest());
    parser->clear();
}

void HttpServer::onRequest(const TcpConnectionPtr& conn,
                           const HttpRequestParserPtr& parser,
                           const HttpRequestPtr& req)
{
    LOG_TRACE << "HttpServer::onRequest conn " << conn->fd();

    httpAsyncCallback_(
        req,
        [this, conn, parser, req](const HttpResponsePtr& resp) {
            onResponse(conn, parser, req, resp);
        }
    );
}

void HttpServer::onResponse(
        const TcpConnectionPtr& conn,
        const HttpRequestParserPtr& parser,
        const HttpRequestPtr& req,
        const HttpResponsePtr& resp)
{
    LOG_TRACE << "HttpServer::onResponse conn " << conn->fd();

    resp->setVersion(req->version());
    resp->setKeepAlive(req->keepAlive());

    TcpBuffer& buf = parser->getSendBuf();
    resp->write(buf);
    conn->send(buf);

    LOG_INFO << "version: " << httpVersionToString(req->version())
             << ", path: " << req->path() << ", code: "
             << httpCodeToString(resp->code());

    if (!req->keepAlive())
    {
        conn->shutdown();
    }
}

} // namespace god