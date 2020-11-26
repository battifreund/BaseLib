#include <Arduino.h>

#include <BLMQTT.h>



BL::MQTT::MQTT(BL::Logger *logging, BL::Config *config) : Logable(logging), Configurable(config)
{
}

BL::ResultCode_t BL::MQTT::begin(char *host, uint16_t port, int max_topics)
{
    log->trace(F(">>>setupMQTT(%s, %d)" CR), host, port);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        log->notice(".");
    }

    log->trace(F("IP: %s" CR), WiFi.localIP().toString().c_str());

    mqttClient = new PubSubClient(espClient);

    if(mqttClient == NULL)
    {
        log->fatal(F("Failed to initialize PubSubClient" CR));
        return BL::FAILED;
    }

    mqttClient->setServer(host, port);
    reconnect();

    topics_max = max_topics;
    topics_cnt = 0;

    topics = (BL::MQTT::Topic **)malloc(sizeof(BL::MQTT::Topic *) * topics_max);

    if (topics == NULL)
    {
        log->fatal(F("Out of memory: Failed to malloc topic store" CR));
        return BL::FAILED;
    }

    mqttClient->setCallback([this](char *topic, uint8_t *payload, unsigned int payload_size) {
        this->dispatch(topic, (char *)payload, payload_size);
    });

    return BL::OK;
}

void BL::MQTT::dispatch(char *topic, char *payload, unsigned int payload_size)
{
    if (payloadbuffer == NULL)
    {
        payloadbuffer = (char *)malloc(payload_size + 1);
        payloadbuffer_size = payload_size + 1;

        if (payloadbuffer == NULL)
        {
            return;
        }
    }
    else
    {
        if (payloadbuffer_size < payload_size + 1)
        {
            payloadbuffer = (char *)realloc(payloadbuffer, payload_size + 1);
            payloadbuffer_size = payload_size + 1;

            if (payloadbuffer == NULL)
            {
                return;
            }
        }
    }

    memcpy(payloadbuffer, payload, payload_size);
    payloadbuffer[payload_size] = 0;

    log->trace("Command received: %s -> %s" CR, topic, payloadbuffer);

    for (int i = 0; i < topics_cnt; i++)
    {
        if(strcmp(topic, topics[i]->getID()) == 0) {
            topics[i]->getHandler()(topic, payloadbuffer, payload_size);
        }
    }
}

BL::ResultCode_t
BL::MQTT::registerTopic(char *id, TopicHandlerFunction_t handler)
{
    if(topics == NULL) {
        log->fatal(F("Uninitilized topic store" CR));
        return BL::FAILED;
    }

    if(topics_cnt + 1 > topics_max)
    {
        topics_max += 5;
        topics = (BL::MQTT::Topic **) realloc(topics, sizeof(BL::MQTT::Topic *) * topics_max);
        if(topics == NULL)
        {
            log->fatal(F("Out of memory: Failed to extend topic store" CR));
            return BL::FAILED;
        }
    }

    topics[topics_cnt] = new BL::MQTT::Topic(logger);
    topics[topics_cnt]->setID(id);
    topics[topics_cnt]->setHandler(handler);

    topics_cnt++;

    return BL::OK;
}

void BL::MQTT::resubscribe()
{
    for (int i = 0; i < topics_cnt; i++) {
        log->trace(F("Subscribe(%s)" CR), topics[i]->getID());
        mqttClient->subscribe(topics[i]->getID());
    }
}

void BL::MQTT::unsubscribe()
{
    for (int i = 0; i < topics_cnt; i++)
    {
        mqttClient->unsubscribe(topics[i]->getID());
    }
}

void BL::MQTT::reconnect()
{
    log->trace(F(">>> reconnect()" CR));

    if (mqttClient == NULL)
    {
        log->fatal(F("reconnect failed: mqttClient NULL" CR));
    }
    // Loop until we're reconnected
    while (!mqttClient->connected())
    {
        log->notice(F("Attempting MQTT connection..." CR));
        // Attempt to connect
        if (mqttClient->connect(WiFi.hostname().c_str()))
        {
            log->notice(F("connected" CR));

            // ... and resubscribe
        }
        else
        {
            log->error(F("Connection failed (rc=%d)" CR), mqttClient->state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void BL::MQTT::loop()
{
    if (mqttClient->connected())
        mqttClient->loop();
}

BL::MQTT::Topic::Topic(BL::Logger *logging) : Logable(logging)
{

}

void BL::MQTT::Topic::setID(char *identifier)
{
    id = identifier;
}

void BL::MQTT::Topic::setHandler(TopicHandlerFunction_t _handler)
{
    handler = _handler;
}

char *BL::MQTT::Topic::getID()
{
    return id;
}

BL::MQTT::TopicHandlerFunction_t BL::MQTT::Topic::getHandler()
{
    return handler;
}