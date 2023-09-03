#ifndef GOD_HTTP_HTTPREQUESTPARSER_H
#define GOD_HTTP_HTTPREQUESTPARSER_H

#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"
#include "god/net/TcpBuffer.h"
#include "god/net/TcpConnection.h"

namespace god
{

class HttpRequestParser;

using HttpRequestParserPtr = std::shared_ptr<HttpRequestParser>;

/// Http请求解析
class HttpRequestParser
{
public:
    enum Status
    {
        kMethod,
        kUri,
        kVersion,
        kHeaders,
        kBody,
        kGotAll,
    };

    explicit HttpRequestParser(const TcpConnectionPtr& conn)
    : weakConn_(conn) { }

    bool parseRequest(TcpBuffer& buf);

    void shutdownConnection(HttpCode code);

    const HttpRequestPtr& getRequest() const
    {
        return request_;
    }

    TcpBuffer& getSendBuf()
    {
        return sendBuf_;
    }

    void clear()
    {
        status_ = kMethod;
        request_->clear();
        sendBuf_.retrieveAll();
    }

private:
    // Tcp连接
    std::weak_ptr<TcpConnection> weakConn_;
    // 解析状态
    Status status_{kMethod};
    // http请求
    HttpRequestPtr request_{new HttpRequest};
    // 发送响应缓冲区
    TcpBuffer sendBuf_;
};

} // namespace god

#endif