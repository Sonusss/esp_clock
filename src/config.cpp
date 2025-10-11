#include "config.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

#if defined(ESP8266)
#define MAXPINS 16
#elif definded(ESP32)
#define MAXPINS 35
#endif
#define MAXLEDS 360
Config::Config()
{
}

uint32_t _clampInt(uint32_t val, uint32_t min, uint32_t max)
{
        const uint32_t t = val < min ? min : val;
        return t > max ? max : t;
}

static uint8_t _hexToByte(char hi, char lo)
{
        auto hexToInt = [](char c) -> int {
                if (c >= '0' && c <= '9')
                        return c - '0';
                if (c >= 'A' && c <= 'F')
                        return c - 'A' + 10;
                if (c >= 'a' && c <= 'f')
                        return c - 'a' + 10;
                return 0;
        };

        return static_cast<uint8_t>((hexToInt(hi) << 4) | hexToInt(lo));
}

static ColorSetting _colorFromLegacyHex(const char *value, uint16_t defaultHue, uint8_t defaultBrightness)
{
        ColorSetting color{defaultHue, defaultBrightness};
        if (value == nullptr)
        {
                return color;
        }

        size_t len = strlen(value);
        if (len != 7 || value[0] != '#')
        {
                return color;
        }

        uint8_t r = _hexToByte(value[1], value[2]);
        uint8_t g = _hexToByte(value[3], value[4]);
        uint8_t b = _hexToByte(value[5], value[6]);

        float rf = static_cast<float>(r) / 255.0f;
        float gf = static_cast<float>(g) / 255.0f;
        float bf = static_cast<float>(b) / 255.0f;

        float maxVal = std::max(rf, std::max(gf, bf));
        float minVal = std::min(rf, std::min(gf, bf));
        float delta = maxVal - minVal;

        float hue = 0.0f;
        if (delta != 0.0f)
        {
                if (maxVal == rf)
                {
                        hue = 60.0f * ((gf - bf) / delta);
                        if (hue < 0.0f)
                        {
                                hue += 360.0f;
                        }
                }
                else if (maxVal == gf)
                {
                        hue = 60.0f * (((bf - rf) / delta) + 2.0f);
                }
                else
                {
                        hue = 60.0f * (((rf - gf) / delta) + 4.0f);
                }
        }

        float brightness = maxVal * 100.0f;

        color.hue = static_cast<uint16_t>(_clampInt(static_cast<uint32_t>(std::roundf(hue)), 0, 360));
        color.brightness = static_cast<uint8_t>(_clampInt(static_cast<uint32_t>(std::roundf(brightness)), 0, 100));
        return color;
}

static ColorSetting _parseColorSetting(JsonVariantConst variant, uint16_t defaultHue, uint8_t defaultBrightness)
{
        ColorSetting color{defaultHue, defaultBrightness};

        if (variant.is<JsonObjectConst>())
        {
                color.hue = static_cast<uint16_t>(_clampInt(static_cast<uint32_t>(variant["hue"] | defaultHue), 0, 360));
                color.brightness = static_cast<uint8_t>(_clampInt(static_cast<uint32_t>(variant["brightness"] | defaultBrightness), 0, 100));
                return color;
        }

        if (variant.is<const char *>())
        {
                return _colorFromLegacyHex(variant.as<const char *>(), defaultHue, defaultBrightness);
        }

        return color;
}

static void _colorSettingToJson(JsonDocument &doc, const char *name, const ColorSetting &color)
{
        JsonObject obj = doc.createNestedObject(name);
        obj["hue"] = color.hue;
        obj["brightness"] = color.brightness;
}

