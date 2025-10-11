#ifndef color_h
#define color_h
#include <NeoPixelBus.h>
#include "config.hpp"

RgbColor colorFromSetting(const ColorSetting &setting)
{
    float hue = static_cast<float>(setting.hue) / 360.0f;
    float brightness = static_cast<float>(setting.brightness) / 100.0f;
    if (brightness <= 0.0f)
    {
        return RgbColor(0, 0, 0);
    }
    if (hue < 0.0f)
    {
        hue = 0.0f;
    }
    HsbColor hsbColor(hue, 1.0f, brightness);
    return RgbColor(hsbColor);
}

void renderAlarm(bool night = false, bool bg = false)
{
    int length = 1;

    float brightness = 1.0;
    if (night)
    {
        brightness = 0.1;
    }
    HsbColor animationColor(1, 1, brightness);

    if (bg)
    {
        bgStrip->ClearTo(off);
        length = config.config.bgLedCount;
    }
    else
    {

        strip->ClearTo(off);
        length = config.config.ledCount;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        if (i % 4 == 0)
        {
            if (bg)
            {
                bgStrip->SetPixelColor(i, animationColor);
            }
            else
            {
                strip->SetPixelColor(i, animationColor);
            }
        }
        yield();
    }
}

void renderRainbow(bool night = false, bool bg = false)
{
    int length = 1;

    float brightness = 1.0;
    if (night)
    {
        brightness = 0.1;
    }

    if (bg)
    {
        bgStrip->ClearTo(off);
        length = config.config.bgLedCount;
    }
    else
    {

        strip->ClearTo(off);
        length = config.config.ledCount;
    }

    float hueStep = 1.0 / (float)length;
    for (uint16_t i = 0; i < length; i++)
    {
        float hue = hueStep * i;
        HsbColor animationColor(hue, 1, brightness);

        if (bg)
        {
            bgStrip->SetPixelColor(i, animationColor);
        }
        else
        {
            strip->SetPixelColor(i, animationColor);
        }
        yield();
    }
}

void updateColors(bool isNight = false)
{

    if (isNight)
    {
        hourColor = colorFromSetting(config.config.hourColorDimmed);
        minuteColor = colorFromSetting(config.config.minuteColorDimmed);
        secondColor = colorFromSetting(config.config.secondColorDimmed);
        dot = colorFromSetting(config.config.hourDotColorDimmed);
        quarter = colorFromSetting(config.config.hourQuarterColorDimmed);
        segment = colorFromSetting(config.config.hourSegmentColorDimmed);
        bgColor = colorFromSetting(config.config.bgColorDimmed);
        dayColor = colorFromSetting(config.config.dayColorDimmed);
        monthColor = colorFromSetting(config.config.monthColorDimmed);
        weekdayColor = colorFromSetting(config.config.weekdayColorDimmed);
    }
    else
    {
        hourColor = colorFromSetting(config.config.hourColor);
        minuteColor = colorFromSetting(config.config.minuteColor);
        secondColor = colorFromSetting(config.config.secondColor);
        dot = colorFromSetting(config.config.hourDotColor);
        quarter = colorFromSetting(config.config.hourQuarterColor);
        segment = colorFromSetting(config.config.hourSegmentColor);
        bgColor = colorFromSetting(config.config.bgColor);
        dayColor = colorFromSetting(config.config.dayColor);
        monthColor = colorFromSetting(config.config.monthColor);
        weekdayColor = colorFromSetting(config.config.weekdayColor);
    }
}

#endif //color_h