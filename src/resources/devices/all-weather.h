#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/serial_storage.h"
#include "resources/utils/utils.h"
#include <stdint.h>

#ifndef all_weather_h
#define all_weather_h
#define READ_OVER_WIRE true
#define SINGLE_SAMPLE true
#define READ_ON_LOW_ONLY true
#define DEVICE_CONNECTED_PIN D7
#define TOTAL_PARAM_VALUES 17

class AllWeather : public Device
{
private:
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
    void parseSerial(String ourReading);
    bool readyRead = false;
    bool readCompile = false;
    bool readReady();
    size_t readSize();
    bool isConnected();
    String deviceName = "AllWeather";
    u_int8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    bool hasSerialIdentity();
    String constrictSerialIdentity();
    String serialResponseIdentity();
    String replaceSerialResponceItem(String message);
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    float VALUE_HOLD[AllWeather::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];
    size_t readAttempt = 0;
    int sendIdentity = -1;
    String fetchReading();
    void readSerial();
    void readWire();
    static const unsigned long WIRE_TIMEOUT = 2000;
    String getWire(String);
    void runSingleSample();

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
    void nullifyPayload(const char *key);
    u_int8_t maintenanceCount();
    u_int8_t paramCount();
    size_t buffSize();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, u_int8_t attempt_count);
    float extractValue(float values[], size_t key);
    float extractValue(float values[], size_t key, size_t max);
};

#endif