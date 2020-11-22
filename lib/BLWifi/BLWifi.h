#ifndef _BLWIFI_H_
#define _BLWIFI_H_

// Logging
#include <ArduinoLog.h> //https://github.com/thijse/Arduino-Log

// WiFiManager
#include <DNSServer.h>   //Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>
#include <BLConfig.h>

namespace BL
{
    class Wifi : Logable, Configurable, Loopable
    {
    private:
        WiFiManager *wifiManager = NULL;

    public:
        Wifi(BL::Logger *logging, BL::Config *config);
        BL::ResultCode_t begin();

        void loop();
    };
}; // namespace BL

#endif