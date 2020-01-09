//
// Created by luocf on 2019/6/15.
//

#ifndef CHATGROUP_ServiceInfo_H
#define CHATGROUP_ServiceInfo_H


#include <string>
#include <ctime>
#include <memory>

namespace micro_service {
    class ServiceInfo {
    public:
        ServiceInfo();
        ServiceInfo(const std::string& service_addr,const std::string& mnemonic,
                const std::string& info_path, const std::string& path, std::time_t save_time);
        std::string mServiceAddr;
        std::string mMnemonic;
        std::string mInfoPath;
        std::string mPath;
        std::time_t mTimeStamp;
    };
}

#endif //CHATGROUP_ServiceInfo_H
