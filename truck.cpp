#include "truck.h"


/**
 * @brief Destroy the Lunar:: Truck:: Truck object
 *
 */
Lunar::Truck::~Truck()
{
}

/**
 * @brief  Starts truck service
 *
 */
void
Lunar::Truck::start()
{
   tick();
}

/**
 * @brief It provides a simple state-machine to determine the next service-stage
 *          Service-Stages:
 *             |-idel
 *             |-loading
 *             |-driving
 *             |-wait-for-unload-station
 *             |-unloading
 *             |-unloading-done
 *
 *    It is the execution time slice
 */
void
Lunar::Truck::tick()
{
   PROCESS_CLOCK++;
   switch (mState)
   {
      case TruckState::IDEL:
         PROCESS_CLOCK = 0;
         startLoading();
      break;

      case TruckState::LOADING:
         if(isLoadingDone()) {
            startDriving();
         }
      break;

      case TruckState::DRIVING:
         if(isDrivingDone()) {
            mUnLoadStationArrivalTime = PROCESS_CLOCK;
            mState = TruckState::WAITING_FOR_UNLOAD_STATION;
         }
      break;

      case TruckState::WAITING_FOR_UNLOAD_STATION:
         // waiting for scheduler to assign an unload-station
      break;

      case TruckState::UNLOADING:
         if(isUnloadingDone()) {
            // waiting for scheduler callback
         }
      break;

      case TruckState::UNLOADING_DONE:
         finalizeDelivery();
      break;

      default:
         mServiceErrors++;
         std::cerr << "[T-ERROR], " << mId << " is in an unknown state (" << static_cast<int>(mState) << ")\n";
         break;
   }
}

/**
 * @brief Initialize parameter for loading task
 *
 */
void
Lunar::Truck::startLoading()
{
   mLoadingStartTime = PROCESS_CLOCK;
   mState            = TruckState::LOADING;
   mLoadingTime      = generateLoadingTime();
}

/**
 * @brief Checks state of loading
 *
 * @return true
 * @return false
 */
bool
Lunar::Truck::isLoadingDone()
{
   if(mState != TruckState::LOADING) {
      return true;
   }

   if(PROCESS_CLOCK >= (mLoadingStartTime + mLoadingTime)) {
      return true;
   }
   return false;
}

/**
 * @brief Set loading time
 *
 * @param t
 */
void
Lunar::Truck::setLoadingTime(int t)
{
   mLoadingTime = t;
}

/**
 * @brief return loading time
 *
 * @return long
 */
long
Lunar::Truck::loadingStartTime()
{
   return mLoadingStartTime;
}

/**
 * @brief Calculates how much loading time left
 *
 * @return int
 */
int
Lunar::Truck::loadingTimeLeft()
{
   if(mState != TruckState::LOADING) {
      return 0;
   }

   auto endTime = mLoadingStartTime + mLoadingTime;
   return (endTime - PROCESS_CLOCK);
}

/**
 * @brief initialize parameters for driving tasks
 *
 */
void
Lunar::Truck::startDriving()
{
   mDrivingStartTime = PROCESS_CLOCK;
   mState = TruckState::DRIVING;
}

/**
 * @brief Calculate how much time left to reach unload-station
 *
 * @return long
 */
long
Lunar::Truck::drivingTimeLeft()
{
   if(mState != TruckState::DRIVING) {
      return 0;
   }

   return (mDrivingStartTime + Lunar::DRIVE_TIME_MINUTES) - PROCESS_CLOCK;
}

/**
 * @brief Calculate if truck has reached the unload-station
 *
 * @return true
 * @return false
 */
bool
Lunar::Truck::isDrivingDone()
{
   if(mState == TruckState::DRIVING &&
      (PROCESS_CLOCK >= (mDrivingStartTime + Lunar::DRIVE_TIME_MINUTES))) {
      return true;
   }

   return false;
}

/**
 * @brief Check if truck is in the waiting-stage
 *
 * @return true
 * @return false
 */
bool
Lunar::Truck::isWaitingForUnloadStation()
{
   return (mState == TruckState::WAITING_FOR_UNLOAD_STATION) ? true : false;
}

