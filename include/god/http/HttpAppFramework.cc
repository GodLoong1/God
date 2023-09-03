#include "god/http/HttpAppFramework.h"

#include <csignal>

#include "god/orm/DbClientManager.h"
#include "god/utils/Logger.h"
#include "god/http/HttpControllersRouter.h"
#include "god/http/StaticFileRouter.h"

namespace god
{

HttpAppFramework::HttpAppFramework()
: staticFileRouter_(new StaticFileRouter),
  httpCtrlRouter_(new HttpControllersRouter(staticFileRouter_)),
  listenerManager_(new ListenerManager),
  dbClientManager_(new DbClientManager)
{
    LOG_TRACE << getThreadName() << ": HttpAppFramework";
}

HttpAppFramework::~HttpAppFramework()
{
    LOG_TRACE << getThreadName() << ": ~HttpAppFramework";
}

void HttpAppFramework::registerHttpController(
    const std::string& pathPattern,
    const HttpBinderBasePtr& binder,
    const std::vector<HttpMethod>& methods)
{
    httpCtrlRouter_->addHttpPath(pathPattern, binder, methods);
}

void HttpAppFramework::run()
{
    getLoop()->assertInLoop();

    ::signal(SIGINT, [](int) {
        app().quit();
    });

    ::signal(SIGTERM, [](int) {
        app().quit();
    });

    ioLoopThreadPool_ = std::make_unique<EventLoopThreadPool>(threadNum_, "IoLoop");
    ioLoopThreadPool_->start();

    std::vector<EventLoop*> ioLoops = ioLoopThreadPool_->getLoops();
    for (size_t i = 0; i != threadNum_; ++i)
    {
        ioLoops[i]->setIndex(i);
    }
    getLoop()->setIndex(threadNum_);

    listenerManager_->createListeners(
        [this](const HttpRequestPtr& req, HttpResponseHandler&& respcb) {
            onAsyncRequest(req, std::move(respcb));
        },
        connectionTimeout_,
        ioLoops
    );

    ioLoops.push_back(getLoop());

    staticFileRouter_->init(ioLoops);

    getLoop()->addInLoop([this] {
        listenerManager_->startListening();
    });

    dbClientManager_->startDbClient();

    getLoop()->loop();
}

void HttpAppFramework::quit()
{
    getLoop()->runInLoop([this] {
        listenerManager_->stopListening();
        listenerManager_.reset();
        staticFileRouter_.reset();
        httpCtrlRouter_.reset();
        dbClientManager_.reset();

        ioLoopThreadPool_.reset();
        getLoop()->quit();
    });
}

void HttpAppFramework::callHandler(
        const HttpRequestPtr&,
        const HttpResponsePtr& resp,
        const HttpResponseHandler& respcb)
{
    LOG_TRACE << "HttpAppFramework::callHandler";
    respcb(resp);
}

void HttpAppFramework::onAsyncRequest(const HttpRequestPtr& req,
                                      HttpResponseHandler&& callback)
{
    LOG_TRACE << "HttpAppFramework::onAsyncRequest";
    httpCtrlRouter_->route(req, std::move(callback));
}

} // namespace god