#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/serial_storage.h"
#include "resources/utils/utils.h"
#include <stdint.h>

#ifndef rika_air_quality_h
#define rika_air_quality_h

static const size_t RIKA_MAX_PARAM_SIZE = 10;

class RikaAirQuality : public Device
{
private:
    String valueMap[RIKA_MAX_PARAM_SIZE] =
        {"temp", "hum", "press", "pm2_5", "pm10", "o3", "ch4", "co2", "no2", "so2"};
    enum
    {
        temp,
        hum,
        press,
        pm2_5,
        pm10,
        o3,
        ch4,
        co2,
        no2,
        so2
    };
    Bootstrap *boots;
    Utils utils;
    void parseSerial(String ourReading);
    bool readyRead = false;
    bool readCompile = false;
    bool readReady();
    size_t readSize();
    const String REQUEST_TAG = "airquality";
    String deviceName = "AirQuality";
    u_int8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    String serialResponseIdentity();
    String replaceSerialResponceItem(String message);
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    float VALUE_HOLD[RikaAirQuality::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];
    size_t readAttempt = 0;
    String fetchReading();

public:
    ~RikaAirQuality();
    RikaAirQuality(Bootstrap *boots);
    void read();
    void loop();
    void clear();
    void print();
    void init();
    String name();
    void nullifyPayload(const char *key);
    u_int8_t matenanceCount();
    u_int8_t paramCount();
    size_t buffSize();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, u_int8_t attempt_count);
    float extractValue(float values[], size_t key);
    float extractValue(float values[], size_t key, size_t max);
};

#endif