/**
 * @brief Calculate how much time truck is waiting for unload-station to be available
 *
 * @return long
 */
long
Lunar::Truck::timeWaitingForUnLoadStation()
{
   if(mState == TruckState::WAITING_FOR_UNLOAD_STATION) {
      return PROCESS_CLOCK - mUnLoadStationArrivalTime;
   }
   return 0;
}

/**
 * @brief Callback-method for scheduler to assign unload-station
 *          and initialize parameters for the unloading task
 * @param sId
 */
void
Lunar::Truck::assignUnloadStation(std::string sId)
{
   mUnloadStationId    = sId;
   mState              = TruckState::UNLOADING;
   mUnloadingStartTime = PROCESS_CLOCK;
}

/**
 * @brief Check if scheduler has assigned any unload-station
 *
 * @return true
 * @return false
 */
bool
Lunar::Truck::hasUnloadingStation()
{
   return (mUnloadStationId.size() > 2) ? true : false;
}

/**
 * @brief Return the truck's unload-station
 *
 * @return std::string
 */
std::string
Lunar::Truck::unloadStationID()
{
   return mUnloadStationId;
}

/**
 * @brief Check if truck is done with unloading
 *
 * @return true
 * @return false
 */
bool
Lunar::Truck::isUnloadingDone()
{
   if(mState == TruckState::UNLOADING_DONE) {
      return true;
   }

   return false;
}

/**
 * @brief Callback method for scheduler to update the status of unloading
 *
 */
void
Lunar::Truck::unloadingDone()
{
   if(mState != TruckState::UNLOADING) {
      mServiceErrors++;
      std::cerr << "[T-ERROR], " << __FUNCTION__ << ", id:" << mId << std::endl;
      return;
   }

   mState = TruckState::UNLOADING_DONE;
}

/**
 * @brief Calculates how much of the unloading time left
 *
 * @return long
 */
long
Lunar::Truck::unloadingTimeLeft()
{
   if(mState == TruckState::UNLOADING) {
      return (mUnloadingStartTime + Lunar::UNLOAD_TIME_MINUTES) - PROCESS_CLOCK;
   }

   return 0;
}

/**
 * @brief Returns current loading time
 *
 * @return int
 */
int
Lunar::Truck::loadingTime()
{
   return mLoadingTime;
}

/**
 * @brief Returns the number of end-to-end delivery
 *
 * @return int
 */
int
Lunar::Truck::numOfDeliveries()
{
   return mDeliveryCompleted;
}

/**
 * @brief Construct a new Lunar:: Truck:: Truck object
 *
 * @param trk
 */
Lunar::Truck::Truck(const Truck &trk)
{
   mId               = trk.mId;
   mState            = trk.mState;
   mLoadingStartTime = trk.mLoadingStartTime;
   mLoadingTime      = trk.mLoadingTime;
   mDrivingStartTime = trk.mDrivingStartTime;
   mUnLoadStationArrivalTime = trk.mUnLoadStationArrivalTime;
   mUnloadingStartTime = trk.mUnloadingStartTime;
   mUnloadStationId    = trk.mUnloadingStartTime;
   mDeliveryCompleted  = trk.mDeliveryCompleted;
   PROCESS_CLOCK       = trk.PROCESS_CLOCK;
   mServiceErrors      = trk.mServiceErrors;
}


/**
 * @brief  Assignment operator
 *
 * @param trk
 * @return Lunar::Truck&
 */
Lunar::Truck &
Lunar::Truck::operator=(const Truck &trk)
{
   if(this == &trk) {
      return *this;
   }

   mId               = trk.mId;
   mState            = trk.mState;
   mLoadingStartTime = trk.mLoadingStartTime;
   mLoadingTime      = trk.mLoadingTime;
   mDrivingStartTime = trk.mDrivingStartTime;
   mUnLoadStationArrivalTime = trk.mUnLoadStationArrivalTime;
   mUnloadingStartTime = trk.mUnloadingStartTime;
   mUnloadStationId    = trk.mUnloadingStartTime;
   mDeliveryCompleted  = trk.mDeliveryCompleted;
   PROCESS_CLOCK       = trk.PROCESS_CLOCK;
   mServiceErrors      = trk.mServiceErrors;

   return *this;
}

