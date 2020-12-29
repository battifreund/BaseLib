#ifndef _BLLOGGER_H_
#define _BLLOGGER_H_

#include <Arduino.h>

// Logging
#include <ArduinoLog.h> //https://github.com/thijse/Arduino-Log

#define SERIAL_BAUDRATE 115200
#define LOGLEVEL LOG_LEVEL_VERBOSE

namespace BL
{
    class Logger
    {
        private:
            int baudrate = SERIAL_BAUDRATE;
            int loglevel = LOGLEVEL;
            Logging *logging = &Log;

        public:
            Logger(int baudrate = SERIAL_BAUDRATE, int loglevel = LOGLEVEL);

            void begin();

            void setLogging(Logging *log = NULL);
            Logging *getLogging();
            void setBaudrate(int baudrate);
            int getBaudrate();
            void setLoglevel(int loglevel);
            int getLoglevel();
    };

    class Logable
    {
        public:
            Logging *log = &Log;
            Logger *logger = NULL;

        public:
            Logable();
            Logable(BL::Logger *log);
            
            void setLogger(BL::Logger *logger_);
    };

}; // namespace BL
#endif