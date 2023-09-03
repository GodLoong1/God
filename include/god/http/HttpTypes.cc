#include "god/http/HttpTypes.h"

#include <unordered_map>

namespace god
{

const std::string_view &httpCodeToString(HttpCode code)
{
    switch (code)
    {
        case k200OK:
        {
            static std::string_view sv = "200 OK";
            return sv;
        }
        case k302Found:
        {
            static std::string_view sv = "302 Found";
            return sv;
        }
        case k400BadRequest:
        {
            static std::string_view sv = "400 Bad Request";
            return sv;
        }
        case k404NotFound:
        {
            static std::string_view sv = "404 Not Found";
            return sv;
        }
        case kUnknown:
        default:
        {
            static std::string_view sv = "Undefined Error";
            return sv;
        }
    }
}

const std::string_view& httpVersionToString(HttpVersion version)
{
    if (version == HttpVersion::kHttp11)
    {
        static std::string_view sv = "HTTP/1.1";
        return sv;
    }
    else if (version == HttpVersion::kHttp10)
    {
        static std::string_view sv = "HTTP/1.0";
        return sv;
    }
    else
    {
        static std::string_view sv = "Unknown";
        return sv;
    }
}

const std::string_view &contentTypeToString(ContentType type)
{
    switch (type)
    {
        case CT_TEXT_HTML:
        {
            static std::string_view sv = "text/html; charset=utf-8";
            return sv;
        }
        case CT_APPLICATION_XML:
        {
            static std::string_view sv = "application/xml; charset=utf-8";
            return sv;
        }
        case CT_APPLICATION_JSON:
        {
            static std::string_view sv = "application/json; charset=utf-8";
            return sv;
        }
        case CT_APPLICATION_X_JAVASCRIPT:
        {
            static std::string_view sv = "application/x-javascript; charset=utf-8";
            return sv;
        }
        case CT_APPLICATION_X_FONT_TRUETYPE:
        {
            static std::string_view sv = "application/x-font-truetype";
            return sv;
        }
        case CT_TEXT_CSS:
        {
            static std::string_view sv = "text/css; charset=utf-8";
            return sv;
        }
        case CT_IMAGE_PNG:
        {
            static std::string_view sv = "image/png";
            return sv;
        }
        case CT_IMAGE_JPG:
        {
            static std::string_view sv = "image/jpeg";
            return sv;
        }
        case CT_IMAGE_GIF:
        {
            static std::string_view sv = "image/gif";
            return sv;
        }
        case CT_IMAGE_SVG_XML:
        {
            static std::string_view sv = "image/svg+xml";
            return sv;
        }
        case CT_NONE:
        {
            static std::string_view sv = "";
            return sv;
        }
        default:
        case CT_TEXT_PLAIN:
        {
            static std::string_view sv = "text/plain; charset=utf-8";
            return sv;
        }
    }
}

ContentType getContentType(const std::string &fileName)
{
    std::string extName;
    if (auto pos = fileName.rfind('.'); pos != std::string::npos)
    {
        extName = fileName.substr(pos + 1);
    }
    else
    {
        return CT_NONE;
    }

    static const std::unordered_map<std::string_view, ContentType> map{
        {"txt", CT_TEXT_PLAIN},
        {"html", CT_TEXT_HTML},
        {"css", CT_TEXT_CSS},
        {"json", CT_APPLICATION_JSON},
        {"xml", CT_APPLICATION_XML},
        {"js", CT_APPLICATION_X_JAVASCRIPT},
        {"ttf", CT_APPLICATION_X_FONT_TRUETYPE},
        {"png", CT_IMAGE_PNG},
        {"jpg", CT_IMAGE_JPG},
        {"gif", CT_IMAGE_GIF},
        {"svg", CT_IMAGE_SVG_XML},
    };

    if (auto it = map.find(extName); it != map.end())
    {
        return it->second;
    }
    return CT_NONE;
}

std::string_view trim(const char* start, const char* end)
{
    // 去除开头空格
    while (start < end && ::isspace(*start))
    {
        ++start;
    }

    // 去除末尾空格
    while (end > start && ::isspace(*(end - 1)))
    {
        --end;
    }
    return std::string_view(start, end - start);
}

std::string urlDecode(const char* start, const char* end)
{
    std::string result;
    size_t len = end - start;
    result.reserve(len * 2);

    int hex = 0;
    for (size_t i = 0; i < len; ++i)
    {
        switch (start[i])
        {
            case '+':
                result += ' ';
                break;
            case '%':
                if ((i + 2) < len &&
                    ::isxdigit(start[i + 1]) &&
                    ::isxdigit(start[i + 2]))
                {
                    unsigned int x1 = start[i + 1];
                    if (x1 >= '0' && x1 <= '9')
                    {
                        x1 -= '0';
                    }
                    else if (x1 >= 'a' && x1 <= 'f')
                    {
                        x1 = x1 - 'a' + 10;
                    }
                    else if (x1 >= 'A' && x1 <= 'F')
                    {
                        x1 = x1 - 'A' + 10;
                    }
                    unsigned int x2 = start[i + 2];
                    if (x2 >= '0' && x2 <= '9')
                    {
                        x2 -= '0';
                    }
                    else if (x2 >= 'a' && x2 <= 'f')
                    {
                        x2 = x2 - 'a' + 10;
                    }
                    else if (x2 >= 'A' && x2 <= 'F')
                    {
                        x2 = x2 - 'A' + 10;
                    }
                    hex = x1 * 16 + x2;

                    result += char(hex);
                    i += 2;
                }
                else
                {
                    result += '%';
                }
                break;
            default:
                result += start[i];
                break;
        }
    }
    return result;
}

} // namespace god