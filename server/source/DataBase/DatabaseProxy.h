//
// Created by luocf on 2019/6/12.
//

#ifndef LOCALSTORGE_DB_PROXY_H
#define LOCALSTORGE_DB_PROXY_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <sqlite3.h>
#include "ServiceInfo.h"
namespace micro_service {
    typedef std::lock_guard<std::mutex> MUTEX_LOCKER;
    class DatabaseProxy : std::enable_shared_from_this<DatabaseProxy> {
    public:
        DatabaseProxy();
        virtual ~DatabaseProxy();
        static constexpr const char *TAG = "DatabaseProxy";
        void setServiceInfo(const std::string& friendid,
                              const std::string& human_code,
                              const std::string& mnemonic,
                                const std::string& info_path,
                                const std::string& path,
                              std::time_t time_stamp);

        std::shared_ptr<ServiceInfo> getServiceInfo(const std::string& friendid,
                                     const std::string& human_code);

        int createTable(const std::string& friend_id);
        bool startDb(const char *data_dir);
        bool closeDb();
        static int callback(void *context, int argc, char **argv, char **azColName);
    private:
        std::mutex _SyncedInfo;
        sqlite3 *mDb;
    };
}
#endif //LOCALSTORGE_DB_PROXY_H
