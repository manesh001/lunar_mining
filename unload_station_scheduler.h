#ifndef UNLOAD_STATION_SCHEDULER_H
#define UNLOAD_STATION_SCHEDULER_H

#include "service_include.h"
#include "unload_station.h"
#include "truck.h"

namespace Lunar {
    class UnloadStationScheduler
    {
        public:
            UnloadStationScheduler() {}

            virtual ~UnloadStationScheduler() {}

            void setUnloadStations(std::list<std::unique_ptr<UnloadStation>> *unloadStations);
            void setTrucks(std::list<std::unique_ptr<Truck>> *trks);

            void tick   ();
            void report ();

        protected:
            std::list<std::unique_ptr<UnloadStation>> *mUnloadStations{nullptr};
            std::list<std::unique_ptr<Truck>> *mTrucks{nullptr};

            static bool decrementSortingForWaitTime(std::unique_ptr<UnloadStation> &stat1,
                                                     std::unique_ptr<UnloadStation> &stat2);

            static bool incrementSortingForWaitTime(std::unique_ptr<Truck> &stat1,
                                                    std::unique_ptr<Truck> &stat2);

        private:
            int  mServiceErrors{0};

            void checkForUnloadingDone();
            void checkForUnloadingRequest();
    };
};
#endif // UNLOAD_STATION_SCHEDULER_H
