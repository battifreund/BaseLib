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
            BL::Config::Entry  *ent = config->getEntry(i);

            if (ent != NULL && ent->isInputField())
            {
                WMParams[i] = new WiFiManagerParameter(
                    ent->getKey(),
                    ent->getInputLabel(),
                    ent->getDefaultValue(),
                    ent->getInputLen());

                wifiManager->addParameter(WMParams[i]);
            }
        }
    }

    wifiManager->autoConnect();

    if (config->getFieldCount() > 0)
    {
        for (int i = 0; i < config->getFieldCount(); i++)
        {
            if (config->setValue(WMParams[i]->getID(), WMParams[i]->getValue()) >= 0)
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