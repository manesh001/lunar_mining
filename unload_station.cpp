#include "unload_station.h"

/**
 * @brief Destroy the Lunar:: Unload Station:: Unload Station object
 *
 */
Lunar::UnloadStation::~UnloadStation()
{
   releaseResources();
}

/**
 * @brief Return stationd id
 *
 * @return std::string
 */
std::string
Lunar::UnloadStation::id()
{
   return mId;
}

/**
 * @brief Returns station state
 *
 * @return Lunar::UnloadStationState
 */
Lunar::UnloadStationState
Lunar::UnloadStation::state()
{
   return mState;
}

/**
 * @brief Set station state
 *
 * @param stat
 */
void
Lunar::UnloadStation::setState(Lunar::UnloadStationState stat)
{
   mState = stat;
}

/**
 * @brief Start station service
 *
 */
void
Lunar::UnloadStation::start()
{
   tick();
}

/**
 * @brief It provides a simple state-machine to determine the next service-stage
 *          Service-Stages:
 *             |-idel
 *             |-unloading
 *             |-unloading-done
 *
 *    It is the execution time slice
 */
void
Lunar::UnloadStation::tick()
{
   PROCESS_CLOCK++;
   switch (mState)
   {
      case UnloadStationState::IDEL:
         if(startUnloading()) {
            mState = UnloadStationState::UNLOADING;
         }
         break;

      case UnloadStationState::UNLOADING:
         if(isUnloadingDone()) {
            mState = Lunar::UnloadStationState::UNLOADING_DONE;
            mUnloadsCompleted++;
         }
         break;

      case UnloadStationState::UNLOADING_DONE:
         break;

      default:
         mServiceErrors++;
         std::cerr << "[S-ERROR], UnloadStattion:" << mId << " is in an unknown state (" << static_cast<int>(mState) << ")\n";
         break;
   }
}

/**
 * @brief Checks if unloading is done
 *          if it is done, update the state of truck in queue
 *
 * @return true
 * @return false
 */
bool
Lunar::UnloadStation::isUnloadingDone()
{
   if(mTrucksWaiting.empty() || mState != UnloadStationState::UNLOADING) {
      return false;
   }

   if(unloadingTimeLeft() <= 0) {
      mTrucksWaiting.front().isDone = true;
   }

   return  mTrucksWaiting.front().isDone;
}


/**
 * @brief Calculate total waiting time
 *
 * @return long
 */
long
Lunar::UnloadStation::totalWaitTime()
{
   if(mTrucksWaiting.empty()) {
      return 0;
   }

   long mTotalWaitTime {0};

   //sum all trucks waiting for unloading
   auto sumTotalWaitTime = [&] (TruckUnloadingInfo &trk) {
         //skip if truck is done
         if(trk.isDone) {
            return;
         }
         else if(trk.startTime > 0) {
            //calculate the remaining time of the active unloading truck
            auto leftTime = (trk.startTime + Lunar::UNLOAD_TIME_MINUTES) - PROCESS_CLOCK;
            if(leftTime > 0) {
               mTotalWaitTime += leftTime;
            }
         }
         else {
            //else add the default unloading-time
            mTotalWaitTime += Lunar::UNLOAD_TIME_MINUTES;
         }
      };

   std::for_each(mTrucksWaiting.begin(), mTrucksWaiting.end(), sumTotalWaitTime);

   return mTotalWaitTime;
}

/**
 * @brief Return num of trucks in the  waiting queue to be processed
 *
 * @return int
 */
int
Lunar::UnloadStation::numOfTrucksInQueue()
{
   return mTrucksWaiting.size();
}

/**
 * @brief Add truck to the waiting queue
 *
 * @param trkId
 * @return true
 * @return false
 */
bool
Lunar::UnloadStation::addTruck(std::string trkId)
{
   // if the waiting queue is empty, just add it
   if(mTrucksWaiting.empty()) {
      mTrucksWaiting.push_back(TruckUnloadingInfo(trkId, PROCESS_CLOCK));
      return true;
   }

   // check if the truck is alreading in the waiting queue
   auto check = [trkId] (TruckUnloadingInfo &trk) -> bool {
      auto found {false};
      (trk.trkId == trkId) ? found = true : found = false;
      return found;
   };
   const auto it = std::ranges::find_if(mTrucksWaiting, check);

   // if the truck is already in the queue, skip the request
   if(it != mTrucksWaiting.end()) {
      mServiceErrors++;
      std::cerr << "[S-ERROR], " << __FUNCTION__ << ", " << it->trkId << " is already in queue" << std::endl;
      return false;       // This truck is already in the queue
   }

   // add the truck to the waiting queue with its arrival time
   mTrucksWaiting.push_back(TruckUnloadingInfo(trkId, PROCESS_CLOCK));

   return true;
}

