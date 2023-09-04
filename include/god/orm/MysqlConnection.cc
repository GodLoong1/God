#include "god/orm/MysqlConnection.h"

#include <mariadb/errmsg.h>

#include <future>

#include "god/orm/MysqlResult.h"
#include "god/utils/Logger.h"

namespace god
{

MysqlConnection::MysqlConnection(EventLoop* loop,
                                 const DbConnInfoPtr& connInfo)
: DbConnection(loop, connInfo),
  mysql_(new MYSQL, [](MYSQL* p) { mysql_close(p); delete p; })
{
    static MysqlEnv env;
    static thread_local MysqlThreadEnv threadEnv;

    mysql_init(mysql_.get());
    mysql_options(mysql_.get(), MYSQL_OPT_NONBLOCK, nullptr);
    int reconnect = 1;
    mysql_optionsv(mysql_.get(), MYSQL_OPT_RECONNECT, &reconnect); 
}

MysqlConnection::~MysqlConnection()
{
    assert(connStatus_ == ConnStatus::Disconnected);
}

void MysqlConnection::start()
{
    loop_->runInLoop([self(shared_from_this())] {
        assert(self->okCallback_);
        assert(self->closeCallback_);
        assert(self->idleCallback_);
        assert(self->connStatus_ == ConnStatus::Disconnected);
        self->realConnectStart();
    });
}

void MysqlConnection::stop()
{
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    loop_->runInLoop([self(shared_from_this()), &promise] {
        self->handleClose();
        promise.set_value();
    });

    future.get();
}

void MysqlConnection::execSql(SqlBinderPtr&& binder)
{
    if (loop_->isInLoop())
    {
        execSqlInLoop(std::move(binder));
    }
    else
    {
        loop_->addInLoop([self(shared_from_this()),
                          binder(std::move(binder))] () mutable {
            self->execSqlInLoop(std::move(binder));
        });
    }
}

void MysqlConnection::execSqlInLoop(SqlBinderPtr&& binder)
{
    loop_->assertInLoop();

    sql_ = binder->sql();
    queryCallback_ = binder->queryCallback();
    exceptCallback_ = binder->exceptCallback();

    std::string to(sql_.size() * 2 + 1, '\0');
    auto len = mysql_real_escape_string(mysql_.get(),
                                        to.data(),
                                        sql_.data(),
                                        sql_.size());
    to.resize(len);
    sql_ = std::move(to);

    LOG_TRACE << "sql: " << sql_;

    realQueryStart();
}

void MysqlConnection::handleEvent()
{
    loop_->assertInLoop();
    assert(connStatus_ != ConnStatus::Disconnected);

    int status = 0;
    uint32_t revents = channel_->revents();

    if (revents & EPOLLIN)
        status |= MYSQL_WAIT_READ;
    if (revents & EPOLLOUT)
        status |= MYSQL_WAIT_WRITE;
    if (revents & EPOLLPRI)
        status |= MYSQL_WAIT_EXCEPT;

    if (connStatus_ == ConnStatus::Connected)
    {
        handleExec(status);
    }
    else if (connStatus_ == ConnStatus::Connecting)
    {
        realConnectCont(status);
    }
    else if (connStatus_ == ConnStatus::SetCharacter)
    {
        setCharacterCont(status);
    }
}

void MysqlConnection::handleExec(int status)
{
    loop_->assertInLoop();
    assert(execStatus_ != ExecStatus::None);

    if (execStatus_ == ExecStatus::RealQuery)
    {
        realQueryCont(status);
    }
    else if (execStatus_ == ExecStatus::StoreResult)
    {
        storeResultCont(status);
    }
    else if (execStatus_ == ExecStatus::NextResult)
    {
        nextResultCont(status);
    }
}

void MysqlConnection::handleClose()
{
    loop_->assertInLoop();
    assert(connStatus_ != ConnStatus::Disconnected);
    connStatus_ = ConnStatus::Disconnected;

    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void MysqlConnection::setChannel()
{
    loop_->assertInLoop();
    LOG_TRACE << "waitStatus: " << waitStatus_;

    if ((waitStatus_ & MYSQL_WAIT_READ) ||
        (waitStatus_ & MYSQL_WAIT_EXCEPT))
    {
        if (!channel_->isReading())
            channel_->enableReading();
    }
    if (waitStatus_ & MYSQL_WAIT_WRITE)
    {
        if (!channel_->isWriting())
            channel_->enableWriting();
    }
    else
    {
        if (channel_->isWriting())
            channel_->disableWriting();
    }
}

void MysqlConnection::realConnectStart()
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::Disconnected);
    connStatus_ = ConnStatus::Connecting;

