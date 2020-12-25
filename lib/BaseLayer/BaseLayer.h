#ifndef _BASELAYER_H_
#define _BASELAYER_H_

#include <BLBase.h>
#include <BLLoop.h>
#include <BLConfig.h>
#include <BLLogger.h>
#include <BLwifi.h>
#include <BLOTA.h>
#include <BLMQTT.h>

namespace BL
{
    class BaseLayer : Loopable
    {
        void setup();
        void loop();
    };
} // namespace BL

#endif