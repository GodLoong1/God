#ifndef GOD_HTTP_STATICFILEROUTER_H
#define GOD_HTTP_STATICFILEROUTER_H

#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"

#include "god/http/IOThreadStorage.h"
#include "god/http/CacheMap.h"
#include "god/net/EventLoop.h"

namespace god
{

/**
 * @brief 静态文件路由
 * 
 */
class StaticFileRouter
{
public:
    void init(const std::vector<EventLoop*>& ioLoops);

    void route(const HttpRequestPtr& req,
               HttpResponseHandler&& respcb);

    void sendStaticFileResponse(const std::string& filePath,
                                const HttpRequestPtr& req,
                                HttpResponseHandler&& respcb);

private:
    size_t cacheTime_{10};
    std::unique_ptr<
        IOThreadStorage<std::unique_ptr<
            CacheMap<std::string, HttpResponsePtr>>>> cacheMap_;
};

} // namespace god

#endif