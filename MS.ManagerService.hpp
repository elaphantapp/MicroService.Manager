#ifndef __ELASTOS_MS_MANAGER_SERVICE_H__
#define __ELASTOS_MS_MANAGER_SERVICE_H__

#include <Connector.h>

namespace elastos {
namespace MicroService {

class ManagerService : public std::enable_shared_from_this<ManagerService>
{
public:
    static constexpr const char* NAME = "ManagerService";

    explicit ManagerService() = default;
    virtual ~ManagerService() = default;

    void init(const std::string& path);
    std::shared_ptr<Connector> getConnector();

private:
    std::string mPath;
    std::shared_ptr<Connector> mConnector;
};

} // MicroService
} // elastos

extern "C" {
void* CreateService(const char* path);
void DestroyService(void* service);
}

#endif //__ELASTOS_MS_MANAGER_SERVICE_H__
