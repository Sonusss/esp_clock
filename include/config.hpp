#ifndef config_h
#define config_h
#include <ArduinoJson.h>
#if defined(ESP8266)
#include <FS.h>
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif

struct ColorSetting
{
    uint16_t hue;
    uint8_t brightness;
};

struct ConfigData
{
    char hostname[64];
    char timeserver[64];
    char timezone[64];

    ColorSetting hourColor;
    ColorSetting minuteColor;
    ColorSetting secondColor;

    ColorSetting hourColorDimmed;
    ColorSetting minuteColorDimmed;
    ColorSetting secondColorDimmed;

    bool hourDot;
    bool hourSegment;
    bool hourQuarter;

    ColorSetting hourDotColor;
    ColorSetting hourSegmentColor;
    ColorSetting hourQuarterColor;
    ColorSetting hourDotColorDimmed;
    ColorSetting hourSegmentColorDimmed;
    ColorSetting hourQuarterColorDimmed;

    bool dayMonth;
    uint32_t monthOffset;
    uint32_t dayOffset;
    uint32_t weekdayOffset;

    ColorSetting monthColor;
    ColorSetting dayColor;
    ColorSetting weekdayColor;

    ColorSetting monthColorDimmed;
    ColorSetting dayColorDimmed;
    ColorSetting weekdayColorDimmed;

    uint16_t nightTimeBegins;
    uint16_t nightTimeEnds;

    char hourHandStyle[8];
    bool hourLight;
    bool blendColors;
    bool fluidMotion;

    bool alarmActive;
    uint32_t alarmTime;

    bool bgLight;
    ColorSetting bgColor;
    ColorSetting bgColorDimmed;
    uint32_t bgLedPin;
    uint32_t bgLedCount;

    uint32_t ledPin;
    uint32_t ledCount;
    uint32_t ledRoot;

    char language[3];

    bool mqttActive;
    bool mqttTLS;
    char mqttServer[64];
    char mqttUser[128];
    char mqttPassword[128];
    char mqttBaseTopic[128];
    char mqttFingerprint[128];
    uint16_t mqttPort;

    bool tlsBundleLoaded;
};

class Config
{
public:
    Config();
    ConfigData config = {};
    void save();
    void load();
    void configToJSON(JsonDocument &doc, bool skipSensitiveData = false);
    bool JSONToConfig(JsonDocument &doc, bool skipSensitiveData = false);
    bool locked = false;
    bool forceReset = false;
    bool tainted = false;

private:
    bool _resetRequest = false;
    uint8_t _validateInt(uint8_t val, uint8_t min, uint8_t max);
};

#endif //config_h