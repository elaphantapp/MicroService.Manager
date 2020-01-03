//
// Created by luocf on 2019/6/15.
//

#include "ServiceInfo.h"
namespace micro_service {
    ServiceInfo::ServiceInfo() {

    }
    ServiceInfo::ServiceInfo(const std::string& service_addr,const std::string& mnemonic,
                             const std::string& info_path, const std::string& path, std::time_t save_time):
            mServiceAddr(service_addr), mMnemonic(mnemonic),mInfoPath(info_path), mPath(path),mTimeStamp(save_time){

    }
}