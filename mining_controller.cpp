#include "mining_controller.h"

/**
 * @brief Destroy the Lunar:: Mining Controller:: Mining Controller object
 *
 */
Lunar::MiningController::~MiningController()
{
    stop();
    releaseResources();
}

/**
 * @brief release allocated resources
 *
 */
void
Lunar::MiningController::releaseResources() {
    // stop thread by setting RUN_SERVICE to false
    RUN_SERVICE = false;

    releaseTrucks();
    releaseUnloadStations();

    mTrucks.clear();
    mUnloadStations.clear();

}

/**
 * @brief Initialize resource needed for the mining service
 *          |-Trucks
 *          |-Unloading stations
 *
 * @return Lunar::ServiceStatus
 */
Lunar::ServiceStatus
Lunar::MiningController::init()
{
    initServiceParams();

    int numOfUnloadStations = initUnloadStationService();
    if(numOfUnloadStations < 1) {
        mServiceErrors++;
        return ServiceStatus::ERROR;
    }

    int numOfTrks = initTruckService();
    if(numOfUnloadStations < 1) {
        mServiceErrors++;
        return ServiceStatus::ERROR;
    }

    generateServiceStartUpInfo();

    return ServiceStatus::SUCESS;
}

/**
 * @brief Start Mining service by calling runSimulator
 *
 * @return Lunar::ServiceStatus
 */
Lunar::ServiceStatus
Lunar::MiningController::start()
{
    auto ret {Lunar::ServiceStatus::SUCESS};
    runSimulation();
    return ret;
}

/**
 * @brief Stop thread by setting RUN_SERVICE to false
 *
 */
void
Lunar::MiningController::stop()
{
    RUN_SERVICE = false;
}

/**
 * @brief This method does the followings
 *          |-start trucks
 *          |->start unload-stations
 *          |->start scheduler
 *          |->start mining serivce clock/time by starting new thread
 *
 */
void
Lunar::MiningController::runSimulation()
{
    startTrucks();
    startUnloadStation();
    startUnloadStationScheduler();

    RUN_SERVICE = true;
    std::thread t1(&MiningController::startEventEngine, this);
    t1.join();
}

/**
 * @brief This method act as service clock/timer
 *          which gives trucks, unload-stations and schedule time slice to run
 *          Also, it sends request to modules to generates short report of thier current state
 */
void Lunar::MiningController::startEventEngine()
{
    while(RUN_SERVICE && PROCESS_CLOCK <= Lunar::hourToMinutes(Lunar::SIMULATION_TIME_HOURS)) {
        PROCESS_CLOCK++;
        tick();
        std::this_thread::yield();
        std::this_thread::sleep_for(PROCESSING_TICK);

        // For detail process monitoring
        generateReport();
    }

    generateSummary();

    releaseUnloadStations();
    releaseTrucks();
}

/**
 * @brief This method gives trucks, unload-stations and schedule time slice to run
 *          it iterates through trucks and assign them execution time
 *          it iterates through unload-stations and assign them execution time
 *          and it gives the scheduler time to execution
 */
void
Lunar::MiningController::tick()
{
    //callback trucks
    auto trkTick = [] (std::unique_ptr<Truck> &trk) {
        trk->tick();
    };
    std::ranges::for_each(mTrucks, trkTick);


    //callback unload-stations
    auto unloadStatTick = [] (std::unique_ptr<UnloadStation> &unloadStat) {
        unloadStat->tick();
    };
    std::ranges::for_each(mUnloadStations, unloadStatTick);

    // callback service scheduler
    mUnloadStationScheduler.tick();
}

/**
 * @brief It generates unload stations with unique ids
 *
 * @return int
 */
int
Lunar::MiningController::initUnloadStationService()
{
    // get the number of unload stations from the config file "mining.cfg"
    int numOfUnloadStations = mCfg->numOfUnloadStations();
    if(numOfUnloadStations < 1) {
        mServiceErrors++;
        std::cerr << "[MC-ERROR], Number of Unload-Station:" << numOfUnloadStations << std::endl;
        return ServiceStatus::ERROR;
    }

    // create list of unload stations
    for( int s {0}; s < numOfUnloadStations; s++ ) {
        auto id = "UnloadStation_" + std::to_string(s+1);
        mUnloadStations.emplace_back( make_unique<UnloadStation>(id));
    }

    return mUnloadStations.size();
}

/**
 * @brief It generates trucks with unique ids
 *
 * @return int
 */
int
Lunar::MiningController::initTruckService()
{
    // get the number of trucks from the config file "mining.cfg"
    int numOfTrks = mCfg->numOfTrucks();
    if(numOfTrks < 1) {
        std::cerr << "[MC-ERROR], Number of Trucks:" << numOfTrks << std::endl;
        mServiceErrors++;
        return ServiceStatus::ERROR;
    }

    // create list of trucks
    for( int s {0}; s < numOfTrks; s++ ) {
        auto id = "Truck_" + std::to_string(s+1);
        mTrucks.emplace_back(make_unique<Truck>(id));
    }

    return mTrucks.size();
}

/**
 * @brief It adjust service parameters based on configuration file "mining"
 *  This method checks for
 *      1. if the "SIMULATION_TIME_HOURS" is set, it will adjust the run-time accordingly.
 *      2. if the "PROCESS_SPEED_UP_BY" is set, it will calculation the speed of the process
 */
