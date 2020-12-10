#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"

#ifndef all_weather_h
#define all_weather_h

class AllWeather : public Device
{
private:
    String valueMap[17] =
        {
            "solar",
            "precipitation",
            "strikes",
            "strike_distance",
            "wind_speed",
            "wind_direction",
            "gust_wind_speed",
            "temperature",
            "atmospheric_pressure",
            "pressure",
            "humidity",
            "humidity_sensor_temperature",
            "x_orientation",
            "y_orientation",
            "null",
            "wind_speed_north",
            "wind_speed_east"};
    enum
    {
        solar,
        precipitation,
        strikes,
        strike_distance,
        wind_speed,
        wind_direction,
        gust_wind_speed,
        air_temperature,
        vapor_pressure,
        atmospheric_pressure,
        relative_humidity,
        humidity_sensor_temperature,
        x_orientation,
        y_orientation,
        null_val,
        wind_speed_north,
        wind_speed_east
    };
    Bootstrap *boots;
    String serialMsgStr = "0R0!";

    Utils utils;
    int readSerial();
    void parseSerial();
    bool readyRead = false;
    bool readCompile = false;
    // long counter = 0;
    String ourReading = "";
    static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    float VALUE_HOLD[AllWeather::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];

public:
    ~AllWeather();
    AllWeather();
    AllWeather(Bootstrap *boots);
    void read();
    void loop();
    void clear();
    void print();
    void init();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
    float extractValue(float values[], size_t key);
};

#endif