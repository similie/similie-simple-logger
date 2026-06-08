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

typedef struct
{
    String param;
    uint16_t value;
} ReadValues;

class RikaAirQuality : public Device
{
private:
    static const size_t RIKA_MAX_PARAM_SIZE = 10;
    static const uint8_t STRING_CONVERT_SIZE = 30;
    const uint16_t WAIT_TIMEOUT = 2000;
    String stringConvertBuffer[STRING_CONVERT_SIZE];
    String valueMap[RIKA_MAX_PARAM_SIZE] =
        {"T", "H", "P", "PM2", "PM10", "03", "CH4", "CO2", "NO2", "SO2"};
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
    String paramHold[RIKA_MAX_PARAM_SIZE];
    ReadValues paramValues[RIKA_MAX_PARAM_SIZE];
    size_t paramHoldSize = 0;
    size_t valueStore = 0;
    void stuffValues();
    void zeroOutParamValueHold();
    size_t applyStringValue(size_t index, String value);
    size_t populateStringContent();
    void printStringContent();
    Bootstrap *boots;
    Utils utils;
    int address = -1;
    String config = "";
    const String REQUEST_TAG = "rs485";
    String deviceName = "AirQuality";
    u_int8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    size_t readAttempt = 0;
    String getWire(String content);
    void parseReadContent(String);
    void processValues(JSONBufferWriter &writer);
    float convertValue(ReadValues *);
    int getIndexForParam(String);

public:
    ~RikaAirQuality();
    RikaAirQuality(Bootstrap *boots);
    RikaAirQuality(Bootstrap *boots, String, int);
    RikaAirQuality(Bootstrap *boots, const char *, int);
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
};

#endif