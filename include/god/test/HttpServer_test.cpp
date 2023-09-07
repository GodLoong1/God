#include "god/http/HttpController.h"
#include "god/net/EventLoop.h"
#include "god/orm/Result.h"
#include "god/utils/Logger.h"
#include <exception>
#include <memory>
#include <stdexcept>

using namespace god;

class MyController : public HttpController<MyController>,
                     public std::enable_shared_from_this<MyController>
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

    void test(const HttpRequestPtr&, HttpResponseHandler&&, int)
    {
        auto db = app().getDbClient("mysql");

        db->execSqlAsync("select * from emp", [this] (const ResultPtr& result) {
            output(result);
        });
    }

    void output(const ResultPtr& result)
    {
        for (auto& row : *result)
        {
            std::string str;
            for (auto& field : row)
            {
                str = str + field.name() + ":" + field.as() + " ";
            }
            str.pop_back();
            LOG_INFO << str;
        }
    }
};

int main()
{
    app().setThreadNum(2)
         .addListener("0.0.0.0", 9981)
         .setConnectionTimeout(30)
         .setDocumentRoot("/home/ubuntu/workspace/God/src/")
         .setHomePage("reference/zh/index.html")
        //  .setLogLevel(LogLevel::info)
         .setLogger(std::make_unique<AsyncLogger>())
         .addDbClient(DbClientType::Mysql, "localhost", 3306, "book", "root",
                            "xL20010824!", 1, 6, "mysql", "")
         .run();
}