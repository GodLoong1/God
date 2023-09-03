#include "god/orm/DbClient.h"

#include "god/orm/MysqlConnection.h"
#include "god/orm/DbConnection.h"
#include "god/orm/SqlBinder.h"
#include "god/utils/Logger.h"

namespace god
{

DbClient::DbClient(DbConnInfoPtr&& connInfo,
                   size_t threadNum, size_t connNum)
: connInfo_(std::move(connInfo)),
  threadNum_(threadNum),
  connNum_(connNum),
  loopPool_(threadNum_, "DbLoop")
{
}

DbClient::~DbClient()
{
    assert(quited_);
}

void DbClient::start()
{
    loopPool_.start();
    auto loopVec = loopPool_.getLoops();

    for (auto* loop : loopVec)
    {
        for (size_t i = 0; i != connNum_; ++i)
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            connections_.insert(newConnection(loop));
        }
    }
}

void DbClient::stop()
{
    quited_ = true;
    std::unordered_set<DbConnectionPtr> temp;
    {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connections_.swap(temp);
        readyConnections_.clear();
        busyConnections_.clear();
    }
    for (auto& conn : temp)
    {
        conn->stop();
    }
}

void DbClient::execSql(SqlBinderPtr&& binder)
{
    DbConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(connectionMutex_);

        if (readyConnections_.empty())
        {
            sqlBinders_.emplace_back(std::move(binder));
        }
        else
        {
            auto iter = readyConnections_.begin();
            busyConnections_.insert(*iter);
            conn = *iter;
            readyConnections_.erase(iter);
        }
    }
    if (conn)
    {
        conn->execSql(std::move(binder));
    }
}

DbConnectionPtr DbClient::newConnection(EventLoop* loop)
{
    DbConnectionPtr connPtr;
    if (connInfo_->type == DbClientType::Mysql)
    {
        connPtr = std::make_shared<MysqlConnection>(loop, connInfo_);
    }
    assert(connPtr);

    connPtr->setCloseCallback([this](const DbConnectionPtr& conn) {
        if (quited_)
        {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            // assert(busyConnections_.count(conn));
            busyConnections_.erase(conn);
            assert(connections_.count(conn));
            connections_.erase(conn);
        }

        // 重连
        auto loop = conn->loop();
        loop->runOnce(1, [selfWeak(weak_from_this()), loop, conn] {
            auto self = selfWeak.lock();
            if (!self)
            {
                return;
            }
            LOG_ERROR << "Reconnect DbConnection";
            std::lock_guard<std::mutex> lock(self->connectionMutex_);
            self->connections_.insert(self->newConnection(loop));
        });
    });

    connPtr->setOkCallback([this](const DbConnectionPtr& conn) {
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            busyConnections_.insert(conn);
        }
        handleSqlBinder(conn);
    });

    connPtr->setIdleCallback([this](const DbConnectionPtr& conn){
        handleSqlBinder(conn);
    });

    connPtr->start();

    return connPtr;
}

void DbClient::handleSqlBinder(const DbConnectionPtr& conn)
{
    SqlBinderPtr binder;
    {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        if (!sqlBinders_.empty())
        {
            binder = std::move(sqlBinders_.front());
            sqlBinders_.pop_front();
        }
        else
        {
            busyConnections_.erase(conn);
            readyConnections_.insert(conn);
        }
    }
    if (binder)
    {
        conn->execSql(std::move(binder));
    }
}

} // namespace god