void
Lunar::MiningController::initServiceParams()
{
    auto simulationDuration = mCfg->simRunTimeInHours();
    if(simulationDuration > 1) {
        Lunar::SIMULATION_TIME_HOURS = simulationDuration;
    }

    auto speedupBy = mCfg->processSpeedUpBy();
    if (speedupBy > 1) {
        speedupBy = 100 % speedupBy;
        PROCESSING_TICK = (speedupBy * PROCESSING_TICK) / 100;
    }
}

/**
 * @brief It calls all trucks to start
 *
 */
void
Lunar::MiningController::startTrucks()
{
    std::ranges::for_each(mTrucks,
                          [] (std::unique_ptr<Truck> &trk) { trk->start();});
}

/**
 * @brief It calls all unload stations to start
 *
 */
void
Lunar::MiningController::startUnloadStation()
{
    std::ranges::for_each(mUnloadStations,
        [] (std::unique_ptr<UnloadStation> &stat) { stat->start();});
}

/**
 * @brief This methode initialize the scheduler with list of trucks and unload-stations
 *
 */
void
Lunar::MiningController::startUnloadStationScheduler()
{
    mUnloadStationScheduler.setUnloadStations(&mUnloadStations);
    mUnloadStationScheduler.setTrucks(&mTrucks);
}

/**
 * @brief Release unload stations
 *
 */
void
Lunar::MiningController::releaseUnloadStations()
{
    for (int i {0}; i < mUnloadStations.size(); i++) {
        mUnloadStations.pop_front();
    }
}

/**
 * @brief Release the trucks
 *
 */
void
Lunar::MiningController::releaseTrucks()
{
    for (int i {0}; i < mTrucks.size(); i++) {
        mTrucks.pop_front();
    }
}

/**
 * @brief set the config data
 *
 * @param cfg
 */
void
Lunar::MiningController::set(Lunar::Config *cfg)
{
    mCfg = cfg;
}

/**
 * @brief Generate reports by requesting reports from the trucks and unload-stations
 *
 */
void
Lunar::MiningController::generateReport()
{
    generateTruckReport();
    generateUnloadStationReport();
}

/**
 * @brief This method iterates through trucks and request a report
 *      The truck "operator<<" is overloaded
 */
void
Lunar::MiningController::generateTruckReport()
{
    // The truck "operator<<" is overloaded
    auto tReportReq = [] (std::unique_ptr<Truck> &trk) {
        std::cout << "[T-REPORT], " << *trk << std::endl;
        };

    std::ranges::for_each(mTrucks, tReportReq);
    std::cout << std::endl;
}

/**
 * @brief This method iterates through stations and request a report
 *      The unload station "operator<<" is overloaded
 */
void
Lunar::MiningController::generateUnloadStationReport()
{
    // The unload station "operator<<" is overloaded
    auto sReportReq = [] (std::unique_ptr<UnloadStation> &stat) {
        std::cout << "[S-REPORT], " << *stat << std::endl;
    };

    std::ranges::for_each(mUnloadStations, sReportReq);
    std::cout << std::endl;
}

/**
 * @brief This method generates the short summary of the simulation run
 *
 */
void
Lunar::MiningController::generateSummary()
{
    std::stringstream ss;
    auto trksTotalDelivery {0};

    //calculate total deliveries by summing up the numbers of deliveries by each truck
    std::ranges::for_each(mTrucks, [&trksTotalDelivery] (std::unique_ptr<Truck> &trk) {
                                    trksTotalDelivery += trk.get()->numOfDeliveries();});

    auto totalRunTime = hourToMinutes(Lunar::SIMULATION_TIME_HOURS);
    ss  << std::setfill('-') << std::setw(40) << "\n" << "[MINING-SUMMARY], \n\t"               << std::left << std::setw(30) << std::setfill(' ')
        << "MiningRunTime:"             << totalRunTime                         << ":min, \n\t" << std::left << std::setw(30)
        << "NumOfUnloadStations:"       << mUnloadStations.size()               << ", \n\t"     << std::left << std::setw(30)
        << "NumOfTrucks:"               << mTrucks.size()                       << ", \n\t"     << std::left << std::setw(30)
        << "NumOfDelivery:"             << trksTotalDelivery                    << ", \n\t"     << std::left << std::setw(30)
        << "AverageTruckDelivery:"      << (trksTotalDelivery / mTrucks.size()) << ", \n\t"     << std::left << std::setw(30)
        << "AverageMiningDeliveryTime:" << (totalRunTime/ trksTotalDelivery)    << ":min\n"     << std::endl;

    std::cerr << ss.rdbuf()->str() << std::endl;

    // iterate through trucks and ask for short summary
    if(mTrucks.empty() == false) {
        ss.clear();
        ss.str("");

        ss << "\n[Truck-SUMMARY], \n\t";
        std::cerr << ss.rdbuf()->str() << std::endl;

        std::ranges::for_each(mTrucks, [] (std::unique_ptr<Truck> &trk) { std::cerr << trk.get()->summary(); });
    }
}

/**
 * @brief It generates information about simulation settings before
 *          starting simulation
 *
 */
void
Lunar::MiningController::generateServiceStartUpInfo()
{
    std::stringstream ss;

    auto totalRunTime = hourToMinutes(Lunar::SIMULATION_TIME_HOURS);

    ss << "[MC-INFO], MiningRunTime:" << totalRunTime                   << "min, "
        << "NumOfUnloadStations:"     << mUnloadStations.size()         << ", "
        << "NumOfTrucks:"             << mTrucks.size()                 << ", "
        << "PROCESSING_TICK:"         << Lunar::PROCESSING_TICK.count() << ", "
        << std::endl;

    std::cerr << ss.rdbuf()->str() << std::endl;
}