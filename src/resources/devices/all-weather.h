#include "resources/utils/sdi-12.h"

#ifndef all_weather_h
#define all_weather_h
#define ALL_WEATHER_PARAMS_TOTAL 17

// class AllWeatherElements : public SDIParamElements
// {
// private:
//     const String deviceName = "AllWeather";
//     const size_t buffSize = 250;
//     const uint8_t totalSize = (uint8_t)ALL_WEATHER_PARAMS_TOTAL;
//     String valueMap[ALL_WEATHER_PARAMS_TOTAL] =
//         {
//             "sol",
//             "pre",
//             "s",
//             "s_d",
//             "ws",
//             "wd",
//             "gws",
//             "t",
//             "a_p",
//             "p",
//             "h",
//             "hst",
//             "x",
//             "y",
//             "null",
//             "wsn",
//             "wse"};

// public:
//     AllWeatherElements() : SDIParamElements(deviceName, valueMap, totalSize, buffSize)
//     {
//     }

//     float extractValue(float values[], size_t key, size_t max) override
//     {
//         switch (key)
//         {
//         case gust_wind_speed:
//         case strike_distance:
//             return utils.getMax(values, max);
//         case precipitation:
//         case strikes:
//             return utils.getSum(values, max);
//         default:
//             return utils.getMedian(values, max);
//         }
//     }
// };

class AllWeatherElements : public SDIParamElements
{
private:
    String valueMap[ALL_WEATHER_PARAMS_TOTAL] =
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
    String getDeviceName()
    {
        return "AllWeather";
    }

    size_t getBuffSize()
    {

        return 250;
    }

    uint8_t getTotalSize()
    {
        return (uint8_t)ALL_WEATHER_PARAMS_TOTAL;
    }
    String *getValueMap()
    {
        return valueMap;
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

class AllWeather : public Device
{
private:
    AllWeatherElements elements;
    SDI12Device *sdi;

public:
    ~AllWeather();
    AllWeather(Bootstrap *boots);
    AllWeather(Bootstrap *boots, int identity);

    void read();
    void loop();
    void clear();
    void print();
    void init();
    String name();
    uint8_t maintenanceCount();
    uint8_t paramCount();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, uint8_t attempt_count);
    void restoreDefaults();
};

#endif