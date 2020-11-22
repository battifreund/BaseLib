#include <Arduino.h>
#include <LittleFS.h>

#include <BLConfig.h>

BL::Config::Config(BL::Logger *logging) : Logable(logging)
{
  
}

BL::Config::~Config()
{
}

BL::ResultCode_t BL::Config::begin(const char *filename,
                                   ConfigTemplate_t *config_templates,
                                   int count,
                                   size_t tokencount,
                                   size_t valuesize)
{
    log->trace(F(">>> begin(count : %d, tokencount : %d, valuesize : %d)" CR), count, tokencount, valuesize);

    setFilename(filename);
    setValueSize(valuesize);
    setConfigEntrySize(valuesize + 1);
    setTemplates(config_templates);

    entries = (char *)malloc(getConfigEntrySize() * count);
    if (entries == NULL)
    {
        log->fatal(F("Malloc (entries) failed!" CR));
        return BL::FAILED;
    }
    setEntryCount(count);

    if (tokencount != 0)
    {
        setTokenCount(tokencount);
    }
    else
    {
        setTokenCount(estimateTokenCount());
    }

    tok = (jsmntok_t *)malloc(getTokenCount() * sizeof(jsmntok_t));
    if (tok == NULL)
    {
        log->fatal(F("Malloc (tokens) failed!" CR));
        return BL::FAILED;
    }

    size_t max_keysize = 0;
    int fc = 0;
    /*
    for (int i = 0; i < count; i++)
    {
        log->trace("--------------------------" CR);
        log->trace("key : %s" CR, config_templates[i].key);
        log->trace("defaultvalue : %s" CR, config_templates[i].defaultvalue);
        log->trace("input_field : %d" CR, config_templates[i].input_field);
        log->trace("label : %s" CR, config_templates[i].label);
        log->trace("input_len : %d" CR, config_templates[i].input_len);
    }
*/
    for (int i = 0; i < count; i++)
    {
        if (max_keysize < strlen(config_templates[i].key))
        {
            max_keysize = strlen(config_templates[i].key);
        }

        log->trace("i %d : max_keysize %d" CR, i, max_keysize);

        setConfigValue(i, config_templates[i].defaultvalue);

        log->trace("SET %s" CR, getConfigValue(i));

        if (config_templates[i].input_field == 1)
        {
            fc++;
        }

        log->trace("fc %d" CR, fc);
    }

    setKeySize(max_keysize);
    setFieldCount(fc);

    /* Prepare parser */
    jsmn_init(&parser);

    load();

    log->trace(F("Config ready!" CR));

    return BL::OK;
}

void BL::Config::setTemplates(ConfigTemplate_t *templ)
{
    templates = templ;
}

BL::ConfigTemplate_t *BL::Config::getTemplates()
{
    return templates;
}

BL::ConfigTemplate_t *BL::Config::getTemplate(int confid)
{
    return &getTemplates()[confid];
}

const char *BL::Config::getFilename()
{
    return filename;
}

void BL::Config::setFilename(const char *name)
{
    filename = name;
}

void BL::Config::setTokenCount(size_t count)
{
    log->trace(F("setTokenCount(%d)" CR), count);
    token_cnt = count;
}

size_t BL::Config::getTokenCount()
{
    return token_cnt;
}

size_t BL::Config::estimateTokenCount()
{
    return entry_cnt * 5;
}

void BL::Config::setKeySize(size_t keysize)
{
    log->trace(F("setKeySize(%d)" CR), keysize);
    key_size = keysize;
}

size_t BL::Config::getKeySize()
{
    return key_size;
}

void BL::Config::setValueSize(size_t valuesize)
{
    log->trace(F("setValueSize(%d)" CR), valuesize);
    value_size = valuesize;
}

size_t BL::Config::getValueSize()
{
    return value_size;
}

void BL::Config::setConfigEntrySize(size_t size)
{
    log->trace(F("setConfigEntrySize(%d)" CR), size);
    config_entry_size = size;
}

size_t BL::Config::getConfigEntrySize()
{
    return config_entry_size;
}

void BL::Config::setEntryCount(size_t count)
{
    log->trace(F("setEntryCount(%d)" CR), count);
    entry_cnt = count;
}

size_t BL::Config::getEntryCount()
{
    return entry_cnt;
}

void BL::Config::setFieldCount(int count)
{
    log->trace(F("setFieldCount(%d)" CR), count);
    field_cnt = count;
}

int BL::Config::getFieldCount()
{
    return field_cnt;
}

char *BL::Config::getConfigEntry(int confid)
{
    return (char *)(entries + confid * getConfigEntrySize());
}

BL::ConfigTemplate_t *BL::Config::getConfigTemplate(int confid)
{
    return &templates[confid];
}

int BL::Config::matchConfigKey(const char *key)
{
    log->trace(F(">>> matchConfigKey(%s)" CR), key);

    int id = -1;
    for (int i = 0; i < getEntryCount(); i++)
    {
        id = strncmp(getConfigTemplate(i)->key, key, getKeySize());
        if (id == 0)
        {
            log->trace(F("<<< matchConfigKey = %d" CR), i);
            return i;
        }
    }

    log->trace(F("<<< matchConfigKey = -1" CR));
    return -1;
}

const char *BL::Config::getConfigKey(int confid)
{
    if (confid < 0 || confid > getEntryCount())
    {
        log->fatal(F("getConfigKey : Illegal Id (%d)" CR), confid);
        return NULL;
    }
    return getConfigTemplate(confid)->key;
}

char *BL::Config::getConfigValue(int confid)
{
    if (confid < 0 || confid > getEntryCount())
    {
        log->fatal(F("getConfigValue : Illegal Id (%d)" CR), confid);
        return NULL;
    }
    return getConfigEntry(confid);
}

