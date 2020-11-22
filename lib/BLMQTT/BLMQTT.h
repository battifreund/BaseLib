#ifndef _BLMQTT_H_
#define _BLMQTT_H_

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>
#include <BLConfig.h>

namespace BL
{
    class MQTT : Logable, Configurable, Loopable
    {
    private:
        BL::Config *config = NULL;

    public:
        MQTT(BL::Logger *logging, BL::Config *config);
        BL::ResultCode_t begin();

        void loop();
    };
}; // namespace BL

#endif