#include "sdi-12.h"

#ifndef all_weather_h
#define all_weather_h
#define TOTAL_PARAM_VALUES 17

class AllWeatherElements : public SDIParamElements
{
private:
    Utils utils;
    String deviceName = "AllWeather";
    size_t buffSize = 250;
    uint8_t maxSize = TOTAL_PARAM_VALUES;
    String valueMap[TOTAL_PARAM_VALUES] =
        {
            "sol",
            "pre",
            "s",
            "s_d",
            "ws",
            "wd",
            "gws",
            "t",
            "a_p",
            "p",
            "h",
            "hst",
            "x",
            "y",
            "null",
            "wsn",
            "wse"};

public:
    AllWeatherElements() : SDIParamElements(deviceName, valueMap, maxSize, buffSize)
    {
    }

    float extractValue(float values[], size_t key, size_t max)
    {
        switch (key)
        {
        case gust_wind_speed:
        case strike_distance:
            return utils.getMax(values, max);
        case precipitation:
        case strikes:
            return utils.getSum(values, max);
        default:
            return utils.getMedian(values, max);
        }
    }
};

class AllWeather : public SDI12Device
{
private:
    AllWeatherElements elements;

public:
    AllWeather(Bootstrap *boots);
    AllWeather(Bootstrap *boots, int identity);
};

#endif