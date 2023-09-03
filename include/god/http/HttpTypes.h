#ifndef GOD_HTTP_HTTPTYPES_H
#define GOD_HTTP_HTTPTYPES_H

#include <string>
#include <string_view>

namespace god
{

/// http返回码
enum HttpCode
{
    kUnknown = 0,
    k200OK,
    k302Found,
    k400BadRequest,
    k404NotFound,
};

/// http版本
enum class HttpVersion
{
    kUnknown = 0,
    kHttp10,
    kHttp11
};

/// 文件类型
enum ContentType
{
    CT_NONE,
    CT_TEXT_PLAIN,
    CT_TEXT_HTML,
    CT_TEXT_CSS,
    CT_APPLICATION_JSON,
    CT_APPLICATION_X_JAVASCRIPT,
    CT_APPLICATION_XML,
    CT_APPLICATION_X_FONT_TRUETYPE,
    CT_IMAGE_PNG,
    CT_IMAGE_JPG,
    CT_IMAGE_GIF,
    CT_IMAGE_SVG_XML,
};

/// 请求方法
enum HttpMethod
{
    Get = 0,
    Post,
    Invalid,
};

// 去除左右空格
std::string_view trim(const char* start, const char* end);

// 解析URL编码
std::string urlDecode(const char* start, const char* end);

// http返回码转字符串
const std::string_view& httpCodeToString(HttpCode code);

// http版本转字符串
const std::string_view& httpVersionToString(HttpVersion version);

// 文件类型转字符串
const std::string_view& contentTypeToString(ContentType type);

// 获取文件类型
ContentType getContentType(const std::string &fileName);


inline std::string_view trim(const std::string_view& str)
{
    return trim(str.begin(), str.end());
}

inline std::string urlDecode(const std::string_view& str)
{
    return urlDecode(str.begin(), str.end());
}

} // namespace god

#endif