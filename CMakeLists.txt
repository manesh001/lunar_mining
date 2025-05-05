cmake_minimum_required(VERSION 3.16)

project(LunarMiningOperation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LunarMiningOperation main.cpp
    config.h                    config.cpp
    unload_station.h            unload_station.cpp
    unload_station_scheduler.h  unload_station_scheduler.cpp
    truck.h                     truck.cpp
    mining_controller.h         mining_controller.cpp
    )

include(GNUInstallDirs)
install(TARGETS LunarMiningOperation
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