char *BL::Config::getConfigValue(const char *key)
{
    return getConfigValue(matchConfigKey(key));
}

int BL::Config::setConfigValue(int confid, const char *value)
{
    char *ce = getConfigEntry(confid);

    if (ce == NULL)
    {
        log->fatal(F("setConfigValue(%d, %s) : Failed" CR), confid, value);
        return -1;
    }

    log->trace(F("setConfigValue(%d, %s)" CR), confid, value);

    strncpy(ce, value, getValueSize());

    log->trace(F("ce-> %s" CR), ce);

    return 0;
}

int BL::Config::setConfigValue(const char *key, const char *value)
{
    return setConfigValue(matchConfigKey(key), value);
}

int BL::Config::parseConfigBuffer(const char *buffer, int size)
{
    log->trace(F(">>> parseConfigBuffer(%d, %s)" CR), size, buffer);

    jsmn_init(&parser);

    int r = jsmn_parse(&parser, buffer, size, tok, getTokenCount());

    log->trace(F("r = %d" CR), r);

    char *token;
    int key = -1;
    int assertions = -1;

    if (r >= 0)
    {
        assertions = 0;

        token = (char *)malloc(size);

        if (token != NULL)
        {
            for (int i = 0; i < r; i += 1)
            {
                strncpy(token,
                        buffer + tok[i].start, tok[i].end - tok[i].start);
                token[tok[i].end - tok[i].start] = '\0';

                log->trace(F("id %d : start %d : end %d : size %d  : type %d : %s" CR), i, tok[i].start, tok[i].end, tok[i].size, tok[i].type, token);

                if (tok[i].type == JSMN_STRING)
                {
                    switch (tok[i].size)
                    {
                    case 0:
                        if (key >= 0)
                        {
                            // setConfigValue(key, token);
                            assertions++;
                        }
                        else
                        {
                            log->warning(F("No key found!" CR));
                        }
                        break;
                    case 1:
                        key = matchConfigKey(token);

                        if (key >= 0)
                        {
                            log->trace(F("key %d : %s" CR), key, getConfigKey(key));
                        }
                        else
                        {
                            log->warning(F("Unknown config key '%s'" CR), token);
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            if (token != NULL)
                free(token);
        }
        else
        {
            log->fatal(F("malloc(%d) failed: Out of memory!" CR), size);
        }
    }
    else
    {
        log->fatal(F("Parsing ConfigBuffer failed!" CR));
    }

    log->trace(F("<<< parseConfigBuffer" CR));

    return assertions;
}

int BL::Config::load()
{
    size_t size = -1;
    char *config_buffer = NULL;

    log->trace(F(">>> load(%s)" CR), getFilename());

    if (LittleFS.begin())
    {
        if (LittleFS.exists(CONFIG_FILE))
        {
            // Open file for reading
            File file = LittleFS.open(CONFIG_FILE, "r");

            size_t size = file.size();

            if (size > 0)
            {
                config_buffer = (char *)malloc(size);

                if (config_buffer != NULL)
                {
                    if (file.readBytes(config_buffer, size) > 0)
                    {
                        log->trace(F("File read! size : %d" CR), size);
                        if (parseConfigBuffer(config_buffer, size) < 0)
                        {
                            log->fatal(F("Parsing failed" CR));
                            return -1;
                        }
                    }
                    else
                    {
                        log->fatal(F("Read failed" CR));
                        size = -1;
                    }
                }
                else
                {
                    log->fatal(F("Out of memory!" CR));
                    size = -1;
                }
            }
            else
            {
                log->fatal(F("File size not expected: %d" CR), size);
                log->trace(F("<<< readConfig()" CR));
                return -1;
            }

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
        else
        {
            log->fatal(F("No config file!"));
        }

        LittleFS.end();
    }
    else
    {
        log->fatal(F("failed to mount FS" CR));
        size = -1;
    }

    if (config_buffer != NULL)
    {
        log->trace(F("Free config_buffer" CR));
        free(config_buffer);
        log->trace(F("Free config_buffer done" CR));
    }

    //end read
    log->trace(F("<<< readConfig()" CR));
    return size;
}

int BL::Config::save()
{
    log->trace(F(">>> save(%s)" CR), getFilename());

    if (LittleFS.begin())
    {

        // Delete existing file, otherwise the configuration is appended to the file
        LittleFS.remove(getFilename());

        // Open file for writing
        File file = LittleFS.open(getFilename(), "w");
        if (!file)
        {
            log->error(F("Failed to create file" CR));
            return -1;
        }
        file.print("{");
        for (int i = 0; i < getEntryCount(); i++)
        {
            if (i > 0)
            {
                file.print(",");
            }
            log->trace(F("\"%s\":\"%s\"" CR), getConfigKey(i), getConfigValue(i));
            file.printf("\"%s\":\"%s\"", getConfigKey(i), getConfigValue(i));
        }
        file.print("}");
        // Close the file

        resetShouldSave();
    }
    else
    {
        log->fatal(F("failed to mount FS" CR));
        log->trace(F("<<< writeConfig()" CR));
        return -1;
    }

    log->trace(F("<<< writeConfig()" CR));
    return 0;
}

void BL::Config::setShouldSave()
{
    should_save = true;
}

void BL::Config::resetShouldSave()
{
    should_save = false;
}

bool BL::Config::getShouldSave()
{
    return should_save;
}

void BL::Config::loop()
{
    if (should_save)
    {
        save();
    }
}

BL::Configurable::Configurable(BL::Config *conf)
{
    config = conf;
}