void Config::configToJSON(JsonDocument &doc, bool skipSensitiveData)
{
        Config::locked = true;
        if (!skipSensitiveData)
        {
                doc["hostname"] = config.hostname;
                doc["timeserver"] = config.timeserver;
                doc["timezone"] = config.timezone;
        }

        _colorSettingToJson(doc, "hourColor", config.hourColor);
        _colorSettingToJson(doc, "minuteColor", config.minuteColor);
        _colorSettingToJson(doc, "secondColor", config.secondColor);

        _colorSettingToJson(doc, "hourColorDimmed", config.hourColorDimmed);
        _colorSettingToJson(doc, "minuteColorDimmed", config.minuteColorDimmed);
        _colorSettingToJson(doc, "secondColorDimmed", config.secondColorDimmed);

        doc["hourDot"] = config.hourDot;
        doc["hourSegment"] = config.hourSegment;
        doc["hourQuarter"] = config.hourQuarter;

        _colorSettingToJson(doc, "hourDotColor", config.hourDotColor);
        _colorSettingToJson(doc, "hourSegmentColor", config.hourSegmentColor);
        _colorSettingToJson(doc, "hourQuarterColor", config.hourQuarterColor);

        _colorSettingToJson(doc, "hourDotColorDimmed", config.hourDotColorDimmed);
        _colorSettingToJson(doc, "hourSegmentColorDimmed", config.hourSegmentColorDimmed);
        _colorSettingToJson(doc, "hourQuarterColorDimmed", config.hourQuarterColorDimmed);

        doc["dayMonth"] = config.dayMonth;
        _colorSettingToJson(doc, "dayColor", config.dayColor);
        _colorSettingToJson(doc, "monthColor", config.monthColor);
        _colorSettingToJson(doc, "weekdayColor", config.weekdayColor);
        _colorSettingToJson(doc, "dayColorDimmed", config.dayColorDimmed);
        _colorSettingToJson(doc, "monthColorDimmed", config.monthColorDimmed);
        _colorSettingToJson(doc, "weekdayColorDimmed", config.weekdayColorDimmed);
        doc["monthOffset"] = config.monthOffset + 1;
        doc["dayOffset"] = config.dayOffset + 1;
        doc["weekdayOffset"] = config.weekdayOffset + 1;

        doc["nightTimeBegins"] = config.nightTimeBegins;
        doc["nightTimeEnds"] = config.nightTimeEnds;

        doc["hourHandStyle"] = config.hourHandStyle;
        doc["hourLight"] = config.hourLight;
        doc["blendColors"] = config.blendColors;
        doc["fluidMotion"] = config.fluidMotion;

        doc["alarmTime"] = config.alarmTime;
        doc["alarmActive"] = config.alarmActive;

        doc["ledPin"] = config.ledPin;
        doc["ledCount"] = config.ledCount;
        doc["ledRoot"] = config.ledRoot + 1;

        doc["bgLight"] = config.bgLight;
        doc["bgLedPin"] = config.bgLedPin;
        doc["bgLedCount"] = config.bgLedCount;
        _colorSettingToJson(doc, "bgColor", config.bgColor);
        _colorSettingToJson(doc, "bgColorDimmed", config.bgColorDimmed);

        doc["language"] = config.language;

        if (!skipSensitiveData)
        {
                doc["mqttActive"] = config.mqttActive;
                doc["mqttServer"] = config.mqttServer;
                doc["mqttUser"] = config.mqttUser;
                doc["mqttPassword"] = config.mqttPassword;
                doc["mqttPort"] = config.mqttPort;
                doc["mqttBaseTopic"] = config.mqttBaseTopic;
        }

#if defined(UART_MODE) || defined(DMA_MODE)
        doc["pinsLocked"] = true;
#else
        doc["pinsLocked"] = false;
#endif
        Config::locked = false;
}

