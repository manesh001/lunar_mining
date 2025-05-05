# Mining Simulator

## Prerequisites

git
C++ 20 or higher
Linux OS

## Installation
Get the source code
#### git clone https://github.com/manesh001/lunar_mining.git

For installation please follow the following steps
#### 1. cd lunar_mining
#### 2. mkdir build
#### 3. cd build
#### 4. cmake ../
#### 5. make

## Usage

Form 

**build folder** 
run

**./LunarMiningOperation 2>&1 | tee log.csv**

## Configuration

Configuration parameters are located in **mining.cfg** file

#number of trucks

**TRUCKS=11**

#number of unload-stations

**UNLOAD_STATIONS=3**

#process speed up by

**PROCESS_SPEED_UP_BY=70**

#simulation run time in hours

**SIMULATION_TIME_HOURS=72**

Please fill free to adjust the parameters

## Output

Output will be pushed to the standard out
For sample output, please see **log.csv** file

## Design

This application has 4 main components.
1. MiningController
2. Scheduler
3. Trucks
4. UnloadStations

To simulate the mining process, the MiningController initializes the trucks, unload stations and scheduler.
The Scheduler will callback each truck and unload stations for status update.
Each truck is responsible to load and drive the Helium-3 to the unload station.
The Scheduler will assign an unload station with the shortes waittime to the truck.
After the unloading is done unload station will update its status to completion.
The Truck will start the next round of loading.
