#include "god/http/HttpControllersRouter.h"

#include <cctype>
#include <memory>
#include <regex>
#include <algorithm>
#include <cassert>

#include "god/http/HttpAppFramework.h"
#include "god/http/HttpResponse.h"
#include "god/http/HttpTypes.h"
#include "god/utils/Logger.h"

namespace god
{

HttpControllersRouter::HttpControllersRouter(
    std::unique_ptr<StaticFileRouter>& fileRouter)
: fileRouter_(fileRouter)
{
}

void HttpControllersRouter::addHttpPath(
    const std::string& path,
    const HttpBinderBasePtr& httpBinder,
    const std::vector<HttpMethod>& methods)
{
    LOG_TRACE << "HttpControllersRouter::addHttpPath: path: " << path;

    std::string pathTmp;
    std::string queryTmp;

    if (size_t pos = path.find('?'); pos != std::string::npos)
    {
        pathTmp = path.substr(0, pos);
        queryTmp = path.substr(pos + 1);
    }
    else
    {
        pathTmp = path;
    }

    std::vector<std::string> queryKey;
    queryKey.reserve(httpBinder->paramCount());

    // 匹配查询占位符 key={param}&
    if (!queryTmp.empty())
    {
        std::regex queryPattern("([^&]*)=\\{([^&]*)\\}&*");
        std::smatch results;

        while (std::regex_search(queryTmp, results, queryPattern))
        {
            if (results.size() > 2)
            {
                queryKey.push_back(results[1].str());
            }
            queryTmp = results.suffix();
        }
    }
    assert(httpBinder->paramCount() == queryKey.size());

    CtrlBinderPtr ctrlBinder(new CtrlBinder);
    ctrlBinder->httpBinder = httpBinder;
    ctrlBinder->queryKey = std::move(queryKey);

    if (auto it = ctrlMap_.find(pathTmp); it != ctrlMap_.end())
    {
        for (HttpMethod method : methods)
        {
            it->second.binders[method] = ctrlBinder;
        }
    }
    else
    {
        RouterItem item;
        item.path = path;

        for (HttpMethod method : methods)
        {
            item.binders[method] = ctrlBinder;
        }
        ctrlMap_.emplace(std::move(pathTmp), std::move(item));
    }
}

void HttpControllersRouter::route(const HttpRequestPtr& req,
                                  HttpResponseHandler&& respcb)

{
    LOG_TRACE << "HttpControllersRouter::route: path: " << req->path();

    auto it = ctrlMap_.find(req->path());
    if (it == ctrlMap_.end())
    {
        fileRouter_->route(req, std::move(respcb));
        return;
    }

    auto& binder = it->second.binders[req->method()];
    if (!binder)
    {
        fileRouter_->route(req, std::move(respcb));
        return;
    }

    // 解析参数
    std::deque<std::string> params;
    if (!binder->queryKey.empty())
    {
        const auto& queryMap = req->queryParams();

        for (const std::string& key : binder->queryKey)
        {
            auto iter = queryMap.find(key);
            if (iter != queryMap.end())
            {
                params.push_back(iter->second);
            }
            else
            {
                params.emplace_back();
            }
        }
    }

    binder->httpBinder->handleHttpRequest(
        req,
        [this, req, respcb = std::move(respcb)]
                (const HttpResponsePtr &resp) mutable {
            invokerHandler(std::move(respcb), req, resp);
        },
        params
    );
}

void HttpControllersRouter::invokerHandler(
        HttpResponseHandler&& respcb,
        const HttpRequestPtr& req,
        const HttpResponsePtr& resp)
{
    LOG_TRACE << "HttpControllersRouter::invokerHandler: respCode: "
              << httpCodeToString(resp->code());

    app().callHandler(req, resp, std::move(respcb));
}

} // namespace god