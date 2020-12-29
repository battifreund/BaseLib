#include <BLOTA.h>

BL::OTA::OTA(BL::Logger *logging, BL::Config *config) : Logable(logging), Configurable(config)
{

}

BL::ResultCode_t BL::OTA::begin()
{
    // -----------------------------------------------------------------------------
    // OTA
    // -----------------------------------------------------------------------------

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword((const char *)"123");

    ota = new ArduinoOTAClass();

    ota->onStart([this]() {
        log->trace(F("Start" CR));
    });

    ota->onEnd([this]() {
        log->trace(F("End" CR));
    });

    ota->onProgress([this](unsigned int progress, unsigned int total) {
        log->notice(F("Progress: %u%%\r"), (progress / (total / 100)));
    });

    ota->onError([this](ota_error_t error) {
        log->error(F("Error[%u]: " CR), error);
        if (error == OTA_AUTH_ERROR)
            log->error(F("Auth Failed" CR));
        else if (error == OTA_BEGIN_ERROR)
            log->error(F("Begin Failed" CR));
        else if (error == OTA_CONNECT_ERROR)
            log->error(F("Connect Failed" CR));
        else if (error == OTA_RECEIVE_ERROR)
            log->error(F("Receive Failed" CR));
        else if (error == OTA_END_ERROR)
            log->error(F("End Failed" CR));
    });

    ota->begin();

    return BL::OK;
}

void BL::OTA::loop()
{
    ota->handle();
}