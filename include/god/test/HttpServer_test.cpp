#include "god/http/HttpController.h"
#include "god/utils/Logger.h"
#include <exception>
#include <stdexcept>

using namespace god;

class MyController : public HttpController<MyController>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(MyController::quit, "/quit", Get);
    METHOD_ADD(MyController::test, "/test?id={}", Get);
    METHOD_LIST_END

    MyController()
    {
        LOG_INFO << "!!!!!!!!!!!!!!!!!!!!!!!MyController()";
    }

    void quit(const HttpRequestPtr&, HttpResponseHandler&&)
    {
        app().quit();
    }

    void test(const HttpRequestPtr&, HttpResponseHandler&&, int )
    {
        auto db = app().getDbClient("mysql");
    }
};

int main()
{
    app().setThreadNum(2)
         .addListener("0.0.0.0", 9981)
         .setConnectionTimeout(30)
         .setDocumentRoot("/home/ubuntu/god/src/")
         .setHomePage("reference/zh/index.html")
        //  .setLogLevel(LogLevel::info)
         .setLogger(std::make_unique<AsyncLogger>())
         .addDbClient(DbClientType::Mysql, "localhost", 3306, "book", "root",
                            "xL20010824!", 1, 6, "mysql", "")
         .run();
}