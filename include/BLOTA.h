#ifndef _BLOTA_H_
#define _BLOTA_H_

#include <Arduino.h>

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>
#include <BLConfig.h>

#define NO_GLOBAL_ARDUINOOTA
#include <ArduinoOTA.h>

namespace BL
{
    class OTA : Logable, Configurable, Loopable
    {
        private:
            ArduinoOTAClass *ota;

        public:
            OTA(BL::Logger *logging, BL::Config *config);

            BL::ResultCode_t begin();

            void loop();
    };
};

#endif