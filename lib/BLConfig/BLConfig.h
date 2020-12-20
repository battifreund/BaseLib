#ifndef _BLCONFIG_H_
#define _BLCONFIG_H_

// JSON
#define JSMN_STATIC
#include <jsmn.h> //https://github.com/zserge/jsmn

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>

#define CONFIG_VALUE_SIZE 80

#define CONFIG_FILE "/config.json"

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
        public:
        class Entry : Logable 
        {
            private:
                ConfigTemplate_t *templ;
                char value[CONFIG_VALUE_SIZE + 1];

            public:
                Entry();
                Entry(BL::Logger *logging);

                void setTemplate(ConfigTemplate_t *templ);
                ConfigTemplate_t *getTemplate();

                const char *getKey();

                void setValue(const char *value);
                char *getValue();

                const char *getDefaultValue();
                boolean isInputField();
                const char *getInputLabel();
                const int getInputLen();
        };

    private:
        const char *filename = CONFIG_FILE;

        jsmn_parser parser;
        size_t token_cnt = 0;
        size_t token_max = 0;
        jsmntok_t *tok = NULL;

        size_t entry_cnt = 0;
        size_t entry_max = 15;
        Entry *entries;
        size_t field_cnt = 0;

        size_t keysize_max = 0;

        boolean should_save = false;

    public:
        Config(BL::Logger *log);
        ~Config();

        BL::ResultCode_t begin(const char *filename,
                               ConfigTemplate_t *config_templates,
                               size_t count);

        const char *getFilename();
        void setFilename(const char *name);

        void setTokenCount(size_t count);
        size_t getTokenCount();

        size_t estimateMaxTokenCount();
        void setMaxTokenCount(size_t count);
        size_t getMaxTokenCount();

        void setEntryCount(size_t count);
        size_t getEntryCount();

        void setFieldCount(size_t count);
        size_t getFieldCount();

        void setKeySize(size_t size);
        size_t getKeySize();

        Entry *getEntry(const char *key);
        Entry *getEntry(size_t confid);

        int setValue(const char *key, const char *value);
        char *getValue(const char *key);

        int parseConfigBuffer(const char *buffer, size_t size);

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