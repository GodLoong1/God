#ifndef GOD_ORM_SQLBINDER_H
#define GOD_ORM_SQLBINDER_H

#include <exception>
#include <functional>
#include <string>
#include <string_view>
#include <charconv>

#include "god/utils/NonCopyable.h"
#include "god/orm/Result.h"

namespace god
{

// 数据库类型
enum class DbClientType
{
    Mysql,
};

/// 数据库连接信息
struct DbConnInfo : NonCopyable
{
    DbConnInfo(DbClientType type,
              const std::string& host,
              uint16_t port,
              const std::string& dbName,
              const std::string& userName,
              const std::string& password,
              const std::string& characterSet)
    : type(type),
      host(host),
      port(port),
      dbName(dbName),
      userName(userName),
      password(password),
      characterSet(characterSet)
    { }

    // 类型
    const DbClientType type;
    // 主机地址
    const std::string host;
    // 端口
    const uint16_t port;
    // 数据库
    const std::string dbName;
    // 用户名
    const std::string userName;
    // 密码
    const std::string password;
    // 字符集
    const std::string characterSet;
};
using DbConnInfoPtr = std::shared_ptr<DbConnInfo>;

// 查询成功回调
using QueryCallback = std::function<void(const ResultPtr& result)>;

// 查询失败回调
using ExceptCallback = std::function<void(const std::exception& e)>;

class SqlBinder;
using SqlBinderPtr = std::shared_ptr<SqlBinder>;

class SqlBinder : NonCopyable
{
public:
    SqlBinder(DbClientType type, const std::string&& sql)
    : type_(type), sql_(std::move(sql)) { }

    SqlBinder& operator<<(QueryCallback&& cb)
    {
        queryCallback_ = std::move(cb);
        return *this;
    }

    SqlBinder& operator<<(ExceptCallback&& cb)
    {
        exceptCallback_ = std::move(cb);
        return *this;
    }

    SqlBinder& operator<<(std::nullptr_t)
    {
        if (type_ == DbClientType::Mysql)
        {
            paramVec_.emplace_back("NULL");
        }
        return *this;
    }

    SqlBinder& operator<<(const std::string_view& str)
    {
        if (type_ == DbClientType::Mysql)
        {
            std::string ret;
            ret.push_back('\'');
            ret.append(str);
            ret.push_back('\'');
            paramVec_.emplace_back(std::move(ret));
        }
        return *this;
    }

    template<typename T, typename =
        std::enable_if_t<std::is_arithmetic_v<T>>>
    SqlBinder& operator<<(T val)
    {
        char buf[64];
        char* p = buf;

        if constexpr (std::is_integral_v<T>)
        {
            p = std::to_chars(p, buf + sizeof(buf), val).ptr;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            p = std::to_chars(p, buf + sizeof(buf), val,
                    std::chars_format::general).ptr;
        }

        paramVec_.emplace_back(buf, p - buf);
        return *this;
    }

    std::string sql();

    QueryCallback queryCallback()
    {
        return std::move(queryCallback_);
    }

    ExceptCallback exceptCallback()
    {
        return std::move(exceptCallback_);
    }

private:
    DbClientType type_;
    std::string sql_;
    QueryCallback queryCallback_;
    ExceptCallback exceptCallback_;
    std::vector<std::string> paramVec_;
};

} // namespace god

#endif