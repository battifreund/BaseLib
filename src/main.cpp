#include <Arduino.h>

#include <BLBase.h>
#include <BLLoop.h>
#include <BLConfig.h>
#include <BLLogger.h>
#include <BLwifi.h>
#include <BLOTA.h>

BL::Logger *logger;
BL::Config *config;
BL::Wifi *wifi;
BL::OTA *ota;

BL::ConfigTemplate_t configTemplates[] PROGMEM = {
    {"mqtt_hostname", "mqtt.diefreunds.de", 1, "MQTT Host", 40},
    {"mqtt_port", "1883", 1, "MQTT Port", 6},
#ifdef TESTMODE
    {"channel", "Test", 1, "Channel", 40},
#else
    {"channel", "Heizung", 1, "Channel", 40},
#endif
    {"temp_sampling", "60000", 0, "Temperature Sampling", 6},
    {"gas_sampling", "1000", 0, "Gas Sampling", 6},
    {"status_interval", "30000", 0, "Status Interval", 6},
    {"debug", "0", 0, "Debug", 2}
    };

void setup()
{
  logger = new BL::Logger();
  logger->begin();

  config = new BL::Config(logger);
  
  if (config->begin(CONFIG_FILE, configTemplates, sizeof(configTemplates) / sizeof(BL::ConfigTemplate_t)) == BL::FAILED)
  {
    logger->getLogging()->fatal(F("Can't initialize Config! Halt!" CR));
    while(1==1);
  }

  wifi = new BL::Wifi(logger, config);
  wifi->begin();

  ota = new BL::OTA(logger, config);
  ota->begin();
}

void loop() {
  config->loop();
  wifi->loop();
  ota->loop();
}