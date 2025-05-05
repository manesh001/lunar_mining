#include "mining_controller.h"
#include "config.h"
#include "service_include.h"

static Lunar::MiningController mCtrl;

/**
 * @brief It handles the resource releases if it catchs interrupt signal
 *
 * @param signalNumber
 */
void signalHandler(int signalNumber) {
    std::cerr << "[INFO], Interrupt signal (" << signalNumber << ") received." << std::endl;
    mCtrl.stop();
    mCtrl.releaseResources();
    exit(signalNumber);
}


int main(int argc, char *argv[])
{
    std::signal(SIGINT,  signalHandler);

    //Read config file
    Lunar::Config cfg;
    if(cfg.read() < 1) {
        std::cerr << "[ERROR], Config Failed " << Lunar::CONFIG_FILE << std::endl;
        return  EXIT_FAILURE;
    }

    //Initialize the mining controller;
    mCtrl.set(&cfg);
    if(mCtrl.init() < 1) {
        return  EXIT_FAILURE;
    }

    //Start the mining controller
    mCtrl.start();

    return  EXIT_SUCCESS;
}
