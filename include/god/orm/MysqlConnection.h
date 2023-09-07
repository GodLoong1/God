#ifndef GOD_ORM_MYSQLCONNECTION_H
#define GOD_ORM_MYSQLCONNECTION_H

#include <mariadb/mysql.h>

#include "god/orm/DbConnection.h"
#include "god/net/Channel.h"

namespace god
{

class MysqlConnection;
using MysqlConnectionPtr = std::shared_ptr<MysqlConnection>;

class MysqlConnection : public DbConnection,
                        public std::enable_shared_from_this<MysqlConnection>
{
public:
    using ConnectionCallback =
        std::function<void(const MysqlConnectionPtr&)>;

    MysqlConnection(EventLoop* loop, const DbConnInfoPtr& connInfo);
    ~MysqlConnection();

    void start() override;
    void stop() override;

    void execSql(SqlBinderPtr&& binder) override;

private:
    struct MysqlEnv
    {
        MysqlEnv() noexcept
        {
            mysql_library_init(0, nullptr, nullptr);
        }
        ~MysqlEnv() noexcept
        {
            mysql_library_end();
        }
    };

    struct MysqlThreadEnv
    {
        MysqlThreadEnv() noexcept
        {
            mysql_thread_init();
        }
        ~MysqlThreadEnv() noexcept
        {
            mysql_thread_end();
        }
    };

    enum class ExecStatus
    {
        None,
        RealQuery,
        StoreResult,
        NextResult,
    };

    void execSqlInLoop(SqlBinderPtr&& binder);

    void handleEvent();
    void handleExec(int status);
    void handleClose();
    void setChannel();

    // 发起连接
    void realConnectStart();
    void realConnectCont(int status);
    void realConnectCheck(MYSQL* err);

    // 设置字符集
    void setCharacterStart();
    void setCharacterCont(int status);
    void setCharacterCheck(int err);

    // 开始查询
    void realQueryStart();
    void realQueryCont(int status);
    void realQueryCheck(int err);

    // 获取结果
    void storeResultStart();
    void storeResultCont(int status);
    void storeResultCheck(MYSQL_RES* ret);

    // 获取下一个结果
    void nextResultStart();
    void nextResultCont(int status);
    void nextResultCheck(int err);

    // 错误处理
    void connError(const char* func);
    void execError(const char* func);

private:
    std::shared_ptr<MYSQL> mysql_;
    std::unique_ptr<Channel> channel_;
    int waitStatus_{0};
    ExecStatus execStatus_{ExecStatus::None};
};

} // namespace god

#endif