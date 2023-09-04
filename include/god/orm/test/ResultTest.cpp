#include "god/orm/Result.h"
#include "god/orm/MysqlResult.h"
#include "god/utils/Logger.h"
#include <mariadb/mysql.h>
#include <iostream>
#include <memory>
#include <type_traits>

god::ResultPtr getResult()
{
    MYSQL mysql;
    mysql_init(&mysql);
    mysql_real_connect(&mysql, "localhost", "root",
        "xL20010824!", "book", 3306, nullptr, 0);
    std::string_view query = "select * from emp";
    mysql_real_query(&mysql, query.data(), query.size());
    MYSQL_RES* res = mysql_store_result(&mysql);
    mysql_close(&mysql);

    return god::ResultPtr(new god::MysqlResult(res, mysql_affected_rows(&mysql)));
}

int main()
{
    std::cout << std::is_same_v<decltype(("")), const char(&)[1]> << std::endl;
}