#ifndef GOD_HTTP_HTTPRESPONSE_H
#define GOD_HTTP_HTTPRESPONSE_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

#include "god/utils/NonCopyable.h"
#include "god/net/TcpBuffer.h"
#include "god/http/HttpTypes.h"

namespace god
{

class HttpResponse;

using HttpResponsePtr = std::shared_ptr<HttpResponse>;
using HttpResponseHandler = std::function<void(const HttpResponsePtr&)>;

/// Http响应
class HttpResponse : NonCopyable
{
public:
    static HttpResponsePtr NewNotFound();
    static HttpResponsePtr NewFile(const std::string& filePath);

    void write(TcpBuffer& buf) noexcept;

    HttpVersion version() const noexcept
    {
        return version_;
    }

    void setVersion(HttpVersion version) noexcept
    {
        version_ = version;
    }

    HttpCode code() const noexcept
    {
        return code_;
    }

    void setCode(HttpCode code) noexcept
    {
        code_ = code;
    }

    void addHeader(std::string&& field, std::string&& value) noexcept
    {
        headers_[std::move(field)] = std::move(value);
    }

    void setBody(std::string&& body) noexcept
    {
        body_ = std::move(body);
    }

    bool keepAlive() const noexcept
    {
        return keepAlive_;
    }

    void setKeepAlive(bool on) noexcept
    {
        keepAlive_ = on;
    }

    void setContentType(ContentType type) noexcept
    {
        type_ = type;
    }

    void clear() noexcept
    {
        version_ = HttpVersion::kHttp11;
        code_ = kUnknown;
        headers_.clear();
        body_.clear();
        keepAlive_ = true;
        type_ = CT_NONE;
    }

private:
    // 版本
    HttpVersion version_{HttpVersion::kHttp11};
    // 返回码
    HttpCode code_{kUnknown};
    // 响应头
    std::unordered_map<std::string, std::string> headers_;
    // 文件类型
    ContentType type_{CT_NONE};
    // 主体
    std::string body_;
    // 保持连接
    bool keepAlive_{true};
};

} // namespace god

#endif
