#include <BLConfig.h>

BL::Config::Config(BL::Logger *logging) : Logable(logging)
{
}

BL::Config::~Config()
{
}

BL::ResultCode_t BL::Config::begin(const char *filename,
                                   ConfigTemplate_t *config_templates,
                                   size_t count)
{
    log->trace(F(">>> begin(count : %d)" CR), count);

    setFilename(filename);

    entries = (Entry *)malloc(count * sizeof(Entry));
    if (entries == NULL)
    {
        log->fatal(F("Malloc (entries) failed!" CR));
        return BL::FAILED;
    }
    memset(entries, 0, count * sizeof(Entry));
    setEntryCount(count);

    if (token_max == 0)
    {
        token_max = estimateMaxTokenCount();
    }

    tok = (jsmntok_t *)malloc(token_max * sizeof(jsmntok_t));
    if (tok == NULL)
    {
        log->fatal(F("Malloc (tok) failed!" CR));
        return BL::FAILED;
    }
    setMaxTokenCount(token_max);

    createEntries(config_templates, count);

    /* Prepare parser */
    jsmn_init(&parser);

    load();

    log->trace(F("Config ready!" CR));

    return BL::OK;
}

void BL::Config::createEntries(ConfigTemplate_t *config_templates, size_t count)
{
    size_t max_keysize = 0;
    size_t fc = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (max_keysize < strlen(config_templates[i].key))
        {
            max_keysize = strlen(config_templates[i].key);
        }

        log->trace("i %d : max_keysize %d" CR, i, max_keysize);

        entries[i].setTemplate(&config_templates[i]);
        entries[i].setValue(config_templates[i].defaultvalue);

        if (config_templates[i].input_field == 1)
        {
            fc++;
        }

        log->trace("fc %d" CR, fc);
    }
    setFieldCount(fc);
    setKeySize(max_keysize);
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

size_t BL::Config::estimateMaxTokenCount()
{
    return entry_cnt * 5;
}

void BL::Config::setMaxTokenCount(size_t count)
{
    log->trace(F("setMaxTokenCount(%d)" CR), count);
    token_max = count;
}

