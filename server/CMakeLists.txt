cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES
        source/Tools/Log.hpp
        source/Tools/Log.cpp
        source/Common/FileUtils.cpp
        source/Common/FileUtils.hpp
        source/DataBase/DatabaseProxy.h
        source/DataBase/DatabaseProxy.cpp
        source/DataBase/ServiceInfo.h
        source/DataBase/ServiceInfo.cpp
        source/Command/ManagerServiceCmd.cpp
        source/Command/ManagerServiceCmd.hpp
        source/ThirdParty/CompatibleFileSystem.hpp
        source/ThirdParty/ghc-filesystem.hpp
        source/ManagerService.h
        source/ManagerService.cpp
        source/ErrCode.cpp
        source/ErrCode.h)
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
