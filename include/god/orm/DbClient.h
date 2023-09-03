#ifndef GOD_ORM_DBCLIENT_H
#define GOD_ORM_DBCLIENT_H

#include "god/net/EventLoopThreadPool.h"
#include "god/utils/NonCopyable.h"
#include "god/orm/SqlBinder.h"
#include "god/orm/DbConnection.h"

#include <future>
#include <memory>
#include <mutex>
#include <deque>
#include <unordered_set>

namespace god
{

class DbClient;
using DbClientPtr = std::shared_ptr<DbClient>;

class DbClient : NonCopyable,
                 public std::enable_shared_from_this<DbClient>
{
public:
    DbClient(DbConnInfoPtr&& connInfo, size_t threadNum, size_t connNum);
    ~DbClient();

    void start();
    void stop();

    template<typename... Args>
    void execSqlAsync(std::string&& sql, Args&&... args)
    {
        SqlBinderPtr binder(new SqlBinder(connInfo_->type, std::move(sql)));
        ((*binder << std::forward<Args>(args)), ...);

        execSql(std::move(binder));
    }

private:
    void execSql(SqlBinderPtr&& binder);

    DbConnectionPtr newConnection(EventLoop* loop);
    void handleSqlBinder(const DbConnectionPtr& connPtr);

private:
    DbConnInfoPtr connInfo_;
    size_t threadNum_;
    size_t connNum_;
    EventLoopThreadPool loopPool_;
    std::atomic<bool> quited_{false};

    std::mutex connectionMutex_;
    std::unordered_set<DbConnectionPtr> connections_;
    std::unordered_set<DbConnectionPtr> readyConnections_;
    std::unordered_set<DbConnectionPtr> busyConnections_;

    std::deque<SqlBinderPtr> sqlBinders_;
};

} // namespace god

#endif