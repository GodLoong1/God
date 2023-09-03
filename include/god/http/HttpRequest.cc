#include "god/http/HttpRequest.h"

#include <algorithm>
#include <cctype>

namespace god
{

bool HttpRequest::setMethod(const char* start, const char* end)
{
    method_ = Invalid;
    std::string_view str(start, end - start);

    switch (str.length())
    {
        case 3:
        {
            if (str == "GET")
            {
                method_ = Get;
            }
            break;
        }
        case 4:
        {
            if (str == "POST")
            {
                method_ = Post;
            }
            break;
        }
    }
    return method_ != Invalid;
}

bool HttpRequest::setVersion(const char* start, const char* end)
{
    version_ = HttpVersion::kUnknown;

    if (end - start == 8 && std::equal(start, end - 1, "HTTP/1."))
    {
        if (*(end - 1) == '1')
        {
            version_ = HttpVersion::kHttp11;
        }
        else if (*(end - 1) == '0')
        {
            version_ = HttpVersion::kHttp10;
            keepAlive_ = false;
        }
    }
    return version_ != HttpVersion::kUnknown;
}

void HttpRequest::setPath(const char* start, const char* end)
{
    path_.assign(urlDecode(start, end));
}

void HttpRequest::setQuery(const char* start, const char* end)
{
    query_.assign(start, end);

    size_t pos = 0;
    std::string_view view(query_);

    while ((pos = view.find('&')) != std::string_view::npos)
    {
        auto pa = view.substr(0, pos);
        if (auto pos = view.find('='); pos != std::string_view::npos)
        {
            std::string key = urlDecode(trim(pa.substr(0, pos)));
            std::string val = urlDecode(trim(pa.substr(pos + 1)));
            queryParams_.emplace(std::move(key), std::move(val));
        }
        view = view.substr(pos + 1);
    }

    if (view.size())
    {
        if (auto pos = view.find('='); pos != std::string_view::npos)
        {
            std::string key = urlDecode(trim(view.substr(0, pos)));
            std::string val = urlDecode(trim(view.substr(pos + 1)));
            queryParams_.emplace(std::move(key), std::move(val));
        }
    }
}

void HttpRequest::addHeader(const char* start,
                            const char* colon,
                            const char* end)
{
    std::string field(trim(start, colon));
    std::string value(trim(colon + 1, end));

    std::transform(field.begin(), field.end(), field.begin(),
                   [](char c) { return ::tolower(c); });

    if (field.size() == 10 && field == "connection")
    {
        if (version_ == HttpVersion::kHttp11)
        {
            if (value.size() == 5 && value == "close")
            {
                keepAlive_ = false;
            }
            else if (value.size() == 10 &&
                     (value == "Keep-Alive" || value == "keep-alive"))
            {
                keepAlive_ = true;
            }
        }
    }
    headers_.emplace(std::move(field), std::move(value));
}

void HttpRequest::setBody(const char* start, const char* end)
{
    body_.assign(start, end);
}

} // namespace god