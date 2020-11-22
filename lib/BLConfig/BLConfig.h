#ifndef _BLCONFIG_H_
#define _BLCONFIG_H_

// JSON
#define JSMN_STATIC
#include <jsmn.h> //https://github.com/zserge/jsmn

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>
#include <BLConfig.h>

#define CONFIG_JSON_TOKCNT 40

#define CONFIG_KEY_SIZE 40
#define CONFIG_VALUE_SIZE 80
#define CONFIG_LABEL_SIZE 20

#define CONFIG_FILE "config.json"

namespace BL
{
    typedef struct ConfigTemplate
    {
        const char *key;
        const char *defaultvalue;
        const int input_field;
        const char *label;
        const int input_len;
    } ConfigTemplate_t;

    class Config : Logable, Loopable
    {
    private:
        const char *filename = CONFIG_FILE;

        jsmn_parser parser;
        int token_cnt = 0;
        jsmntok_t *tok = NULL;

        ConfigTemplate_t *templates = NULL;

        int entry_cnt = 0;
        size_t config_entry_size = 0;
        char *entries = NULL;

        size_t key_size = CONFIG_KEY_SIZE;
        size_t value_size = CONFIG_VALUE_SIZE;
        int field_cnt = 0;

        boolean should_save = false;

    public:
        Config(BL::Logger *log);
        ~Config();

        const char *getFilename();
        void setFilename(const char *name);

        BL::ResultCode_t begin(const char *filename,
                               ConfigTemplate_t *config_templates, 
                               int count,
                               size_t tokencount = 0,
                               size_t valuesize = CONFIG_VALUE_SIZE);

        void setTemplates(ConfigTemplate_t *templ = NULL);
        ConfigTemplate_t *getTemplates();
        ConfigTemplate_t *getTemplate(int confid);

        void setTokenCount(size_t count);
        size_t getTokenCount();
        size_t estimateTokenCount();

        void setKeySize(size_t keysize);
        size_t getKeySize();

        void setValueSize(size_t valuesize);
        size_t getValueSize();

        void setEntryCount(size_t count);
        size_t getEntryCount();

        void setFieldCount(int count);
        int getFieldCount();

        void setConfigEntrySize(size_t size);
        size_t getConfigEntrySize();

        char *getConfigEntry(int confid);
        ConfigTemplate_t *getConfigTemplate(int confid);

        int matchConfigKey(const char *key);

        const char *getConfigKey(int confid);

        char *getConfigValue(int confid);
        char *getConfigValue(const char *key);

        int setConfigValue(int confid, const char *value);
        int setConfigValue(const char *key, const char *value);

        int parseConfigBuffer(const char *buffer, int size);

        int load();
        int save();

        void setShouldSave();
        void resetShouldSave();
        bool getShouldSave();

        void loop();
    };

    class Configurable 
    {
        public:
            BL::Config *config;

        public:
            Configurable(BL::Config *conf);
    };

}; // namespace BL

#endif