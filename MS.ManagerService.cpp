#include "MS.ManagerService.hpp"
#include "MS.ManagerListener.hpp"
#include "Json.hpp"
#include "Utils/Log.hpp"

namespace elastos {
namespace MicroService {

void ManagerService::init(const std::string& path)
{
    mPath = path;
    mConnector = std::make_shared<Connector>(NAME);
    auto listener = std::make_shared<ManagerListener>(weak_from_this());
    auto castlistener = std::dynamic_pointer_cast<PeerListener::MessageListener>(listener);
    mConnector->SetMessageListener(castlistener);
}

std::shared_ptr<Connector> ManagerService::getConnector()
{
    return mConnector;
}

} // MicroService
} // elastos

extern "C" {

void* CreateService(const char* path)
{
    auto service = new elastos::MicroService::ManagerService();
    service->init(path);
    return static_cast<void *>(service);
}

void DestroyService(void* service)
{
    if (service == nullptr) {
        return;
    }

    auto instance = static_cast<elastos::MicroService::ManagerService*>(service);
    delete instance;
}

}