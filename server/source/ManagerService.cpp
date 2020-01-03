//
// Created by luocf on 2019/6/13.
//
#include <cstring>
#include <future>
#include <cstdlib>
#include <stack>
#include <iostream>
#include <ctime>

using namespace std;
#include <thread>
#include <chrono>
#include <Tools/Log.hpp>
#include "Json.hpp"
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>
#include <ThirdParty/CompatibleFileSystem.hpp>
#include "Command/ManagerServiceCmd.hpp"
#include "ManagerService.h"
#include "ErrCode.h"
namespace micro_service {
    /***********************************************/
    /***** static function implement ***************/
    /***********************************************/
    static std::string toLower(const std::string& str)
    {
        std::string ret(str.size(), 'a');
        std::transform(str.begin(), str.end(), ret.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return ret;
    }
    ManagerService::ManagerService(const std::string& path)
            : mPath(path){
        mConnector = new Connector(ManagerService_TAG);
        this->start();
    }
    
    ManagerService::~ManagerService() {
        this->stop();
    }

    int ManagerService::start() {
        if (mConnector == NULL) return -1;
        printf("Service start!\n");
        std::shared_ptr<PeerListener::MessageListener> message_listener = std::make_shared<ManagerServiceMessageListener>(this);
        mConnector->SetMessageListener(message_listener);
        auto status = PeerNode::GetInstance()->GetStatus();
        printf("ManagerService Start status: %d\n", static_cast<int>(status));
        std::shared_ptr<ElaphantContact::UserInfo> user_info = mConnector->GetUserInfo();
        if (user_info.get() != NULL) {
            user_info->getHumanCode(mOwnerHumanCode);
            printf("Service start mOwnerHumanCode:%s\n", mOwnerHumanCode.c_str());
        }
        mDatabaseProxy = new DatabaseProxy();
        mDatabaseProxy->startDb(mPath.c_str());

        mWorkThread = std::thread(&ManagerService::runWorkThread, this); //引用
        return 0;
    }

    void ManagerService::createServiceInBackground(const std::string& friend_id,
            const std::string&service_name) {
        //create mnemonic and generate private
        std::string mnemonic = ::generateMnemonic(KeypairLanguage, KeypairWords);
        //generate random dir
        unsigned seed = this->getTimeStamp();
        srand(seed);
        std::string service_path = mPath +"/"+std::to_string(random())+"/";
        std::string info_file_path = service_path +"/info.txt";
        FileUtils::mkdirs(service_path.c_str(), 0777);
        this->startServiceInner(friend_id, mnemonic, service_name, service_path, info_file_path);
    }

    void ManagerService::startServiceInBackground(const std::string& friend_id,
                                                  const std::string&service_name, const std::string& human_code) {
        std::shared_ptr<ServiceInfo> service_info = mDatabaseProxy->getServiceInfo(friend_id, human_code);
        std::string mnemonic = "";
        std::string service_path = "";
        std::string info_file_path = "";
        if (service_info.get() != nullptr) {
            mnemonic = service_info->mMnemonic;
            service_path = service_info->mPath;
            info_file_path = service_info->mInfoPath;
        }
        this->startServiceInner(friend_id, mnemonic, service_name, service_path, info_file_path);
    }
    void ManagerService::startServiceInner(const std::string& friend_id,
            const std::string& mnemonic,
            const std::string& service_name,
            const std::string& service_path,
            const std::string& info_file_path) {

        //launcher service
        std::async(&ManagerService::bindService, this, friend_id, service_name,
                   service_path, info_file_path, mnemonic);
    }

    std::string ManagerService::getPrivateKey(const std::string& mnemonic){
        void* seedData = nullptr;
        int seedSize = ::getSeedFromMnemonic(&seedData, mnemonic.c_str(), KeypairLanguage);
        auto privKey = ::getSinglePrivateKey(seedData, seedSize);
        freeBuf(seedData);
        std::string retval = privKey;
        freeBuf(privKey);
        return retval;
    }

    void ManagerService::bindService(
            const std::string& friend_id,
            const std::string& service_name,
            const std::string& data_path,
            const std::string& info_path,
            const std::string& mnemonic) {
        std::string private_key = this->getPrivateKey(mnemonic);
        //get libxxxservice.so path by service_name
        std::string library_path = "";
        std::string service_name_tolower = toLower(service_name);
        std::string error_msg = "success";
        int ret_status = 0;
        if (service_name_tolower == "personalstorage") {
            library_path = "./libs/libLocalStorageService.so";
        } else if (service_name_tolower == "chatgroup") {
            library_path = "./libs/libChatGroupService.so";
        } else if (service_name_tolower == "hashaddressmapping") {
            library_path = "./libs/libHashAddressMappingService.so";
        } else {
            ret_status = -1;
            error_msg = "error service name is unknown";
            printf("service_name is unknown\n");
        }

        char *nargv[] = {(char*)"PeerNodeLauncher",
                         (char*)"-name", (char *) library_path.c_str(),
                         (char*)"-path", (char *) data_path.c_str(),
                         (char*)"-key", (char *) private_key.c_str(),
                         (char*)"-info", (char *) info_path.c_str(),
                         (char*) 0}; //命令行参数都以0结尾
        Log::I(ManagerService_TAG,"bindService execv: %s %s %s %s %s %s %s %s %s", nargv[0],nargv[1],nargv[2],nargv[3],nargv[4],nargv[5],nargv[6],nargv[7],nargv[8]);
        pid_t pid;
        pid = fork();
        switch (pid) {
            case 0:
                execv("./bin/PeerNodeLauncher", nargv);
                perror("exec");
                exit(1);
                break;
            case -1:
                perror("fork");
                exit(1);
                break;
            default:
                printf("exec is completed\n");
                std::string service_info = "";
                if (!library_path.empty()) {
                    //sleep 4s
                    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
                    //read info file for service address info
                    std::shared_ptr <uint8_t> data = std::make_shared<uint8_t>(1024*1024);
                    int length = FileUtils::readFromFile(info_path.c_str(), data);
                    if (length != 0) {
                        service_info = std::string(data.get(), data.get()+length);
                        Log::I(ManagerService_TAG,
                               "startServiceInner service_info: %s",
                               service_info.c_str());
                        auto service_info_json = Json::parse(service_info.c_str());
                        //save data to db
                        auto humaninfo = service_info_json["HumanInfo"];
                        auto human_code  = humaninfo["HumanCode"];
                        mDatabaseProxy->setServiceInfo(friend_id, human_code, mnemonic,
                                                       info_path, data_path, this->getTimeStamp());
                    } else {
                        ret_status = -2;
                        error_msg = "error service info file not exist!";
                    }
                }
                //send msg to client
                Json content;
                content["status"] = ret_status;
                content["msg"] = error_msg;
                content["info"] = service_info;
                content["service_name"] = service_name;
                Json respJson;
                respJson["serviceName"] = ManagerService_TAG;
                respJson["type"] = "textMsg";
                respJson["content"] = content;
                std::string message = respJson.dump();
                int ret = mConnector->SendMessage(friend_id, message);
                if (ret != 0) {
                    Log::I(ManagerService_TAG,
                           "startServiceInner .c_str(): %s error:%d",
                           message.c_str(), ret);
                }
                break;
        }
    }
    void ManagerService::runWorkThread() {
        printf("manager::runWorkThread in\n");
        int iPid = (int)getpid();
        while (true) {
            printf("runWorkThread,  lk(mQueue_lock) iPid:%d\n", iPid);
            std::unique_lock<std::mutex> lk(mQueue_lock);
            printf("runWorkThread,  mQueue_cond.wait;  mQueue.empty():%d, iPid:%d\n", mQueue.empty()?1:0, iPid);
            mQueue_cond.wait(lk, [this] { return !mQueue.empty(); });
            std::shared_ptr<std::string> msg = mQueue.front();
            printf("runWorkThread,  mQueue.pop(), iPid:%d\n", iPid);
            mQueue.pop();
            printf("runWorkThread,  lk.unlock(), iPid:%d\n", iPid);
            lk.unlock();
            mWrite_cond.notify_one();
            try {
                auto msg_json = Json::parse(msg->c_str());
                printf("runWorkThread, _createService msg->c_str():%s\n", msg->c_str());
                int cmd = msg_json["cmd"];
                switch (cmd) {
                    case Command_Create: {
                        this->createServiceInBackground(msg_json["friend_id"], msg_json["service_name"]);
                        break;
                    }
                    case Command_Restart: {
                        this->startServiceInBackground(msg_json["friend_id"], msg_json["service_name"], msg_json["human_code"]);
                        break;
                    }
                }
            } catch (std::exception &e) {
                std::cout << "[exception caught: " << e.what() << "]\n";
            }
            printf("runWorkThread,  mWrite_cond.notify_all(); iPid:%d\n", iPid);
        }
        printf("manager::runWorkThread out\n");
    }
    int ManagerService::stop() {
        mDatabaseProxy->closeDb();
        if (mConnector == NULL) return -1;
        printf("Service stop!\n");
        return 0;
    }
    std::time_t ManagerService::getTimeStamp() {
        return time(0);
    }

    void ManagerService::receiveMessage(const std::string& friend_id, const std::string& message, std::time_t send_time) {
        std::string errMsg;
        std::string msg = message;
        std::string pre_cmd = msg + " " + friend_id;//Pretreatment cmd
        ManagerServiceCmd::Do(this, pre_cmd, errMsg);
    }

    int ManagerService::acceptFriend(const std::string& friendid) {
        mConnector->AcceptFriend(friendid);
        mDatabaseProxy->createTable(friendid);
        return 0;
    }

    void ManagerService::helpCmd(const std::vector<std::string> &args, const std::string &message) {
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            Json respJson;
            respJson["serviceName"] = ManagerService_TAG;
            respJson["type"] = "textMsg";
            respJson["content"] = message;
            int ret = mConnector->SendMessage(friend_id, respJson.dump());
            if (ret != 0) {
                Log::I(ManagerService_TAG,
                       "helpCmd .c_str(): %s",
                       message.c_str());
            }
        }
    }

