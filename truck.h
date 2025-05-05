#ifndef TRUCK_H
#define TRUCK_H

#include "service_include.h"

namespace Lunar {
    class Truck
    {
        public:
            Truck() {}

            Truck(std::string id) : mId(id) {}
            Truck(const Truck &trk);
            Truck &operator=(const Truck &trk);

            virtual ~Truck();

            void start();
            void tick ();
            void reset();
            void setId (std::string id);
            std::string id();

            void setState   (TruckState stat);
            TruckState state();

            void setLoadingTime(int t);
            int  loadingTime();
            void startLoading();
            long loadingStartTime();
            int  loadingTimeLeft();

            bool isWaitingForUnloadStation();
            long timeWaitingForUnLoadStation();
            void assignUnloadStation(std::string sId);
            bool hasUnloadingStation();
            std::string unloadStationID();
            long unloadingTimeLeft();
            void unloadingDone();

            int  numOfDeliveries();

            std::string report();
            std::string summary();

        protected:
            friend std::ostream &operator<<(std::ostream &os, Lunar::Truck &trk)
            {
                os << trk.report();
                return os;
            }

        private:
            unsigned long PROCESS_CLOCK{0};

            std::string mId    {};
            TruckState  mState {TruckState::IDEL};
            std::string mUnloadStationId {};
            int  mLoadingTime       {0};
            long mLoadingStartTime  {0};
            long mDrivingStartTime  {0};
            long mUnLoadStationArrivalTime{0};
            long mUnloadingStartTime{0};
            int  mDeliveryCompleted {0};
            int  mServiceErrors     {0};

            std::list<std::pair<std::string, int>> mWaitTimeLst {};

            bool isLoadingDone      ();
            void startDriving       ();
            long drivingTimeLeft    ();
            bool isDrivingDone      ();
            bool isUnloadingDone    ();
            void finalizeDelivery   ();
        };
}

#endif // TRUCK_H
