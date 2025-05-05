#include "unload_station_scheduler.h"

/**
 * @brief set/obtain unload-station queue
 *
 * @param unloadStations
 */
void
Lunar::UnloadStationScheduler::setUnloadStations(std::list<std::unique_ptr<UnloadStation>> *unloadStations)
{
    mUnloadStations = unloadStations;
}

/**
 * @brief set/obtain trucks queue
 *
 * @param trks
 */
void
Lunar::UnloadStationScheduler::setTrucks(std::list<std::unique_ptr<Truck>> *trks)
{
    mTrucks = trks;
}

/**
 * @brief It checks on
 *              |-unloading station state
 *              |-trucks state
 *    It is the execution time slice
 */
void
Lunar::UnloadStationScheduler::tick()
{
    checkForUnloadingDone();
    checkForUnloadingRequest();
}

/**
 * @brief This method iterate through unload-stations
 *          and creates a list of trucks that are done with unloading
 *          then iterates through list and updating truck's state accordingly
 */
void
Lunar::UnloadStationScheduler::checkForUnloadingDone()
{
    if(mUnloadStations == nullptr || mTrucks == nullptr ||
        mUnloadStations->empty()   || mTrucks->empty()) {
         return;
    }

    std::vector<std::string> trkDone;

    // iterate through unload-stations
    // and add the trucks that are done unloading into the list
    for (auto statItr = mUnloadStations->begin(); statItr != mUnloadStations->end(); statItr++) {

        std::string tmp = Lunar::UnloadStationStateName.find(statItr->get()->state())->second;
        if(statItr->get()->state() == UnloadStationState::UNLOADING_DONE) {
            auto tId = statItr->get()->releaseTruck();
            trkDone.push_back(tId);
        }
    }

    if(trkDone.empty()) {
        return;
    }

    auto count {0};
    //iterate through the list and updating truck's state accordingly
    for( auto &t : trkDone) {
        auto itr = std::find_if(mTrucks->begin(), mTrucks->end(), [&t] (std::unique_ptr<Truck> &trk) {
                                bool found {false};
                                (trk.get()->id() == t) ? found = true : found = false;
                                return found;});

            if(itr != mTrucks->end()) {
                if(itr->get()->hasUnloadingStation()) {
                    itr->get()->unloadingDone();
                }
            }
    }
}

/**
 * @brief This method iterates through trucks to see if any of them are waiting for unloading.
 *          If yes, it assigns unload station with least waiting time to the truck
 *
 */
void
Lunar::UnloadStationScheduler::checkForUnloadingRequest()
{
    if(mUnloadStations == nullptr || mTrucks == nullptr ||
        mUnloadStations->empty()   || mTrucks->empty()) {
         return;
    }

    // sort the unload stations based on least waiting time
    mUnloadStations->sort(decrementSortingForWaitTime);
    mTrucks->sort(incrementSortingForWaitTime);

    auto statItr = mUnloadStations->begin();
    auto trkItr  = mTrucks->begin();

    // iterate through the trucks list
    // find truck in the waiting state
    // assign unload-station with least waiting-time to the truck in the waiting state
    for (auto trkItr = mTrucks->begin(); trkItr != mTrucks->end(); trkItr++) {

        // if we reached the end of the station list
        // sort the unload stations again
        if(statItr == mUnloadStations->end()) {
            mUnloadStations->sort(decrementSortingForWaitTime);
            statItr = mUnloadStations->begin();
        }

        if (trkItr->get()->state()               == TruckState::WAITING_FOR_UNLOAD_STATION &&
            trkItr->get()->hasUnloadingStation() == false) {

                trkItr->get()->assignUnloadStation(statItr->get()->id());
                statItr->get()->addTruck(trkItr->get()->id());
                statItr++;
        }
    }
}


/**
 * @brief It is sort algorithm to sort unload stations based on least wait time
 *
 * @param stat1
 * @param stat2
 * @return true
 * @return false
 */
bool
Lunar::UnloadStationScheduler::decrementSortingForWaitTime(std::unique_ptr<UnloadStation> &stat1,
                                                           std::unique_ptr<UnloadStation> &stat2)
{
    return (stat1->totalWaitTime() < stat2->totalWaitTime()) ? true : false;
}

/**
 * @brief It is sort algorithm to sort trucks based on thier oldest arrival time
 *
 * @param trk1
 * @param trk2
 * @return true
 * @return false
 */
bool
Lunar::UnloadStationScheduler::incrementSortingForWaitTime(std::unique_ptr<Truck> &trk1,
                                                          std::unique_ptr<Truck> &trk2)
{
    return (trk1->timeWaitingForUnLoadStation() > trk2->timeWaitingForUnLoadStation()) ? true : false;
}

/**
 * @brief It request report from each truck and unload station
 *
 */
void
Lunar::UnloadStationScheduler::report()
{
    if(mUnloadStations != nullptr) {
        auto sReportReq = [] (std::unique_ptr<UnloadStation> &stat) {
            std::cout << "[SCHD-REPORT], " << *stat << std::endl;
        };

        std::ranges::for_each(*mUnloadStations, sReportReq);
        std::cout << std::endl;
    }

    if(mTrucks != nullptr) {
        auto sReportReq = [] (std::unique_ptr<Truck> &trk) {
            std::cout << "[SCHD-REPORT], " << *trk << std::endl;
        };

        std::ranges::for_each(*mTrucks, sReportReq);
        std::cout << std::endl;
    }
}
