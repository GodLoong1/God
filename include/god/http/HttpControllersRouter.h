#ifndef GOD_HTTP_HTTPCONTROLLERSROUTES_H
#define GOD_HTTP_HTTPCONTROLLERSROUTES_H

#include <type_traits>
#include <unordered_map>
#include <string>
#include <functional>
#include <regex>
#include <iostream>

#include "god/http/HttpBinder.h"
#include "god/http/HttpTypes.h"
#include "god/http/StaticFileRouter.h"
#include "god/utils/NonCopyable.h"
#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"

namespace god
{

/// Http控制器路由
class HttpControllersRouter : NonCopyable
{
public:
    HttpControllersRouter(std::unique_ptr<StaticFileRouter>& fileRouter);

    void addHttpPath(const std::string& path,
                     const HttpBinderBasePtr& binder,
                     const std::vector<HttpMethod>& methods);

    void route(const HttpRequestPtr& req,
               HttpResponseHandler&& respcb);

private:
    void invokerHandler(HttpResponseHandler&& respcb,
                      const HttpRequestPtr& req,
                      const HttpResponsePtr& resp);

    struct CtrlBinder 
    {
        // 处理函数
        HttpBinderBasePtr httpBinder;
        // 查询参数
        std::vector<std::string> queryKey;
    };
    using CtrlBinderPtr = std::shared_ptr<CtrlBinder>;

    struct RouterItem 
    {
        // 请求路径(包含查询参数)
        std::string path;
        // 请求方法数组
        CtrlBinderPtr binders[HttpMethod::Invalid]{nullptr};
    };

    std::unique_ptr<StaticFileRouter>& fileRouter_;
    // 路径映射(不包含查询参数)
    std::unordered_map<std::string, RouterItem> ctrlMap_;
};

} // namespace god

#endif