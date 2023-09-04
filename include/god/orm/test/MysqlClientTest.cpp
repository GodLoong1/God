#include "god/orm/DbClientManager.h"
#include "god/orm/Result.h"
#include "god/orm/SqlBinder.h"
#include "god/utils/Logger.h"
#include <future>
#include <unistd.h>

static constexpr int count = 100;

std::promise<void> promise;
std::future<void> future = promise.get_future();

void queryFunc(const god::ResultPtr& result)
{
    LOG_INFO << "affectedRow: " << result->affectedRows();

    static int i = 0;
    if (++i == count)
    {
        promise.set_value();
    }
}

void ExceptFunc(const std::exception& e)
{
    LOG_INFO << "exception what: " << e.what();
}

int main()
{
    GOD_LOG->setPlace(std::make_unique<god::LogFileOut>("MysqlClient"));
    GOD_LOG->setLevel(god::LogLevel::info);

    god::DbClientManager manager;
    manager.addDbClient(god::DbClientType::Mysql, "localhost", 3306, "book", "root", "xL20010824!", 1, 3, "mysql", "");
    manager.startDbClient();
    auto db = manager.getDbClient("mysql");

    LOG_INFO << "start";

    for (int i = 0; i != count; ++i)
    {
        db->execSqlAsync("select * from emp;", queryFunc, ExceptFunc);
    }
    LOG_INFO << "end";

    future.get();
}