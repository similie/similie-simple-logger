#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/utils.h"
#include <stdint.h>

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
    void payloadRestorator(String payload);
    void parseSerial();
    bool readyRead = false;
    bool sendingOfflinePayload = false;
    bool readCompile = false;
    bool readReady();
    size_t readSize();
    void processPop(String value);
    String getPopStartIndex(String read);
    bool sendPopRead();
    size_t firstSpaceIndex(String value, u8_t index);
    Processor *holdProcessor;
    String popString = "";
    // long counter = 0;
    u8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    bool hasSerialIdentity();
    String constrictSerialIdentity();
    String serialResponseIdentity();
    bool inValidMessageString(String message);
    String replaceSerialResponceItem(String message);
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    float VALUE_HOLD[AllWeather::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];
    size_t skipMultiple(unsigned int size);
    size_t readAttempt = 0;
    int sendIdentity = -1;

public:
    ~AllWeather();
    AllWeather();
    AllWeather(Bootstrap *boots);
    AllWeather(Bootstrap *boots, int identity);
    void read();
    void loop();
    void clear();
    void print();
    void init();

    void nullifyPayload(const char *key);
    void storePayload(String payload, String topic);
    u8_t matenanceCount();
    u8_t paramCount();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
    float extractValue(float values[], size_t key);
    float extractValue(float values[], size_t key, size_t max);
    void popOfflineCollection(Processor *processor, String topic, u8_t count);
};

#endif