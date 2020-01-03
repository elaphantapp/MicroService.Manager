//
// Created by luocf on 2019/6/13.
//

#ifndef LAUNCHER_SERVICE_H
#define LAUNCHER_SERVICE_H

#include <stdlib.h>
#include <functional>
#include <memory> // std::unique_ptr
#include <ctime>
#include <thread>
#include <regex>
#include <future>
#include <queue>
#include "Common/FileUtils.hpp"
#include "Connector.h"
#include "PeerListener.h"
#include "PeerNode.h"
#include "Contact.hpp"
#include "ContactListener.hpp"
#include "DataBase/DatabaseProxy.h"
#include "DataBase/ServiceInfo.h"
using namespace elastos;

namespace micro_service {
    static const int MAX_QUEUE_SIZE = 12;
    static const char *ManagerService_TAG = "ManagerService";
    class ManagerService:public std::enable_shared_from_this<ManagerService>{
    public:
        ManagerService(const std::string& path);
        ~ManagerService();
        int acceptFriend(const std::string& friendid);
        void receiveMessage(const std::string& friend_id, const std::string& message, std::time_t send_time);
        void helpCmd(const std::vector<std::string> &args, const std::string& message);
        void startServiceCmd(const std::vector<std::string> &args);
        void createServiceCmd(const std::vector<std::string> &args);
        void startServiceInBackground(const std::string& friend_id, const std::string&service_name,
                const std::string& service_addr);
        void createServiceInBackground(const std::string& friend_id, const std::string&service_name);
        void startServiceInner(const std::string& friend_id, const std::string& mnemonic,  const std::string&service_name,
                               const std::string& service_path,  const std::string& info_file_path);
        std::string getPrivateKey(const std::string& mnemonic);
        std::time_t getTimeStamp();
        std::string mOwnerHumanCode;
    protected:
        std::string mPath;

    private:
        static constexpr const char* KeypairLanguage = "english";
        static constexpr const char* KeypairWords = "";
        static constexpr const char* MnemonicFileName = "mnemonic";
        std::thread mWorkThread;
        std::queue<std::shared_ptr<std::string>> mQueue;
        std::mutex mQueue_lock;
        std::condition_variable mQueue_cond;
        std::condition_variable mWrite_cond;
        Connector* mConnector;
        DatabaseProxy* mDatabaseProxy;
        void bindService(const std::string& friend_id,
                         const std::string& service_name,
                         const std::string& data_path,
                         const std::string& info_path,
                         const std::string& mnemonic);
        void runWorkThread();
        void sendMsgToWorkThread(std::string msg);
        int start();
        int stop();
    };
    
    class ManagerServiceMessageListener :public PeerListener::MessageListener{
    public:
        ManagerServiceMessageListener( ManagerService* ManagerService);
        ~ManagerServiceMessageListener();
        void onEvent(ElaphantContact::Listener::EventArgs& event) override ;
        void onReceivedMessage(const std::string& humanCode, ElaphantContact::Channel channelType,
                               std::shared_ptr<ElaphantContact::Message> msgInfo) override;
    private:
        ManagerService*mManagerService;
    };

    extern "C" {
        micro_service::ManagerService* CreateService(const char* path);
        void DestroyService(micro_service::ManagerService* service);
    }
}

#endif //LAUNCHER_SERVICE_H
