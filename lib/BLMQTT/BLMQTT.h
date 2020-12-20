#ifndef _BLMQTT_H_
#define _BLMQTT_H_

#include <functional>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// MQTT Client
// MQTT_KEEPALIVE : keepAlive interval in Seconds
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 180
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 180
#endif

#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>
#include <BLConfig.h>

namespace BL
{
    class MQTT : Logable, Configurable, Loopable
    {
        typedef std::function<void(char *, char *, unsigned int)> TopicHandlerFunction_t;

        class Topic : Logable
        {
        private:
            char *id = NULL;
            TopicHandlerFunction_t handler;

        public:
            Topic(BL::Logger *logging);

            void setID(char *identifier);
            void setHandler(TopicHandlerFunction_t handler);

            char *getID();
            TopicHandlerFunction_t getHandler();
        };

    private:
        BL::Config *config = NULL;

        WiFiClient espClient;
        PubSubClient *mqttClient = NULL;

        int topics_max;
        int topics_cnt;
        Topic **topics;

        size_t payloadbuffer_size = 0;
        char *payloadbuffer = NULL;

        char *mqttName = NULL;
        char *mqttPassword = NULL;

        unsigned long lastConnect;
        unsigned long lastTry;
        unsigned long reconInterval = 5000;

#if defined(ESP32)
        //
        // https://github.com/espressif/arduino-esp32/issues/3722
        //
        int _reconnectTries = 0;
#endif

    public:
        MQTT(BL::Logger *logging, BL::Config *config);

        BL::ResultCode_t begin(char *host, uint16_t port, char *mqttName, char *mqttPassword = NULL, int max_topics = 5);
        bool reconnect(unsigned long timeout = 0);
        void setReconInterval(unsigned long interval);
        unsigned long getReconInterval();
        void setLastConnect(unsigned long last);
        unsigned long getLastConnect();
        void setLastTry(unsigned long last);
        unsigned long getLastTry();

        BL::ResultCode_t registerTopic(char *id, TopicHandlerFunction_t handler);
        void resubscribe();
        void unsubscribe();

        void dispatch(char *topic, char *payload, unsigned int payload_size);

        void setMQTTName(char *name);
        char *getMQTTName();

        void loop();
    };
}; // namespace BL

#endif