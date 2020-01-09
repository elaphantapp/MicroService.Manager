//
// Created by luocf on 2019/6/12.
//
#include <map>
#include <iostream>
#include <string.h>
#include <Tools/Log.hpp>
#include "DatabaseProxy.h"
namespace micro_service {
    DatabaseProxy::DatabaseProxy(){
    }
    DatabaseProxy::~DatabaseProxy() {
    }
    void DatabaseProxy::setServiceInfo(const std::string &friendid, const std::string &human_code,
                                       const std::string &mnemonic, const std::string &info_path,
                                       const std::string &path, std::time_t time_stamp) {
        std::shared_ptr<ServiceInfo> service_info = getServiceInfo(friendid, human_code);
        MUTEX_LOCKER locker_sync_data(_SyncedInfo);
        //查询一条记录
        char *errMsg = NULL;
        std::string t_strSql;
        if (service_info.get() == nullptr) {
            t_strSql = "insert into table_"+friendid+" values(NULL,'"+human_code+"'";
            t_strSql += ",'"+mnemonic+"'";
            t_strSql += ",'"+info_path+"'";
            t_strSql += ",'"+path+"'";
            t_strSql += ","+std::to_string(time_stamp)+"";
            t_strSql += ");";
        } else {
            t_strSql = "update table_"+friendid+" set mnemonic='"+mnemonic+"'";
            t_strSql += ",info_path='"+info_path+"'";
            t_strSql += ",path='"+path+"'";
            t_strSql += ",time_stamp="+std::to_string(time_stamp);
            t_strSql += " where human_code='"+human_code+"';";
        }

        //消息直接入库
        int rv = sqlite3_exec(mDb, t_strSql.c_str(), callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::I(DatabaseProxy::TAG, "SQLite setServiceInfo error: %s\n",
                   errMsg);
        } else {
            Log::I(TAG, "setServiceInfo, successful sql:%s", t_strSql.c_str());
        }
    }
    std::shared_ptr<ServiceInfo> DatabaseProxy::getServiceInfo(const std::string& friendid,
                                                              const std::string& human_code) {
        MUTEX_LOCKER locker_sync_data(_SyncedInfo);
        std::shared_ptr<ServiceInfo> service_info;
        //查询一条记录
        char **azResult;   //二维数组存放结果
        char *errMsg = NULL;
        int nrow;          /* Number of result rows written here */
        int ncolumn;
        std::string t_strSql;
        t_strSql = "select * from table_"+friendid+" where human_code=='"+human_code+"';";
        /*step 2: sql语句对象。*/
        sqlite3_stmt *pStmt;
        int rc = sqlite3_prepare_v2(
                mDb, //数据库连接对象
                t_strSql.c_str(), //指向原始sql语句字符串
                strlen(t_strSql.c_str()), //
                &pStmt,
                NULL
        );
        if (rc != SQLITE_OK) {
            Log::I(TAG, "sqlite3_prepare_v2 error:");
            return service_info;
        }

        rc = sqlite3_get_table(mDb, t_strSql.c_str(), &azResult, &nrow, &ncolumn, &errMsg);
        if (rc == SQLITE_OK) {
            Log::I(TAG, "getServiceInfo, successful sql:%s", t_strSql.c_str());
        } else {
            Log::I(TAG, "getServiceInfo, Can't get table: %s", sqlite3_errmsg(mDb));
            return service_info;
        }

        if (nrow != 0 && ncolumn != 0) {     //有查询结果,不包含表头所占行数
            for (int i = nrow; i >=1; i--) {        // 第0行为数据表头
                service_info = std::make_shared<ServiceInfo>( std::string(azResult[6*i + 1]),
                                                              std::string(azResult[6*i + 2]),
                                                            std::string(azResult[6*i + 3]),
                                                            std::string(azResult[6*i + 4]),
                                                              atol(azResult[6*i + 5]));
                Log::I(TAG, "getServiceInfo, service addr:%s", service_info->mServiceAddr.c_str());
                break;
            }
        }
        sqlite3_free_table(azResult);
        sqlite3_finalize(pStmt);     //销毁一个SQL语句对象
        return service_info;
    }

    int  DatabaseProxy::createTable(const std::string& friend_id) {
        MUTEX_LOCKER locker_sync_data(_SyncedInfo);
        std::string create_table = "CREATE TABLE IF NOT EXISTS table_"+friend_id+"(id INTEGER PRIMARY KEY AUTOINCREMENT, human_code TEXT NOT NULL, mnemonic TEXT, info_path TEXT NOT NULL, path TEXT NOT NULL, timestamp INTEGER)";
        char *errMsg = NULL;
        int rv = sqlite3_exec(mDb, create_table.c_str(), callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::I(DatabaseProxy::TAG, "SQLite create_table statement execution error: %s\n",
                   errMsg);
            return 1;
        }
        return 0;
    }

    int DatabaseProxy::callback(void *context, int argc, char **argv, char **azColName) {
        auto database_proxy = reinterpret_cast<DatabaseProxy *>(context);
        int i;
        for (i = 0; i < argc; ++i) {
            Log::I(DatabaseProxy::TAG, "database %s = %s\n", azColName[i],
                   argv[i] ? argv[i] : "NULL");
        }
        return 0;
    }

    bool DatabaseProxy::closeDb() {
        if (mDb != nullptr) {
            sqlite3_close(mDb);
        }
        return true;
    }

    bool DatabaseProxy::startDb(const char *data_dir) {
        std::string strConn = std::string(data_dir) + "/manager.db";
        char *errMsg;
        //打开一个数据库，如果改数据库不存在，则创建一个名字为databaseName的数据库文件
        int rv;
        rv = sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
        if (rv != SQLITE_OK) {
            Log::I(DatabaseProxy::TAG, "sqlite3_config error: %d\n", rv);
            return 1;
        }
        rv = sqlite3_open(strConn.c_str(), &mDb);
        if (rv != SQLITE_OK) {
            Log::I(DatabaseProxy::TAG, "Cannot open database: %s\n", sqlite3_errmsg(mDb));
            sqlite3_close(mDb);
            return 1;
        }
        return 0;
    }
}
