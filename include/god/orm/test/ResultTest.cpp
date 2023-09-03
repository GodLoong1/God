#include "god/orm/Result.h"
#include "god/orm/MysqlResult.h"
#include "god/utils/Logger.h"
#include <mariadb/mysql.h>
#include <iostream>
#include <memory>

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
    god::ResultPtr result = getResult();

    for (const god::Row& row : *result)
    {
        LOG_INFO << "id: " << row["id"].as<const char*>()
                 << ", workno: " << row["workno"].as<int>()
                 << ", name: " << row["name"].as<std::string_view>()
                 << ", gender: " << row["gender"].as<std::string>()
                 << ", age: " << row["age"].as<double>()
                 << ", idcard: " << row["idcard"].as<size_t>()
                 << ", workaddress: " << row["workaddress"].as<std::string>()
                 << ", entrydate: " << row["entrydate"].as();
    }
}