    void ManagerService::startServiceCmd(const std::vector<std::string> &args) {
        if (args.size() >= 4) {
            //获取服务信息，根据服务信息启动service，发送到WorkThread
            Json respJson;
            respJson["cmd"] = Command_Restart;
            respJson["friend_id"] = args[3];
            respJson["service_name"] = args[1];
            respJson["human_code"] = args[2];
            sendMsgToWorkThread(respJson.dump());
        }
    }

    void ManagerService::createServiceCmd(const std::vector<std::string> &args) {
        if (args.size() >= 3) {
            //获取服务信息，根据服务信息启动service，发送到WorkThread
            Json respJson;
            respJson["cmd"] = Command_Create;
            respJson["friend_id"] = args[2];
            respJson["service_name"] = args[1];
            sendMsgToWorkThread(respJson.dump());
        }
    }

    void ManagerService::sendMsgToWorkThread(std::string msg) {
        int iPid = (int)getpid();
        printf("sendMsgToWorkThread, iPid:%d, msg:%s\n", iPid, msg.c_str());
        std::unique_lock<std::mutex> lk(mQueue_lock);
        mWrite_cond.wait(lk, [this] { return mQueue.size() < MAX_QUEUE_SIZE; });
        mQueue.push(std::make_shared<std::string>(msg));
        lk.unlock();
        mQueue_cond.notify_one();
    }