bool Config::JSONToConfig(JsonDocument &doc, bool skipSensitiveData)
{

        Config::locked = true;
        if (doc.containsKey("hostname"))
        {
                strlcpy(config.hostname,
                        doc["hostname"],
                        sizeof(config.hostname));
        }
        else
        {
#if defined(ESP8266)
                uint32_t chipid = ESP.getChipId();
#elif defined(ESP32)
                uint64_t chipid = ESP.getEfuseMac();
#endif
                snprintf(config.hostname, sizeof(config.hostname), "ESPCLOCK-%06X", chipid);
        }

        strlcpy(config.timeserver,
                doc["timeserver"] | "pool.ntp.org",
                sizeof(config.timeserver));
        strlcpy(config.timezone,
                doc["timezone"] | "Europe/Berlin",
                sizeof(config.timezone));

        config.hourColor = _parseColorSetting(doc["hourColor"], 0, 100);
        config.minuteColor = _parseColorSetting(doc["minuteColor"], 120, 100);
        config.secondColor = _parseColorSetting(doc["secondColor"], 240, 100);

        config.hourColorDimmed = _parseColorSetting(doc["hourColorDimmed"], 0, 47);
        config.minuteColorDimmed = _parseColorSetting(doc["minuteColorDimmed"], 120, 47);
        config.secondColorDimmed = _parseColorSetting(doc["secondColorDimmed"], 240, 47);

        strlcpy(config.hourHandStyle,
                doc["hourHandStyle"] | "simple",
                sizeof(config.hourHandStyle));
        config.hourDot = doc["hourDot"] | false;
        config.hourSegment = doc["hourSegment"] | false;
        config.hourQuarter = doc["hourQuarter"] | false;

        config.hourDotColor = _parseColorSetting(doc["hourDotColor"], 0, 0);
        config.hourSegmentColor = _parseColorSetting(doc["hourSegmentColor"], 0, 0);
        config.hourQuarterColor = _parseColorSetting(doc["hourQuarterColor"], 240, 0);

        config.hourDotColorDimmed = _parseColorSetting(doc["hourDotColorDimmed"], 0, 0);
        config.hourSegmentColorDimmed = _parseColorSetting(doc["hourSegmentColorDimmed"], 0, 0);
        config.hourQuarterColorDimmed = _parseColorSetting(doc["hourQuarterColorDimmed"], 0, 0);

        config.dayMonth = doc["dayMonth"] | false;
        config.dayColor = _parseColorSetting(doc["dayColor"], 312, 100);
        config.monthColor = _parseColorSetting(doc["monthColor"], 59, 100);
        config.weekdayColor = _parseColorSetting(doc["weekdayColor"], 167, 100);

        config.dayColorDimmed = _parseColorSetting(doc["dayColorDimmed"], 312, 48);
        config.monthColorDimmed = _parseColorSetting(doc["monthColorDimmed"], 59, 34);
        config.weekdayColorDimmed = _parseColorSetting(doc["weekdayColorDimmed"], 167, 46);

        config.nightTimeBegins = doc["nightTimeBegins"] | 1320;
        config.nightTimeBegins = _clampInt(config.nightTimeBegins, 0, 1440);

        config.nightTimeEnds = doc["nightTimeEnds"] | 480;
        config.nightTimeEnds = _clampInt(config.nightTimeEnds, 0, 1440);

        config.hourLight = doc["hourLight"] | false;
        config.blendColors = doc["blendColors"] | true;
        config.fluidMotion = doc["fluidMotion"] | true;

        config.alarmActive = doc["alarmActive"] | false;
        config.alarmTime = doc["alarmTime"] | 480;

        config.bgLight = doc["bgLight"] | false;
        config.bgColor = _parseColorSetting(doc["bgColor"], 0, 0);
        config.bgColorDimmed = _parseColorSetting(doc["bgColorDimmed"], 0, 0);

#if defined(BITBANG_MODE) || defined(DEBUG_BUILD) || defined(ESP32)
        config.bgLedPin = doc["bgLedPin"] | 15;
        config.bgLedPin = _clampInt(config.bgLedPin, 0, MAXPINS);
#elif defined(UART_MODE)
        if (!skipSensitiveData)
                config.bgLedPin = 1;
#elif defined(DMA_MODE)
        if (!skipSensitiveData)
                config.bgLedPin = 2;
#endif

        config.bgLedCount = doc["bgLedCount"] | 60;
        config.bgLedCount = _clampInt(config.bgLedCount, 0, MAXLEDS);

#if defined(BITBANG_MODE) || defined(DEBUG_BUILD) || defined(ESP32)
        config.ledPin = doc["ledPin"] | 4;
        config.ledPin = _clampInt(config.ledPin, 0, MAXPINS);
#elif defined(UART_MODE)
        if (!skipSensitiveData)
                config.ledPin = 2;
#elif defined(DMA_MODE)
        if (!skipSensitiveData)
                config.ledPin = 3;
#endif

        config.ledCount = doc["ledCount"] | 60;
        config.ledCount = _clampInt(config.ledCount, 0, MAXLEDS);

        config.ledRoot = doc["ledRoot"] | 1;
        config.ledRoot = _clampInt(config.ledRoot, 1, MAXLEDS);

        config.dayOffset = doc["dayOffset"] | 1;
        config.dayOffset = _clampInt(config.dayOffset, 1, MAXLEDS);

        config.monthOffset = doc["monthOffset"] | 1;
        config.monthOffset = _clampInt(config.monthOffset, 1, MAXLEDS);

        config.weekdayOffset = doc["weekdayOffset"] | 1;
        config.weekdayOffset = _clampInt(config.weekdayOffset, 1, MAXLEDS);

        strlcpy(config.language,
                doc["language"] | "en",
                sizeof(config.language));

        if (!skipSensitiveData)
        {
                config.mqttActive = doc["mqttActive"] | false;
                strlcpy(config.mqttServer,
                        doc["mqttServer"] | "mqtthost",
                        sizeof(config.mqttServer));
                strlcpy(config.mqttUser,
                        doc["mqttUser"] | "mqttuser",
                        sizeof(config.mqttUser));
                strlcpy(config.mqttUser,
                        doc["mqttUser"] | "username",
                        sizeof(config.mqttUser));
                strlcpy(config.mqttPassword,
                        doc["mqttPassword"] | "password",
                        sizeof(config.mqttPassword));
                config.mqttPort = doc["mqttPort"] | 1883;
                config.mqttPort = _clampInt(config.mqttPort, 1, 65535);
        }
        char defaultBaseTopic[144] = "espneopixelclock/";
        snprintf(defaultBaseTopic, sizeof(defaultBaseTopic), "espneopixelclock/%s", config.hostname);

        strlcpy(config.mqttBaseTopic,
                doc["mqttBaseTopic"] | defaultBaseTopic,
                sizeof(config.mqttBaseTopic));

        config.ledRoot--;
        config.dayOffset--;
        config.monthOffset--;
        config.weekdayOffset--;

        if (doc["reset"] == true)
        {
                Config::_resetRequest = true;
        }
        else
        {
                Config::_resetRequest = false;
        };

        Config::locked = false;

        if (doc["saveData"] == true)
        {
                return true;
        }
        else
        {
                return false;
        };
}