size_t BL::Config::getMaxTokenCount()
{
    return token_max;
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

void BL::Config::setFieldCount(size_t count)
{
    log->trace(F("setFieldCount(%d)" CR), count);
    field_cnt = count;
}

size_t BL::Config::getFieldCount()
{
    return field_cnt;
}

void BL::Config::setKeySize(size_t size)
{
    log->trace(F("setKeySize(%d)" CR), size);
    keysize_max = size;
}

size_t BL::Config::getKeySize()
{
    return keysize_max;
}

BL::Config::Entry *BL::Config::getEntry(size_t confid)
{
    if (confid >= getEntryCount())
    {
        log->fatal(F("getEntry : Illegal Id (%d)" CR), confid);
        return NULL;
    }

    return &entries[confid];
}

BL::Config::Entry *BL::Config::getEntry(const char *key)
{
    for (size_t i = 0; i < getEntryCount(); i++)
    {
        if (strcmp(getEntry(i)->getKey(), key) == 0)
        {
            return getEntry(i);
        }
    }

    log->fatal(F("getEntry : Illegal Key (%s)" CR), key);
    return NULL;
}

char *BL::Config::getValue(const char *key)
{
    Entry *ent = getEntry(key);
    if (ent != NULL)
    {

        return ent->getValue();
    }
    else
    {
        return NULL;
    }
}

int BL::Config::setValue(const char *key, const char *value)
{
    Entry *ce = getEntry(key);

    if (ce == NULL)
    {
        log->fatal(F("setValue(%s, %s) : Failed" CR), key, value);
        return -1;
    }

    log->trace(F("setConfigValue(%s, %s)" CR), key, value);

    ce->setValue(value);

    setShouldSave();

    return 0;
}

int BL::Config::parseConfigBuffer(const char *buffer, size_t size)
{
    log->trace(F(">>> parseConfigBuffer(%d, %s)" CR), size, buffer);

    jsmn_init(&parser);

    int r = jsmn_parse(&parser, buffer, size, tok, getMaxTokenCount());

    log->trace(F("r = %d" CR), r);

    char *token;
    Entry *ent = NULL;

    int assertions = -1;

    if (r >= 0)
    {
        assertions = 0;

        token = (char *)malloc(size + 1);

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
                        if (ent != NULL)
                        {
                            ent->setValue(token);
                            assertions++;
                            ent = NULL;
                        }
                        else
                        {
                            log->warning(F("No key found!" CR));
                        }
                        break;
                    case 1:
                        ent = getEntry(token);

                        if (ent == NULL)
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

#if defined(ESP32)
    if (FILESYSTEM.begin(true))
#else
    if (FILESYSTEM.begin())
#endif
    {
        if (FILESYSTEM.exists(CONFIG_FILE))
        {
            // Open file for reading
            File file = FILESYSTEM.open(CONFIG_FILE, "r");

            size_t size = file.size();

            if (size > 0)
            {
                config_buffer = (char *)malloc(size + 1);

                if (config_buffer != NULL)
                {
                    if (file.readBytes(config_buffer, size) > 0)
                    {
                        config_buffer[size] = '\0';
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

        FILESYSTEM.end();
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

#if defined(ESP32)
    if (FILESYSTEM.begin(true))
#else
    if (FILESYSTEM.begin())
#endif
    {

        // Delete existing file, otherwise the configuration is appended to the file
        FILESYSTEM.remove(getFilename());

        // Open file for writing
        File file = FILESYSTEM.open(getFilename(), "w");
        if (!file)
        {
            log->error(F("Failed to create file" CR));
            return -1;
        }
        file.print("{");
        for (size_t i = 0; i < getEntryCount(); i++)
        {
            if (i > 0)
            {
                file.print(",");
            }
            log->trace(F("\"%s\":\"%s\"" CR), getEntry(i)->getKey(), getEntry(i)->getValue());
            file.printf("\"%s\":\"%s\"", getEntry(i)->getKey(), getEntry(i)->getValue());
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

BL::Config::Entry::Entry()
{
    templ = NULL;
    value[0] = '\0';
}

BL::Config::Entry::Entry(BL::Logger *logging) : Logable(logging)
{
    templ = NULL;
    value[0] = '\0';
}

void BL::Config::Entry::setTemplate(ConfigTemplate_t *templ_)
{
    templ = templ_;
}

BL::ConfigTemplate_t *BL::Config::Entry::getTemplate()
{
    return templ;
}

const char *BL::Config::Entry::getKey()
{
    if (templ == NULL)
    {
        log->fatal(F("getKey : No template!" CR));
        return NULL;
    }
    return templ->key;
}

const char *BL::Config::Entry::getDefaultValue()
{
    if (templ == NULL)
    {
        log->fatal(F("getDefaultValue : No template!" CR));
        return NULL;
    }
    return templ->defaultvalue;
}

boolean BL::Config::Entry::isInputField()
{
    if (templ == NULL)
    {
        log->fatal(F("isInputField : No template!" CR));
        return NULL;
    }
    return templ->input_field == 1;
}

const char *BL::Config::Entry::getInputLabel()
{
    if (templ == NULL)
    {
        log->fatal(F("getInputLabel : No template!" CR));
        return NULL;
    }
    return templ->label;
}

const int BL::Config::Entry::getInputLen()
{
    if (templ == NULL)
    {
        log->fatal(F("getInputLen : No template!" CR));
        return -1;
    }
    return templ->input_len;
}

char *BL::Config::Entry::getValue()
{
    return value;
}

void BL::Config::Entry::setValue(const char *value_)
{
    strncpy(value, value_, CONFIG_VALUE_SIZE);
}