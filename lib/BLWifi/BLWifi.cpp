#include <BLWifi.h>

BL::Wifi::Wifi(BL::Logger *logging, BL::Config *conf) : Logable(logging), Configurable(conf)
{
}

BL::ResultCode_t BL::Wifi::begin()
{
    log->trace(F(">>>wifiManSetup" CR));

    wifiManager = new WiFiManager();

    if (wifiManager == NULL)
    {
        return BL::FAILED;
    }

    wifiManager->setDebugOutput(true);
    wifiManager->setConfigPortalTimeout(60);
    
    WiFiManagerParameter **WMParams = (WiFiManagerParameter **)malloc(config->getFieldCount() * sizeof(WiFiManagerParameter *));

    if (WMParams == NULL)
    {
    }

    if (config->getFieldCount() > 0)
    {
        for (unsigned int i = 0; i < config->getEntryCount(); i++)
        {
            ConfigTemplate_t *ct = config->getConfigTemplate(i);

            if (ct != NULL && ct->input_field)
            {
                WMParams[i] = new WiFiManagerParameter(
                    config->getConfigKey(i),
                    ct->label,
                    config->getConfigValue(i),
                    ct->input_len);

                wifiManager->addParameter(WMParams[i]);
            }
        }
    }

    wifiManager->autoConnect();

    if (config->getFieldCount() > 0)
    {
        for (int i = 0; i < config->getFieldCount(); i++)
        {
            if (config->setConfigValue(WMParams[i]->getID(), WMParams[i]->getValue()) >= 0)
            {
                log->trace(F("%s -> %s" CR), WMParams[i]->getID(), WMParams[i]->getValue());
            }
        }
        config->setShouldSave();
    }

    log->trace(F("<<<wifiManSetup" CR));

    return BL::OK;
}

void BL::Wifi::loop()
{
}