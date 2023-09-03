#ifndef GOD_HTTP_HTTPREQUEST_H
#define GOD_HTTP_HTTPREQUEST_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#include "god/utils/NonCopyable.h"
#include "god/http/HttpTypes.h"

namespace god
{

class HttpRequest;

using HttpRequestPtr = std::shared_ptr<HttpRequest>;

/// Http请求
class HttpRequest : NonCopyable
{
public:
    bool setMethod(const char* start, const char* end);
    bool setVersion(const char* start, const char* end);
    void setPath(const char* start, const char* end);
    void setQuery(const char* start, const char* end);
    void addHeader(const char* start, const char* colon, const char* end);
    void setBody(const char* start, const char* end);

    void setPath(const std::string& str)
    {
        setPath(str.data(), str.data() + str.size());
    }

    HttpMethod method() const noexcept
    {
        return method_;
    }

    const std::string& path() const noexcept
    {
        return path_;
    }

    const std::string& query() const noexcept
    {
        return query_;
    }

    HttpVersion version() const noexcept
    {
        return version_;
    }

    const std::unordered_map<std::string, std::string>& headers()
    {
        return headers_;
    }

    const std::string& getHeader(const std::string& field) const noexcept
    {
        static std::string ret;
        if (auto it = headers_.find(field); it != headers_.end())
        {
            return it->second;
        }
        return ret;
    }

    const std::string& body() const noexcept
    {
        return body_;
    }

    const std::unordered_map<std::string, std::string>& queryParams()
    {
        return queryParams_;
    }

    bool keepAlive() const noexcept
    {
        return keepAlive_;
    }

    void clear()
    {
        method_ = HttpMethod::Invalid;
        path_.clear();
        query_.clear();
        version_ = HttpVersion::kUnknown;
        headers_.clear();
        body_.clear();
        queryParams_.clear();
        keepAlive_ = true;
    }

private:
    // 请求方法
    HttpMethod method_{HttpMethod::Invalid};
    // 请求路径
    std::string path_;
    // 查询参数
    std::string query_;
    // 请求版本
    HttpVersion version_{HttpVersion::kUnknown};
    // 请求头
    std::unordered_map<std::string, std::string> headers_;
    // 请求体
    std::string body_;
    // 查询参数键值
    std::unordered_map<std::string, std::string> queryParams_;
    // 保持连接
    bool keepAlive_{true};
};

} // namespace god

#endif