/**
 * @brief Save some parameters of delivery process for bookkeeping purposes
 *        reset task parameters for next task
 */
void
Lunar::Truck::finalizeDelivery()
{
   mDeliveryCompleted++;

   auto waitTime = PROCESS_CLOCK - mUnLoadStationArrivalTime;
   mWaitTimeLst.push_back(std::pair<std::string, int>(mUnloadStationId, waitTime));
   reset();
}

/**
 * @brief Reset parameters for next task
 *
 */
void
Lunar::Truck::reset()
{
   mLoadingTime      = 0;
   mLoadingStartTime = 0;
   mDrivingStartTime = 0;
   mUnLoadStationArrivalTime = 0;
   mUnloadingStartTime = 0;
   mUnloadStationId    = "";
   mState              = TruckState::IDEL;
}

/**
 * @brief Return truck-id
 *
 * @return std::string
 */
std::string
Lunar::Truck::id()
{
   return mId;
}

/**
 * @brief Set truck-id
 *
 * @param name
 */
void
Lunar::Truck::setId(std::string name)
{
   mId = name;
}

/**
 * @brief Return truck current state
 *
 * @return Lunar::TruckState
 */
Lunar::TruckState
Lunar::Truck::state()
{
   return mState;
}

/**
 * @brief Set truck state
 *
 * @param stat
 */
void
Lunar::Truck::setState(Lunar::TruckState stat)
{
   mState= stat;
}

/**
 * @brief Generated a simple report Generate simple report based on the current state of the truck
 *
 * @return std::string
 */
std::string
Lunar::Truck::report()
{
   std::stringstream ss;
   auto it = TruckStateName.find(mState);
   switch (mState)
   {
      case TruckState::IDEL:
         ss << mId                  << ", "
            << "DeliveryCompleted:" << mDeliveryCompleted  << ", "
            << "State:"             << it->second;
      break;

      case TruckState::LOADING:
         ss << mId                  << ", "
            << "State:"             << it->second        << ", "
            << "LoadingTime:"       << mLoadingTime      << ":min, "
            << "LoadingTimeLeft:"   << loadingTimeLeft() << ":min";
      break;

      case TruckState::DRIVING:
         ss << mId                  << ", "
            << "State:"             << it->second        << ", "
            << "DrivingTimeLeft:"   << drivingTimeLeft() << ":min";
      break;

      case TruckState::WAITING_FOR_UNLOAD_STATION:
         ss << mId                  << ", "
            << "State:"             << it->second                    << ", "
            << "UnloadWaitTime:"    << timeWaitingForUnLoadStation() << ":min";
      break;

      case TruckState::UNLOADING:
         ss << mId                  << ", "
            << "State:"             << it->second          << ", "
            << "At:"                << mUnloadStationId    << ", "
            << "UnloadingTimeLeft:" << unloadingTimeLeft() << ":min";
      break;

      default:
         break;
   }

   return ss.rdbuf()->str();
}

/**
 * @brief Generate simple summary of all deliveries
 *
 * @return std::string
 */
std::string
Lunar::Truck::summary()
{
   std::stringstream ss;

   auto runTime      {Lunar::hourToMinutes(Lunar::SIMULATION_TIME_HOURS)};
   int avgDelivery   = (mDeliveryCompleted > 0) ? (runTime/ mDeliveryCompleted) : 0;
   int totalWaitTime {0};

   if(mWaitTimeLst.size() > 0) {
      std::ranges::for_each(mWaitTimeLst, [&totalWaitTime] (std::pair<std::string, int> &t)
                           { totalWaitTime += t.second; });
   }

   ss << "[T-SUMMARY], " << mId << ", TotalRunTime:"<< runTime << ":min, "
      << "NumOfDelivery:"        << mDeliveryCompleted         << ", "
      << "AverageDeliveryTime:"  << avgDelivery                << ":min, "
      << "TotalWaitTime:"        << totalWaitTime              << ":min"
      << std::endl;

   return ss.rdbuf()->str();
}
