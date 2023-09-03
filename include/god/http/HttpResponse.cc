#include "god/http/HttpResponse.h"
#include "god/http/HttpTypes.h"
#include "god/utils/Logger.h"

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace god
{

HttpResponsePtr HttpResponse::NewNotFound()
{
    HttpResponsePtr resp(new HttpResponse);
    resp->setCode(k404NotFound);
    resp->setBody("<html><body><h1>404 Not Found</h1></body></html>");
    resp->setContentType(CT_TEXT_HTML);
    resp->setKeepAlive(false);

    return resp;
}

HttpResponsePtr HttpResponse::NewFile(const std::string& filePath)
{
    struct stat st;
    if (::stat(filePath.data(), &st) < 0)
    {
        return HttpResponse::NewNotFound();
    }

    int fd = ::open(filePath.data(), O_RDONLY);
    if (fd < 0)
    {
        return HttpResponse::NewNotFound();
    }

    void* ptr = ::mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        return HttpResponse::NewNotFound();
    }

    HttpResponsePtr resp(new HttpResponse);
    resp->setCode(k200OK);
    resp->setBody(std::string(static_cast<char*>(ptr), st.st_size));
    resp->setContentType(getContentType(filePath));

    ::munmap(ptr, st.st_size);

    return resp;
}

void HttpResponse::write(TcpBuffer& buf) noexcept
{
    buf.write(httpVersionToString(version_));
    buf.write(" ");
    buf.write(httpCodeToString(code_));
    buf.write("\r\n");

    if (keepAlive_)
    {
        buf.write("Connection: Keep-Alive\r\n");
    }
    else
    {
        buf.write("Connection: close\r\n");
    }

    for (const auto& [key, val] : headers_)
    {
        buf.write(key);
        buf.write(": ");
        buf.write(val);
        buf.write("\r\n");
    }

    buf.write("Content-Type: ");
    buf.write(contentTypeToString(type_));
    buf.write("\r\n");

    if (!body_.empty())
    {
        buf.write("Content-Length: ");
        buf.write(std::to_string(body_.size()));
        buf.write("\r\n\r\n");
        buf.write(body_);
    }
    else
    {
        buf.write("\r\n");
    }
}

} // namespace god