    ManagerServiceMessageListener::ManagerServiceMessageListener(ManagerService* service) {
        mManagerService = service;
    }

    ManagerServiceMessageListener::~ManagerServiceMessageListener() {

    }

    void ManagerServiceMessageListener::onEvent( ElaphantContact::Listener::ContactListener::EventArgs& event) {
        Log::W(ManagerService_TAG, "onEvent type: %d\n", event.type);
        switch (event.type) {
            case ElaphantContact::Listener::EventType::FriendRequest: {
                auto friendEvent = dynamic_cast<ElaphantContact::Listener::RequestEvent*>(&event);
                Log::W(ManagerService_TAG, "FriendRequest from: %s\n", friendEvent->humanCode.c_str());
                mManagerService->acceptFriend(friendEvent->humanCode);
                break;
            }
            case ElaphantContact::Listener::EventType::StatusChanged: {
                auto statusEvent = dynamic_cast<ElaphantContact::Listener::StatusEvent*>(&event);
                Log::I(ManagerService_TAG, "StatusChanged from: %s, statusEvent->status:%d\n", statusEvent->humanCode.c_str(), static_cast<int>(statusEvent->status));
                break;
            }
            case ElaphantContact::Listener::EventType::HumanInfoChanged:{
                auto infoEvent = dynamic_cast<ElaphantContact::Listener::InfoEvent*>(&event);
                Log::I(ManagerService_TAG, "HumanInfoChanged from: %s\n", infoEvent->humanCode.c_str());
                break;
            }
            default: {
                break;
            }
        }
    };

    void ManagerServiceMessageListener::onReceivedMessage(const std::string& humanCode, ElaphantContact::Channel channelType,
                                                     std::shared_ptr<ElaphantContact::Message> msgInfo) {
        auto text_data = dynamic_cast<ElaphantContact::Message::TextData*>(msgInfo->data.get());
        std::string content = text_data->toString();
        try {
            Json json = Json::parse(content);
            std::string msg_content = json["content"];
            printf("ManagerServiceMessageListener onReceivedMessage humanCode: %s,msg_content:%s \n", humanCode.c_str(), msg_content.c_str());
            mManagerService->receiveMessage(humanCode,
                                          msg_content, mManagerService->getTimeStamp());
        } catch (const std::exception& e) {
            printf("ManagerServiceMessageListener parse json failed\n");
        }
    }

    extern "C" {
    micro_service::ManagerService* CreateService(const char* path) {
        return new micro_service::ManagerService(path);
    }
    void DestroyService(micro_service::ManagerService* service) {
        if (service) {
            delete service;
        }
    }
    }
}