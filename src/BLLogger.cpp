#include <Arduino.h>

#include <BLLogger.h>

BL::Logger::Logger(int baudrate, int loglevel)
{
    setBaudrate(baudrate);
    setLoglevel(loglevel);
}

void printTimestamp(Print *_logOutput)
{
    char c[16];
    sprintf(c, "%10lu  -  ", millis());
    _logOutput->print(c);
}

void BL::Logger::begin()
{
    // Init serial port and clean garbage
    Serial.begin(baudrate);
    while (!Serial && !Serial.available())
    {
    }
    Serial.println(".\n");
    Serial.println(".\n");
    Serial.println(".\n");

    // Pass log level, whether to show log level, and print interface.
    // Available levels are:
    // LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE
    // Note: if you want to fully remove all logging code, uncomment #define DISABLE_LOGGING in Logging.h
    //       this will significantly reduce your project size

    logging->begin(loglevel, &Serial, false);
    //Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
    //Log.setSuffix(printNewline); // Uncomment to get newline as suffix

    logging->setPrefix(&printTimestamp);

    logging->notice(F("Logging started" CR));
}

void BL::Logger::setLogging(Logging *log)
{
    logging = log;
}

Logging *BL::Logger::getLogging()
{
    return logging;
}

void BL::Logger::setBaudrate(int baudrate)
{
    this->baudrate = baudrate;
}

int BL::Logger::getBaudrate()
{
    return baudrate;
}

void BL::Logger::setLoglevel(int loglevel)
{
    this->loglevel = loglevel;
}

int BL::Logger::getLoglevel()
{
    return loglevel;
}

BL::Logable::Logable()
{
    log = &Log;
}

BL::Logable::Logable(BL::Logger *logging) 
{
    if (logging == NULL)
    {
        log = &Log;
    }
    else
    {
        log = logging->getLogging();
    }

    logger = logging;
}