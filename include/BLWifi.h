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
        WiFiManagerParameter **WMParams = NULL;

        unsigned long lastConnect;
        unsigned long lastTry;
        unsigned long reconInterval = 5000;

    public:
        Wifi(BL::Logger *logging, BL::Config *config);
        BL::ResultCode_t begin(unsigned long reconInterval = 5000);

        void setReconInterval(unsigned long interval);
        unsigned long getReconInterval();
        void setLastConnect(unsigned long last);
        unsigned long getLastConnect();
        void setLastTry(unsigned long last);
        unsigned long getLastTry();

        void transferParams();
        const __FlashStringHelper* mapWifiStatus(wl_status_t status);

        void loop();
    };
}; // namespace BL

#endif