    MYSQL* err;
    waitStatus_ = mysql_real_connect_start(&err,
                                           mysql_.get(),
                                           info_->host.c_str(),
                                           info_->userName.c_str(),
                                           info_->password.c_str(),
                                           info_->dbName.c_str(),
                                           info_->port,
                                           nullptr,
                                           CLIENT_MULTI_STATEMENTS);
    LOG_TRACE << "mysql_real_connect_start: " << waitStatus_;

    auto fd = mysql_get_socket(mysql_.get());
    if (fd < 0)
    {
        LOG_FATAL << "mysql_get_socket error";
    }

    channel_.reset(new Channel(loop_, mysql_get_socket(mysql_.get())));
    channel_->setEventCallback([this] { handleEvent(); });

    realConnectCheck(err);
}

void MysqlConnection::realConnectCont(int status)
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::Connecting);

    MYSQL* err;
    waitStatus_ = mysql_real_connect_cont(&err, mysql_.get(), status);
    LOG_TRACE << "mysql_real_connect_cont: " << waitStatus_;

    realConnectCheck(err);
}

void MysqlConnection::realConnectCheck(MYSQL* err)
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::Connecting);

    if (waitStatus_ == 0)
    {
        if (!err)
        {
            connError("mysql_real_connect()");
        }
        else
        {
            connStatus_ = ConnStatus::Connected;
            if (info_->characterSet.empty())
            {
                okCallback_(shared_from_this());
            }
            else
            {
                setCharacterStart();
            }
        }
    }
    else
    {
        setChannel();
    }
}

void MysqlConnection::setCharacterStart()
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::Connected);
    connStatus_ = ConnStatus::SetCharacter;

    int err;
    waitStatus_ = mysql_set_character_set_start(&err,
                                                mysql_.get(),
                                                info_->characterSet.c_str());
    LOG_TRACE << "mysql_set_character_set_start: " << waitStatus_;

    setCharacterCheck(err);
}

void MysqlConnection::setCharacterCont(int status)
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::SetCharacter);

    int err;
    waitStatus_ = mysql_set_character_set_cont(&err,
                                               mysql_.get(),
                                               status);
    LOG_TRACE << "mysql_set_character_set_cont: " << waitStatus_;

    setCharacterCheck(err);
}

void MysqlConnection::setCharacterCheck(int err)
{
    loop_->assertInLoop();
    assert(connStatus_ == ConnStatus::SetCharacter);

    if (waitStatus_ == 0)
    {
        if (err)
        {
            connError("mysql_set_character_set()");
        }
        else
        {
            connStatus_ = ConnStatus::Connected;
            okCallback_(shared_from_this());
        }
    }
    else
    {
        setChannel();
    }
}

void MysqlConnection::realQueryStart()
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::None);
    execStatus_ = ExecStatus::RealQuery;

    int err;
    waitStatus_ = mysql_real_query_start(&err,
                                         mysql_.get(),
                                         sql_.c_str(),
                                         sql_.size());
    LOG_TRACE << "mysql_real_query_start: " << waitStatus_;

    realQueryCheck(err);
}

void MysqlConnection::realQueryCont(int status)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::RealQuery);

    int err;
    waitStatus_ = mysql_real_query_cont(&err, mysql_.get(), status);
    LOG_TRACE << "mysql_real_query_cont: " << waitStatus_;

    realQueryCheck(err);
}

