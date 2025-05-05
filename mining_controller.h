#ifndef MINING_CONTROLLER_H
#define MINING_CONTROLLER_H

#include "mining_controller.h"
#include "config.h"
#include "unload_station.h"
#include "truck.h"
#include "unload_station_scheduler.h"

namespace Lunar {

    class MiningController
    {
        public:
            MiningController() {}

            MiningController(Config *cfg) :
                mCfg(cfg) {}

                virtual ~MiningController();
                void     releaseResources();

                ServiceStatus init ();
                ServiceStatus start();
                void          stop ();

                void set(Lunar::Config *cfg);

        protected:
            Config *mCfg;
            std::list<std::unique_ptr<Truck>> mTrucks;
            std::list<std::unique_ptr<UnloadStation>> mUnloadStations;
            UnloadStationScheduler mUnloadStationScheduler;

            int  initUnloadStationService();
            int  initTruckService  ();
            void initServiceParams ();
            void startTrucks       ();
            void startUnloadStation();
            void startUnloadStationScheduler();
            void runSimulation     ();
            void startEventEngine  ();

            void generateReport     ();
            void generateTruckReport();
            void generateUnloadStationReport();
            void generateSummary    ();
            void generateServiceStartUpInfo();

            void releaseUnloadStations();
            void releaseTrucks();

        private:
            bool RUN_SERVICE {false};
            unsigned long PROCESS_CLOCK {0};
            int mServiceErrors {0};

            void tick();
    };
};

#endif // MINING_CONTROLLER_H
