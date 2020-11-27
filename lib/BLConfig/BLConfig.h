#ifndef _BLCONFIG_H_
#define _BLCONFIG_H_

// JSON
#define JSMN_STATIC
#include <jsmn.h> //https://github.com/zserge/jsmn

#include <BLBase.h>
#include <BLLoop.h>
#include <BLLogger.h>

#define CONFIG_VALUE_SIZE 80

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
        int token_cnt = 0;
        int token_max = 0;
        jsmntok_t *tok = NULL;

        int entry_cnt = 0;
        int entry_max = 15;
        Entry *entries;
        int field_cnt = 0;

        int keysize_max = 0;

        boolean should_save = false;

    public:
        Config(BL::Logger *log);
        ~Config();

        BL::ResultCode_t begin(const char *filename,
                               ConfigTemplate_t *config_templates, 
                               int count);

        const char *getFilename();
        void setFilename(const char *name);

        void setTokenCount(size_t count);
        size_t getTokenCount();
        size_t estimateTokenCount();

        void setEntryCount(size_t count);
        size_t getEntryCount();

        void setFieldCount(int count);
        int getFieldCount();

        void setKeySize(int size);
        int getKeySize();

        Entry *getEntry(const char *key);
        Entry *getEntry(int confid);
        
        int setValue(const char *key, const char *value);
        char *getValue(const char *key);

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