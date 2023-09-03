#ifndef GOD_ORM_DBCONNECTION_H
#define GOD_ORM_DBCONNECTION_H

#include <cstdint>
#include <string>
#include <functional>
#include <memory>

#include "god/orm/SqlBinder.h"
#include "god/net/EventLoop.h"
#include "god/utils/NonCopyable.h"

namespace god
{

class DbConnection;
using DbConnectionPtr = std::shared_ptr<DbConnection>;

class DbConnection : NonCopyable
{
public:
    using DbConnectionCallback =
        std::function<void(const DbConnectionPtr&)>;

    DbConnection(EventLoop* loop, const DbConnInfoPtr& info)
    : loop_(loop), info_(info) { }

    void setOkCallback(const DbConnectionCallback& cb)
    {
        okCallback_ = cb;
    }

    void setIdleCallback(const DbConnectionCallback& cb)
    {
        idleCallback_ = cb;
    }

    void setCloseCallback(const DbConnectionCallback& cb)
    {
        closeCallback_ = cb;
    }

    EventLoop* loop()
    {
        return loop_;
    }

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void execSql(SqlBinderPtr&& binder) = 0;

protected:
    enum class ConnStatus
    {
        Connecting,
        Connected,
        SetCharacter,
        Disconnected,
    };

    EventLoop* loop_;
    DbConnInfoPtr info_;

    std::string sql_;
    QueryCallback queryCallback_;
    ExceptCallback exceptCallback_;
    ConnStatus connStatus_{ConnStatus::Disconnected};

    DbConnectionCallback okCallback_;
    DbConnectionCallback idleCallback_;
    DbConnectionCallback closeCallback_;
};

} // namespace god

#endif