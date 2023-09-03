#include "god/http/StaticFileRouter.h"

#include "god/http/HttpAppFramework.h"
#include "god/http/HttpTypes.h"
#include "god/http/IOThreadStorage.h"
#include "god/utils/Logger.h"
#include <future>

namespace god
{

void StaticFileRouter::init(const std::vector<EventLoop*>& ioLoops)
{
    cacheMap_ = std::make_unique<
        IOThreadStorage<std::unique_ptr<
            CacheMap<std::string, HttpResponsePtr>>>>();

    cacheMap_->init(
        [&ioLoops](std::unique_ptr<CacheMap<std::string,
                                            HttpResponsePtr>>& mapPtr,
                   size_t i) {
            assert(i == ioLoops[i]->getIndex());
            mapPtr = std::make_unique<
            CacheMap<std::string, HttpResponsePtr>>(ioLoops[i], 1.0, 4, 50);
    });
}

void StaticFileRouter::route(const HttpRequestPtr& req,
                             HttpResponseHandler&& respcb)
{
    if (req->path().find("..") != std::string::npos)
    {
        respcb(HttpResponse::NewNotFound());
        return;
    }

    std::string path;
    if (req->path() == "/")
    {
        path = app().getHomePage();
    }
    else
    {
        path = req->path().substr(1);
    }

    path = app().getDocumentRoot() + path;
    LOG_TRACE << "StaticFileRouter::route: path: " << path;

    sendStaticFileResponse(path, req, std::move(respcb));
}

void StaticFileRouter::sendStaticFileResponse(
    const std::string& filePath,
    const HttpRequestPtr& req,
    HttpResponseHandler&& respcb)
{
    // 查找缓存响应
    HttpResponsePtr cacheResp;
    auto optval = cacheMap_->getThreadData()->find(filePath);
    if (optval)
    {
        cacheResp = optval.value();
    }

    if (cacheResp)
    {
        LOG_TRACE << "Using file cache";
        app().callHandler(req, cacheResp, respcb);
        return;
    }

    HttpResponsePtr resp;
    if (!resp)
    {
        resp = HttpResponse::NewFile(filePath);
    }

    if (resp->code() != k404NotFound)
    {

        if (req->path() == "/")
        {
            resp->setCode(k302Found);
            resp->addHeader("Location", "/" + app().getHomePage());
        }

        if (cacheTime_ > 0)
        {
            cacheMap_->getThreadData()->insert(filePath, resp, cacheTime_);
        }
        app().callHandler(req, resp, respcb);
        return;
    }

    respcb(resp);
    return;
}

} // namespace god