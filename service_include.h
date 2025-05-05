#ifndef SERVICE_INCLUDE_H
#define SERVICE_INCLUDE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <format>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ranges>
#include <list>
#include <vector>
#include <map>
#include <thread>
#include <random>
#include <ctime>
#include <csignal>
#include <cctype>


namespace Lunar {

    // Default Parameters

    const int           LOADING_TIME_MIN_HOURS{1};                // hour
    const int           LOADING_TIME_MAX_HOURS{5};                // hour
    const int           DRIVE_TIME_MINUTES    {30};               // 30 min
    const int           UNLOAD_TIME_MINUTES   {5};                // 5 min
    const std::string   CONFIG_FILE           {"../mining.cfg"};  // default config-file
    const char          CONFIG_COMMENT_TAGE   {'#'};              // default comment tage for config-file
    const char          CONFIG_DELIMITER      {'='};              // default delimiter for config-file

    static int          SIMULATION_TIME_HOURS {72};               // to speed up the simulation "decrease" SIMULATION_TIME_HOURS
                                                                  // or update the param SIMULATION_TIME_HOURS in mining.cfg

    static std::chrono::milliseconds PROCESSING_TICK(100);        // to speed up the simulation "decrease" PROCESSING_TICK
                                                                  // or update the param PROCESS_SPEED_UP_BY in mining.cfg

    struct TruckUnloadingInfo {
        std::string   trkId      {};
        unsigned long arrivalTime{0};
        unsigned int  startTime  {0};
        bool          isDone     {false};
    };

    enum class TruckState {
        IDEL   = 0,
        LOADING,
        DRIVING,
        WAITING_FOR_UNLOAD_STATION,
        UNLOADING,
        UNLOADING_DONE,
        COUNT
    };

    enum class UnloadStationState {
        IDEL   = 0,
        UNLOADING,
        UNLOADING_DONE,
        COUNT
    };

    enum ServiceStatus {
        ERROR   = -1,
        UNKNOWN = 0,
        SUCESS  = 1,
    };

    struct TruckLog {
        std::string mId         {};
        int         mLoadingTime{0};
        int         mWaitTime   {0};
    };


    enum class ServiceParams {
        TRUCK = 0,
        UNLOAD_STATION,
        PROCESS_SPEED_UP_BY,
        SIMULATION_TIME_HOURS,
        COUNT };

    const static std::map<std::string, ServiceParams> ConfigParam {
        {"TRUCKS",              ServiceParams::TRUCK},
        {"UNLOAD_STATIONS",     ServiceParams::UNLOAD_STATION},
        {"PROCESS_SPEED_UP_BY", ServiceParams::PROCESS_SPEED_UP_BY},
        {"SIMULATION_TIME_HOURS",ServiceParams::SIMULATION_TIME_HOURS}
    };

    const static std::map<TruckState, std::string> TruckStateName {
        {TruckState::IDEL,                          "IDEL"},
        {TruckState::LOADING,                       "LOADING"},
        {TruckState::DRIVING,                       "DRIVING"},
        {TruckState::WAITING_FOR_UNLOAD_STATION,    "WAITING_FOR_UNLOAD_STATION"},
        {TruckState::UNLOADING,                     "UNLOADING"},
        {TruckState::UNLOADING_DONE,                "UNLOADING_DONE"},
    };

    const static std::map<UnloadStationState, std::string> UnloadStationStateName {
        {UnloadStationState::IDEL,          "IDEL"},
        {UnloadStationState::UNLOADING,     "UNLOADING"},
        {UnloadStationState::UNLOADING_DONE,"UNLOADING_DONE"}
    };


    static long hourToMinutes(int val) { return (val * 60); }
    static long mintueToSeconds(int val) { return (val * 60); }
    static int secondToMinutes(long val) { return (val / 60); }
    static int speedUpSimByMagnitude(int val, int mag) { return  (val / mag); }

    static int generateLoadingTime() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(LOADING_TIME_MIN_HOURS, LOADING_TIME_MAX_HOURS);
        int randomNum = distrib(gen);
        return hourToMinutes(randomNum); // to minutes
    }
}

#endif // SERVICE_INCLUDE_H