void MysqlConnection::realQueryCheck(int err)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::RealQuery);

    if (waitStatus_ == 0)
    {
        if (err)
        {
            execError("mysql_real_query()");
        }
        else
        {
            storeResultStart();
        }
    }
    else
    {
        setChannel();
    }
}

void MysqlConnection::storeResultStart()
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::RealQuery ||
           execStatus_ == ExecStatus::NextResult);
    execStatus_ = ExecStatus::StoreResult;

    MYSQL_RES* ret;
    waitStatus_ = mysql_store_result_start(&ret, mysql_.get());
    LOG_TRACE << "mysql_store_result_start: " << waitStatus_;

    storeResultCheck(ret);
}

void MysqlConnection::storeResultCont(int status)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::StoreResult);

    MYSQL_RES* ret;
    waitStatus_ = mysql_store_result_cont(&ret, mysql_.get(), status);
    LOG_TRACE << "mysql_store_result_cont: " << waitStatus_;

    storeResultCheck(ret);
}

void MysqlConnection::storeResultCheck(MYSQL_RES* ret)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::StoreResult);

    if (waitStatus_ == 0)
    {
        if (!ret)
        {
            execError("mysql_store_result");
        }
        else
        {
            ResultPtr result(new MysqlResult(ret, mysql_affected_rows(mysql_.get())));
            if (queryCallback_)
            {
                queryCallback_(result);
            }

            if (!mysql_more_results(mysql_.get()))
            {
                execStatus_ = ExecStatus::None;
                idleCallback_(shared_from_this());
            }
            else
            {
                nextResultStart();
            }
        }
    }
    else
    {
        setChannel();
    }
}

void MysqlConnection::nextResultStart()
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::StoreResult);
    execStatus_ = ExecStatus::NextResult;

    int err;
    waitStatus_ = mysql_next_result_start(&err, mysql_.get());
    LOG_TRACE << "mysql_next_result_start: " << waitStatus_;

    nextResultCheck(err);
}

void MysqlConnection::nextResultCont(int status)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::NextResult);

    int err;
    waitStatus_ = mysql_next_result_cont(&err, mysql_.get(), status);
    LOG_TRACE << "mysql_next_result_cont: " << waitStatus_;

    nextResultCheck(err);
}

void MysqlConnection::nextResultCheck(int err)
{
    loop_->assertInLoop();
    assert(execStatus_ == ExecStatus::NextResult);

    if (waitStatus_ == 0)
    {
        if (err)
        {
            execError("mysql_next_result");
        }
        else
        {
            storeResultStart();
        }
    }
    else
    {
        setChannel();
    }
}

void MysqlConnection::connError(const char* func)
{
    loop_->assertInLoop();
    assert(connStatus_ != ConnStatus::Disconnected);

    LOG_ERROR << "Error(" << mysql_errno(mysql_.get())
              << ") \"" << mysql_error(mysql_.get()) << "\" "
              << "Failed to " << func;
    handleClose();
}

void MysqlConnection::execError(const char* func)
{
    loop_->assertInLoop();
    assert(execStatus_ != ExecStatus::None);
    execStatus_ = ExecStatus::None;

    auto errorNo = mysql_errno(mysql_.get());
    LOG_ERROR << "Error(" << errorNo << ") ["
              << mysql_sqlstate(mysql_.get())
              << "] \"" << mysql_error(mysql_.get()) << "\", "
              << "sql: \"" << sql_ << "\", Failed to " << func;

    if (exceptCallback_)
    {
        exceptCallback_(std::runtime_error(mysql_error(mysql_.get())));
    }

    if (errorNo != CR_SERVER_GONE_ERROR && errorNo != CR_SERVER_LOST)
    {
        idleCallback_(shared_from_this());
    }
    else
    {
        handleClose();
    }
}

} // namepsace god