/**
 * @brief Return unloading time
 *
 * @return int
 */
int
Lunar::UnloadStation::unloadingTime()
{
   return mUnloadingTime;
}

/**
 * @brief Start unloading next truck in the waiting queue
 *
 * @return true
 * @return false
 */
bool
Lunar::UnloadStation::startUnloading()
{
   if(mTrucksWaiting.empty()) {
      return false;
   }

   // sort the waiting queue based on arrival time
   mTrucksWaiting.sort(sortBasedOnArrivalTime);

   // truck with the longest waiting time is in front
   // start the unloading by assiging the start time
   mTrucksWaiting.front().startTime = PROCESS_CLOCK;

   return true;
}

/**
 * @brief Calcuates and return unload time left for the active truck
 *
 * @return int
 */
int
Lunar::UnloadStation::unloadingTimeLeft()
{
   if(mTrucksWaiting.empty() || mState != UnloadStationState::UNLOADING) {
      return 0;
   }

   // calculate the expected runtime left by adding the default unloading time to start-time mins the current process-clock
   auto tm = (mTrucksWaiting.front().startTime + Lunar::UNLOAD_TIME_MINUTES) - PROCESS_CLOCK;
   if(tm < 0) {
      tm = -1;
   }

   return  tm;
}

/**
 * @brief Release truck that is done unloading and return its id
 *
 * @return std::string
 */
std::string
Lunar::UnloadStation::releaseTruck()
{
   std::string trkId {""};

   if(mTrucksWaiting.empty()) {
      mServiceErrors++;
      std::cerr << "[S-ERORR], " << __FUNCTION__ << ":" << __LINE__ << ", " << mId << ", Waiting Queue:EMPTY" << std::endl;
   }
   else if(mTrucksWaiting.front().isDone) {
      // check it the active true is flaged as done,
      // if true,
      //    save the truck id and
      //    remove the truck from waiting queue and
      //    change the state to the idel state for the next run
      trkId = mTrucksWaiting.front().trkId;
      mTrucksWaiting.pop_front();
      mState = UnloadStationState::IDEL;
   }

   return trkId;
}

/**
 * @brief Sorting algorithum for the waiting queue based on the longest arrival time
 *
 * @param stat1
 * @param stat2
 * @return true
 * @return false
 */
bool
Lunar::UnloadStation::sortBasedOnArrivalTime(TruckUnloadingInfo &trk1,
                                             TruckUnloadingInfo &trk2)
{
   return (trk1.arrivalTime > trk2.arrivalTime) ? true : false;
}

/**
 * @brief release waiting queue
 *
 */
void
Lunar::UnloadStation::releaseResources()
{
   mTrucksWaiting.clear();
}

/**
 * @brief Generate simple report based on the current state of the station
 *
 * @return std::string
 */
std::string
Lunar::UnloadStation::report()
{
   std::stringstream ss;
   auto it = UnloadStationStateName.find(mState);
   switch (mState)
   {
   case UnloadStationState::IDEL:
      ss << mId                  << ", ";
      ss << "State:"             << it->second           << ", ";
      ss << "TrucksInQueue:"     << mTrucksWaiting.size()<< ", ";
      ss << "TotalWaitTime:"     << totalWaitTime()      << ":min";
      break;

   case UnloadStationState::UNLOADING:
      ss << mId                  << ", ";
      ss << "State:"             << it->second           << ", ";
      ss << (mTrucksWaiting.size() ? mTrucksWaiting.front().trkId : "[S-ERORR]") << " Unloading, ";
      ss << "UnloadingTimeLeft:" << unloadingTimeLeft()  << ":min, ";
      ss << "TrucksInQueue:"     << mTrucksWaiting.size()<< ", ";
      ss << "TotalWaitTime:"     << totalWaitTime()      << ":min";
      break;

   case UnloadStationState::UNLOADING_DONE:
      ss << mId                  << ", ";
      ss << "State:"             << it->second           << ", ";
      ss << (mTrucksWaiting.size() ? mTrucksWaiting.front().trkId : "[S-ERORR]") << ", ";
      ss << "TrucksInQueue:"     << mTrucksWaiting.size()<< ", ";
      ss << "TotalWaitTime:"     << totalWaitTime()      << ":min";
      break;

   default:
      break;
   }

   return ss.rdbuf()->str();
}


