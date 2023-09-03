#ifndef GOD_ORM_DBCLIENTMANAGER_H
#define GOD_ORM_DBCLIENTMANAGER_H

#include <map>
#include <string>
#include <cassert>

#include "god/utils/NonCopyable.h"
#include "god/orm/DbClient.h"

namespace god
{

class DbClientManager : NonCopyable
{
public:
    ~DbClientManager();

    void addDbClient(DbClientType type,
                     const std::string& host,
                     const uint16_t port,
                     const std::string& dbName,
                     const std::string& userName,
                     const std::string& password,
                     const size_t threadNum,
                     const size_t connNum,
                     const std::string& clientName,
                     const std::string& characterSet);
    
    void startDbClient();

    DbClientPtr& getDbClient(const std::string& name)
    {
        assert(clientMap_.count(name));
        return clientMap_[name];
    }

private:
    std::map<std::string, DbClientPtr> clientMap_;
};

} // namespace god::orm

#endif