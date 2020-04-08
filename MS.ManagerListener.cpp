#include "MS.ManagerListener.hpp"
#include "Json.hpp"
#include "Utils/Log.hpp"

namespace elastos {
namespace MicroService {

ManagerListener::ManagerListener(std::weak_ptr<ManagerService> service)
    : mService(service)
{
}
ManagerListener::~ManagerListener()
{
    mService.reset();
}

void ManagerListener::onEvent(ElaphantContact::Listener::EventArgs& event)
{
    switch (event.type) {
    case ElaphantContact::Listener::EventType::StatusChanged:
    {
        auto statusEvent = dynamic_cast<ElaphantContact::Listener::StatusEvent*>(&event);
        Log::I(ManagerService::NAME, "Service %s received %s status changed %u\n",
                                     ManagerService::NAME,
                                     event.humanCode.c_str(),
                                     statusEvent->status);
        break;
    }
    case ElaphantContact::Listener::EventType::FriendRequest:
    {
        auto requestEvent = dynamic_cast<ElaphantContact::Listener::RequestEvent*>(&event);
        Log::I(ManagerService::NAME, "Service %s received %s friend request %s\n",
                                     ManagerService::NAME,
                                     event.humanCode.c_str(),
                                     requestEvent->summary.c_str());
        auto service = mService.lock();
        if (service.get() == nullptr) {
            Log::E(ManagerService::NAME, "Service has been deleted.");
            break;
        }
        service->getConnector()->AcceptFriend(event.humanCode);
        break;
    }
    case ElaphantContact::Listener::EventType::HumanInfoChanged:
    {
        auto infoEvent = dynamic_cast<ElaphantContact::Listener::InfoEvent*>(&event);
        Log::I(ManagerService::NAME, "Service %s received %s info changed %s\n",
                                     ManagerService::NAME,
                                     event.humanCode.c_str(),
                                     infoEvent->toString().c_str());
        break;
    }
    default:
        Log::E(ManagerService::NAME, "Unprocessed event: %d", static_cast<int>(event.type));
        break;
    }
}

void ManagerListener::onReceivedMessage(const std::string& humanCode, ElaphantContact::Channel channelType,
                               std::shared_ptr<ElaphantContact::Message> msgInfo)
{
    Log::E(ManagerService::NAME, "Service %s received message %s from %s\n",
                                 ManagerService::NAME,
                                 msgInfo->data->toString().c_str(),
                                 humanCode.c_str());
}

} // MicroService
} // elastos
