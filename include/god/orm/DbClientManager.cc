#include "god/orm/DbClientManager.h"

#include "god/utils/Logger.h"

namespace god
{

DbClientManager::~DbClientManager()
{
    for (auto& [name, client]: clientMap_)
    {
        LOG_TRACE << "DbClient: " << name << " stop !!!";
        client->stop();
    }
}

void DbClientManager::addDbClient(DbClientType type,
                                  const std::string& host,
                                  const uint16_t port,
                                  const std::string& dbName,
                                  const std::string& userName,
                                  const std::string& password,
                                  const size_t threadNum,
                                  const size_t connNum,
                                  const std::string& clientName,
                                  const std::string& characterSet)
{
    DbConnInfoPtr info(new DbConnInfo(type, host, port, dbName,
        userName, password, characterSet));

    DbClientPtr client(
        new DbClient(std::move(info), threadNum, connNum));

    clientMap_[clientName] = std::move(client);
}

void DbClientManager::startDbClient()
{
    for (auto& [name, client]: clientMap_)
    {
        LOG_TRACE << "DbClient: " << name << " start !!!";
        client->start();
    }
}

} // namespace god