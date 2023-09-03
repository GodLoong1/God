#ifndef GOD_HTTP_HTTPAPPFRAMEWORK_H
#define GOD_HTTP_HTTPAPPFRAMEWORK_H

#include "god/orm/DbClientManager.h"
#include "god/net/EventLoopThreadPool.h"
#include "god/http/ListenerManager.h"
#include "god/http/HttpBinder.h"
#include "god/utils/Logger.h"

namespace god
{

class StaticFileRouter;
class HttpControllersRouter;

/// Http框架
class HttpAppFramework
{
public:
    static HttpAppFramework& Instance()
    {
        static HttpAppFramework instance;
        return instance;
    }

    HttpAppFramework();
    ~HttpAppFramework();

    template<typename Func>
    HttpAppFramework& registerHandler(
        const std::string& pathPattern,
        Func&& function,
        const std::vector<HttpMethod>& methods)
    {
        auto binder = std::make_shared<HttpBinder<Func>>(
            std::forward<Func>(function));
        registerHttpController(pathPattern, binder, methods);

        return *this;
    }

    void registerHttpController(
        const std::string& pathPattern,
        const HttpBinderBasePtr& binder,
        const std::vector<HttpMethod>& methods);

    void run();
    void quit();

    EventLoop* getLoop() const
    {
        static EventLoop loop;
        return &loop;
    }

    size_t getCurrentThreadIndex() const
    {
        if (EventLoop* loop = EventLoop::GetLoop())
        {
            return loop->getIndex();
        }
        return EventLoop::nindex;
    }

    HttpAppFramework& setThreadNum(size_t threadNum)
    {
        threadNum_ = threadNum;
        return *this;
    }

    size_t getThreadNum() const
    {
        return threadNum_;
    }

    HttpAppFramework& setConnectionTimeout(size_t timeout)
    {
        connectionTimeout_ = timeout;
        return *this;
    }

    HttpAppFramework& setDocumentRoot(const std::string& rootPath)
    {
        rootPath_ = rootPath;
        return *this;
    }

    const std::string& getDocumentRoot() const
    {
        return rootPath_;
    }

    HttpAppFramework& setHomePage(const std::string& homePageFile)
    {
        homePageFile_ = homePageFile;
        return *this;
    }

    const std::string& getHomePage() const
    {
        return homePageFile_;
    }

    DbClientPtr& getDbClient(const std::string& name)
    {
        return dbClientManager_->getDbClient(name);
    }

    HttpAppFramework& addListener(const std::string& ip, uint16_t port)
    {
        listenerManager_->addListener(ip, port);
        return *this;
    }

    HttpAppFramework& setLogger(std::unique_ptr<Logger>&& logger)
    {
        GOD_LOG = std::move(logger);
        return *this;
    }

    HttpAppFramework& setLogLevel(LogLevel level)
    {
        GOD_LOG->setLevel(level);
        return *this;
    }

    HttpAppFramework& setLogForamt(std::unique_ptr<LogFormat>&& format)
    {
        GOD_LOG->setFormat(std::move(format));
        return *this;
    }

    HttpAppFramework& setLogPlace(std::unique_ptr<LogPlace>&& place)
    {
        GOD_LOG->setPlace(std::move(place));
        return *this;
    }

    HttpAppFramework& addDbClient(DbClientType type,
                                  const std::string& host,
                                  uint16_t port,
                                  const std::string& databaseName,
                                  const std::string& userName,
                                  const std::string& password,
                                  size_t threadNum,
                                  size_t connNum,
                                  const std::string clientName,
                                  const std::string characterSet)
    {
        dbClientManager_->addDbClient(type,
                                      host,
                                      port,
                                      databaseName,
                                      userName,
                                      password,
                                      threadNum,
                                      connNum, 
                                      clientName,
                                      characterSet);
        return *this;
    }

    void callHandler(
        const HttpRequestPtr& req,
        const HttpResponsePtr& resp,
        const HttpResponseHandler& respcb);

    void onAsyncRequest(const HttpRequestPtr& req,
                        HttpResponseHandler&& respcb);

private:
    std::unique_ptr<StaticFileRouter> staticFileRouter_;
    std::unique_ptr<HttpControllersRouter> httpCtrlRouter_;
    std::unique_ptr<ListenerManager> listenerManager_;
    std::unique_ptr<DbClientManager> dbClientManager_;

    size_t threadNum_{1};
    std::unique_ptr<EventLoopThreadPool> ioLoopThreadPool_;

    size_t connectionTimeout_{60};
    std::string rootPath_{"./"};
    std::string homePageFile_{"index.html"};
};

inline HttpAppFramework& app()
{
    return HttpAppFramework::Instance();
}

} // namespace god

#endif