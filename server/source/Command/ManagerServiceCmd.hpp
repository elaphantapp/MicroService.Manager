#ifndef _LAUNCHER_SERVICE_CMD_HPP_
#define _LAUNCHER_SERVICE_CMD_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "ManagerService.h"
namespace micro_service {
    constexpr int Command_Create = 0;
    constexpr int Command_Restart = 1;
    class ManagerServiceCmd {
    public:
        /*** type define ***/

        /*** static function and variable ***/
        static int Do(void* context,
                      const std::string& cmdLine,
                      std::string& errMsg);

        /*** class function and variable ***/

    protected:
        /*** type define ***/

        /*** static function and variable ***/

        /*** class function and variable ***/

    private:
        /*** type define ***/
        struct CommandInfo {
            std::string mCmd;
            std::string mLongCmd;
            std::function<int(void* context, const std::vector<std::string>&, std::string&)> mFunc;
            std::string mUsage;
        };

        /*** static function and variable ***/
        static int Help(void* context,
                        const std::vector<std::string>& args,
                        std::string& errMsg);
        static int CreateService(void* context,
                               const std::vector<std::string>& args,
                               std::string& errMsg);
        static int StartService(void* context,
                                 const std::vector<std::string>& args,
                                 std::string& errMsg);
        static const std::vector<CommandInfo> gCommandInfoList;

        /*** class function and variable ***/
        explicit ManagerServiceCmd() = delete;
        virtual ~ManagerServiceCmd() = delete;
    };
}
/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

#endif /* _LAUNCHER_SERVICE_CMD_HPP_ */