void Config::load()
{
        // Open file for reading
        Config::locked = true;

#if defined(ESP8266)
        File sourcefile = LittleFS.open("data.json", "r");
#elif defined(ESP32)
        File sourcefile = SPIFFS.open("/data.json", "r");
#endif

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        DynamicJsonDocument doc(2048);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, sourcefile);
        doc.shrinkToFit();
        Config::JSONToConfig(doc);
        if (error)
        {
                sourcefile.close();
                Serial.println(F("Failed to read file, using default configuration"));
                Config::save();
                return;
        }
        // Copy values from the JsonDocument to the Config
        // Close the file (Curiously, File's destructor doesn't close the file)
        sourcefile.close();
        Config::locked = false;
}

void Config::save()
{
        bool format = false;
        // Delete existing file, otherwise the configuration is appended to the file
        Config::locked = true;
#if defined(ESP8266)
        File targetfile = LittleFS.open("data.json", "w");
#elif defined(ESP32)
        File targetfile = SPIFFS.open("/data.json", "w");
#endif

        // Open file for writing
        if (!targetfile)
        {
                Serial.println(F("Failed to create config file. Reformatting FS and rebooting."));
                format = true;

        }

        DynamicJsonDocument doc(2048);
        Config::configToJSON(doc);
        doc.shrinkToFit();
        // Serialize JSON to file
        if (serializeJson(doc, targetfile) == 0)
        {
                Serial.println(F("Failed to write to config file. Reformatting FS and rebooting."));
                format = true;

        }
        if (format) {
#if defined(ESP8266)
                LittleFS.format();
                ESP.restart();
#elif defined(ESP32)
                SPIFFS.format();
                ESP.restart();
#endif
        }

        // Close the file
        targetfile.close();
        Config::locked = false;
        Config::forceReset = Config::_resetRequest;
}
