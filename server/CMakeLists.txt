cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES
        Tools/Log.hpp
        Tools/Log.cpp
        Common/FileUtils.cpp
        Common/FileUtils.hpp
        DataBase/DatabaseProxy.h
        DataBase/DatabaseProxy.cpp
        DataBase/ServiceInfo.h
        DataBase/ServiceInfo.cpp
        Command/ManagerServiceCmd.cpp
        Command/ManagerServiceCmd.hpp
        ThirdParty/CompatibleFileSystem.hpp
        ThirdParty/ghc-filesystem.hpp
        ManagerService.h
        ManagerService.cpp
        ErrCode.cpp
        ErrCode.h)
INCLUDE_DIRECTORIES(source/)
LINK_DIRECTORIES("source/")
add_library(MicroServiceManager  SHARED )
target_sources(MicroServiceManager PRIVATE
        ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(MicroServiceManager pthread sqlite3)

pkg_search_module(pkg-Elastos.SDK.Contact REQUIRED Elastos.SDK.Contact)
message(STATUS "Elastos.SDK.Contact include path: ${pkg-Elastos.SDK.Contact_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(MicroServiceManager PRIVATE "${pkg-Elastos.SDK.Contact_INCLUDE_DIRS}" )
TARGET_LINK_LIBRARIES(MicroServiceManager PeerNode)

install(TARGETS MicroServiceManager
        LIBRARY DESTINATION lib/PeerNodePlugins
        ARCHIVE DESTINATION lib/PeerNodePlugins)
