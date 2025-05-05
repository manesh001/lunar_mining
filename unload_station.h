#ifndef UNLOAD_STATION_H
#define UNLOAD_STATION_H

#include "service_include.h"

namespace Lunar {
    class UnloadStation
    {
        public:
            UnloadStation (std::string id) :
                mId(id) {}

            virtual ~UnloadStation   ();

            void start();
            void tick ();

            void setId(std::string id);
            std::string id();

            void setState(UnloadStationState stat);
            UnloadStationState state();

            int  unloadingTime      ();
            bool startUnloading     ();
            bool isUnloadingDone    ();
            int  unloadingTimeLeft  ();
            int  numOfTrucksInQueue ();
            long totalWaitTime      ();

            bool        addTruck    (std::string trkId);
            std::string releaseTruck();
            std::string report      ();

        protected:
            void releaseResources();

            friend std::ostream &operator<<(std::ostream &os, Lunar::UnloadStation &stat)
            {
                os << stat.report();
                return os;
            }

        private:
            unsigned long PROCESS_CLOCK {0};
            std::string mId             {};
            UnloadStationState mState   {UnloadStationState::IDEL};
            int  mUnloadingTime         {Lunar::UNLOAD_TIME_MINUTES};
            long mUnloadsCompleted      {0};
            int  mServiceErrors         {0};

            std::list<TruckUnloadingInfo> mTrucksWaiting;

            static bool sortBasedOnArrivalTime(TruckUnloadingInfo &trk1, TruckUnloadingInfo &trk2);
    };
}

#endif // UNLOAD_STATION_H
