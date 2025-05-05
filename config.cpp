#include "config.h"

/**
 * @brief Construct a new Lunar:: Config:: Config object
 *
 * @param path
 */
Lunar::Config::Config(std::string path) {
    mPath = path;
}

/**
 * @brief Destroy the Lunar:: Config:: Config object
 *
 */
Lunar::Config::~Config()
{
    mLst.clear();
}

/**
 * @brief read the config file "mining.cfg"
 *
 * @param path
 * @return int
 */
int
Lunar::Config::read(std::string path)
{
    std::string line;

    mPath = path;

    std::ifstream inputFile(mPath);
    if (inputFile.is_open() == false ) {
        std::cerr << "[ERROR], Unable to open file" << path << std::endl;
        return ServiceStatus::ERROR;
    }

    while (std::getline(inputFile, line)) {
        if(line.find(CONFIG_COMMENT_TAGE) != std::string::npos) {
            continue;
        }
        if(addToConfLst(line) == false) {
            std::cerr << "[ERROR], Failed to find config-param in " << line << std::endl;
            return ServiceStatus::ERROR;
        }
    }

    inputFile.close();
    return mLst.size();
}

/**
 * @brief It creates config list from  params
 *
 * @param line
 * @return true
 * @return false
 */
bool
Lunar::Config::addToConfLst(const std::string &line)
{
    bool ret {false};
    int num {0};

    std::string token;
    std::vector<std::string> tokens;
    std::stringstream ss(line);

    while (std::getline(ss, token, Lunar::CONFIG_DELIMITER)) {
        tokens.push_back(token);
    }

    if(tokens.size() != 2 || std::isdigit(tokens[1].at(0)) == false) {
        return ret;
    }

    num = getIntParam(tokens[1]);

    auto it = ConfigParam.find(tokens.at(0));
    if(it != ConfigParam.end()) {
        mLst[it->second] = num;
        ret = true;
    }

    return ret;
}

/**
 * @brief It parses the key=value
 *
 * @param param
 * @return int
 */
int
Lunar::Config::getIntParam(const std::string &param)
{
    int num {0};
    try {
        num = std::stoi(param);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "[ERROR], Invalid argument: " << e.what() << std::endl;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "[ERROR], Out of range: " << e.what() << std::endl;
    }

    return num;
}

/**
 * @brief It returns the value for param TRUCKS
 *
 * @return int
 */
int
Lunar::Config::numOfTrucks()
{
    auto it = mLst.find(ServiceParams::TRUCK);
    if(it == mLst.end()) {
        return Lunar::ERROR;
    }

    return it->second;
}

/**
 * @brief It returns the value for param UNLOAD_STATIONS
 *
 * @return int
 */
int
Lunar::Config::numOfUnloadStations()
{
    auto it = mLst.find(ServiceParams::UNLOAD_STATION);
    if(it == mLst.end()) {
        return Lunar::ERROR;
    }

    return it->second;
}

/**
 * @brief It returns the process speed up factor
 *
 * @return int
 */
int
Lunar::Config::processSpeedUpBy()
{
    auto it = mLst.find(ServiceParams::PROCESS_SPEED_UP_BY);
    if(it == mLst.end()) {
        return Lunar::ERROR;
    }

    return it->second;
}

/**
 * @brief It returns the run time for simulation
 *
 * @return int
 */
int
Lunar::Config::simRunTimeInHours()
{
    auto it = mLst.find(ServiceParams::SIMULATION_TIME_HOURS);
    if(it == mLst.end()) {
        return Lunar::ERROR;
    }

    return it->second;
}