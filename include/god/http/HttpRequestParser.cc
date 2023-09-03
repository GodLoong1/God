#include "HttpRequestParser.h"

#include "god/http/HttpTypes.h"

namespace god
{

bool HttpRequestParser::parseRequest(TcpBuffer& buf)
{
    static constexpr char CRLF[] = "\r\n";

    while (true)
    {
        switch (status_)
        {
            case kMethod:
            {
                const char* methodPos = std::find(buf.readPeek(),
                                                  buf.writePeek(), ' ');
                if (methodPos == buf.writePeek())
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }

                if (!request_->setMethod(buf.readPeek(), methodPos))
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }

                buf.retrieveUntil(methodPos + 1);
                status_ = kUri;
                continue;
            }
            case kUri:
            {
                const char* urlPos = std::find(buf.readPeek(),
                                               buf.writePeek(), ' ');
                if (urlPos == buf.writePeek())
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }
            
                const char* queryPos = std::find(buf.readPeek(),
                                                 urlPos, '?');
                if (queryPos == urlPos)
                {
                    request_->setPath(buf.readPeek(), urlPos);
                }
                else
                {
                    request_->setPath(buf.readPeek(), queryPos);
                    request_->setQuery(queryPos + 1, urlPos);
                }

                buf.retrieveUntil(urlPos + 1);
                status_ = kVersion;
                continue;
            }
            case kVersion:
            {
                const char* versionPos = std::search(buf.readPeek(),
                                                     buf.writePeek(),
                                                     CRLF,
                                                     CRLF + 2);
                if (versionPos == buf.writePeek())
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }

                if (!request_->setVersion(buf.readPeek(), versionPos))
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }

                buf.retrieveUntil(versionPos + 2);
                status_ = kHeaders;
                continue;
            }
            case kHeaders:
            {
                const char* headerPos = std::search(buf.readPeek(),
                                                    buf.writePeek(),
                                                    CRLF,
                                                    CRLF + 2);
                if (headerPos == buf.writePeek())
                {
                    buf.retrieveAll();
                    shutdownConnection(k400BadRequest);
                    return false;
                }

                const char* colon = std::find(buf.readPeek(),
                                              headerPos, ':');
                if (colon != headerPos)
                {
                    request_->addHeader(buf.readPeek(), colon, headerPos);
                    buf.retrieveUntil(headerPos + 2);
                    continue;
                }

                buf.retrieveUntil(headerPos + 2);
                status_ = kBody;
                continue;
            }
            case kBody:
            {
                const std::string& str = request_->getHeader("content-length");
                if (!str.empty())
                {
                    size_t len = std::stoul(str);
                    if (len <= buf.readByte())
                    {
                        request_->setBody(buf.readPeek(), buf.readPeek() + len);
                        buf.retrieveUntil(buf.readPeek() + len);
                    }
                    else
                    {
                        buf.retrieveAll();
                        shutdownConnection(k400BadRequest);
                        return false;
                    }
                }
                status_ = kGotAll;
                continue;
            }
            case kGotAll:
            {
                return true;
            }
        }
    }
    return false;
}

void HttpRequestParser::shutdownConnection(HttpCode code)
{
    if (auto conn = weakConn_.lock())
    {
        char buf[64];
        int len = snprintf(buf, sizeof(buf),
                           "HTTP/1.1 %s\r\nConnection: close\r\n\r\n",
                           httpCodeToString(code).data());
        conn->send(std::string(buf, len));
    }
}

} // namespace god