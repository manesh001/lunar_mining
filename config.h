#ifndef CONFIG_H
#define CONFIG_H

#include "service_include.h"

namespace Lunar
{

   class Config
   {
   public:
      Config(std::string path = Lunar::CONFIG_FILE);
      virtual ~Config();

      int read                (std::string path = Lunar::CONFIG_FILE);
      int numOfTrucks         ();
      int numOfUnloadStations ();
      int processSpeedUpBy    ();
      int simRunTimeInHours   ();

   protected:
      std::string mPath {Lunar::CONFIG_FILE};
      std::map<ServiceParams, int>mLst;

      bool addToConfLst (const std::string &param);
      int  getIntParam  (const std::string &param);

   };
}
#endif // CONFIG_H
