#include <BLWifi.h>

BL::Wifi::Wifi(BL::Logger *logging, BL::Config *conf) : Logable(logging), Configurable(conf)
{
}

void BL::Wifi::transferParams()
{
    if (config->getFieldCount() > 0)
    {
        for (size_t i = 0; i < config->getFieldCount(); i++)
        {
            if (config->setValue(WMParams[i]->getID(), WMParams[i]->getValue()) >= 0)
            {
                log->trace(F("%s -> %s" CR), WMParams[i]->getID(), WMParams[i]->getValue());
            }
        }
        config->setShouldSave();
    }
}

BL::ResultCode_t BL::Wifi::begin(unsigned long interval)
{
    log->trace(F(">>>wifiManSetup" CR));

    setReconInterval(interval);

    wifiManager = new WiFiManager();

    if (wifiManager == NULL)
    {
        return BL::FAILED;
    }

    wifiManager->setDebugOutput(true);
    wifiManager->setConfigPortalTimeout(600);

    WMParams = (WiFiManagerParameter **)malloc(config->getFieldCount() * sizeof(WiFiManagerParameter *));

    if (WMParams == NULL)
    {
        // TODO
    }

    if (config->getFieldCount() > 0)
    {
        for (size_t i = 0; i < config->getEntryCount(); i++)
        {
            BL::Config::Entry *ent = config->getEntry(i);

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
    setLastConnect(millis());

    transferParams();

    log->trace(F("<<<wifiManSetup" CR));

    return BL::OK;
}

void BL::Wifi::setReconInterval(unsigned long interval)
{
    reconInterval = interval;
}

unsigned long BL::Wifi::getReconInterval()
{
    return reconInterval;
}

void BL::Wifi::setLastConnect(unsigned long last)
{
    lastConnect = last;
}

unsigned long BL::Wifi::getLastConnect()
{
    return lastConnect;
}

void BL::Wifi::setLastTry(unsigned long last)
{
    lastTry = last;
}

unsigned long BL::Wifi::getLastTry()
{
    return lastTry;
}

const __FlashStringHelper *BL::Wifi::mapWifiStatus(wl_status_t status)
{
    const __FlashStringHelper *statstr = F("Unknown status");

    switch (status)
    {
    case WL_IDLE_STATUS:
        statstr = F("Idle status");
    case WL_NO_SSID_AVAIL:
        statstr = F("No SSID available");
    case WL_SCAN_COMPLETED:
        statstr = F("Scan complete");
    case WL_CONNECTED:
        statstr = F("Connected");
    case WL_CONNECT_FAILED:
        statstr = F("Connection failed");
    case WL_CONNECTION_LOST:
        statstr = F("Connection lost");
    case WL_DISCONNECTED:
        statstr = F("Disconnected");
    };

    return statstr;
}

void BL::Wifi::loop()
{
    if (!WiFi.isConnected())
    {
        if (millis() > getLastTry() + getReconInterval())
        {
            log->notice(F("Wifi Connecting ..." CR));
            
            WiFi.mode(WIFI_STA);

#if defined(ESP8266)
            //trying to fix connection in progress hanging
            ETS_UART_INTR_DISABLE();
            wifi_station_disconnect();
            ETS_UART_INTR_ENABLE();
#else
            esp_wifi_disconnect();
#endif

            if (WiFi.begin() == WL_CONNECTED)
            {
                log->notice(F("Wifi reconnect successful!" CR));
                setLastConnect(millis());
            }
            else
            {
                unsigned long timeout = millis() + 150000L;
                unsigned long lastlog = millis();

                while (!WiFi.isConnected() && millis() < timeout)
                {
                    yield();
                    if (millis() > lastlog + 3000)
                    {
                        lastlog = millis();
                        log->notice(F("Wifi status : %s" CR), mapWifiStatus(WiFi.status()));
                    }
                }

                if (WiFi.status() != WL_CONNECTED)
                {
                    log->error(F("Wifi reconnect failed: %d %s" CR), WiFi.status(), mapWifiStatus(WiFi.status()));
                }
                else
                {
                    log->notice(F("Wifi reconnect successful!" CR));
                    setLastConnect(millis());
                }
            }
            setLastTry(millis());
        }
    }
}