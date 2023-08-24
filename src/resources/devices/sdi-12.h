#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/serial_storage.h"
#include "resources/utils/utils.h"
#include <stdint.h>

#ifndef sdi_12_h
#define sdi_12_h
#define READ_OVER_WIRE true
#define SINGLE_SAMPLE true
#define READ_ON_LOW_ONLY true
#define DEVICE_CONNECTED_PIN D7
#define TOTAL_PARAM_VALUES 17

enum
{
    // all weather
    solar = 0,
    precipitation = 1,
    strikes = 2,
    strike_distance = 3,
    wind_speed = 4,
    wind_direction = 5,
    gust_wind_speed = 6,
    air_temperature = 7,
    vapor_pressure = 8,
    atmospheric_pressure = 9,
    relative_humidity = 10,
    humidity_sensor_temperature = 11,
    x_orientation = 12,
    y_orientation = 13,
    null_val = 14,
    wind_speed_north = 15,
    wind_speed_east = 16,
    // air quality
    temp = 0,
    hum = 1,
    press = 2,
    pm2_5 = 3,
    pm10 = 4,
    o3 = 5,
    ch4 = 6,
    co2 = 7,
    no2 = 8,
    so2 = 9,
    // soil moisture
    vwc = 0,
    soil_temp = 1,
};

class SDIParamElements
{
private:
    String deviceName = "SDI12Device";
    uint8_t totalSize = 0;
    String *valueMap;
    size_t buffSize = 0;

public:
    SDIParamElements();
    SDIParamElements(String deviceName, String *valueMap, uint8_t size, size_t buffSize)
    {
        this->deviceName = deviceName;
        this->valueMap = valueMap;
        this->totalSize = size;
        this->buffSize = buffSize;
    }
    float valueHold[Bootstrap::OVERFLOW_VAL][Bootstrap::OVERFLOW_VAL];
    String getDeviceName()
    {
        return deviceName;
    }

    size_t getBuffSize()
    {

        return buffSize;
    }

    uint8_t getTotalSize()
    {
        return totalSize;
    }
    String *getValueMap()
    {
        return valueMap;
    }

    float *getMappedValue(uint8_t iteration)
    {
        return valueHold[iteration];
    }
    float getMappedValue(uint8_t iteration, uint8_t index)
    {
        return valueHold[iteration][index];
    }

    void setMappedValue(float value, uint8_t iteration, uint8_t index)
    {
        valueHold[iteration][index] = value;
    }

    virtual float extractValue(float values[], size_t key, size_t max)
    {
        return 0;
    }
};

class SDI12Device : public Device
{
protected:
    Bootstrap *boots;
    SDIParamElements *elements;
    String serialMsgStr = "~R0!";
    Utils utils;
    void parseSerial(String ourReading);
    bool readyRead = false;
    bool readCompile = false;
    bool readReady();
    size_t readSize();
    bool isConnected();
    u_int8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    bool hasSerialIdentity();
    String constrictSerialIdentity();
    String serialResponseIdentity();
    String replaceSerialResponseItem(String message);
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    size_t readAttempt = 0;
    int sendIdentity = -1;
    String fetchReading();
    void readSerial();
    void readWire();
    static const unsigned long WIRE_TIMEOUT = 1100;
    String getWire(String);
    void runSingleSample();
    String getCmd();
    String uniqueName();

public: // SDI12Device(boots, elements)
    ~SDI12Device();
    SDI12Device(Bootstrap *);
    SDI12Device(Bootstrap *, int);
    void setElements(SDIParamElements *);
    void read();
    void loop();
    void clear();
    void print();
    virtual void init();
    String name();
    void nullifyPayload(const char *key);
    u_int8_t maintenanceCount();
    u_int8_t paramCount();
    size_t buffSize();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, u_int8_t attempt_count);
    virtual float extractValue(float values[], size_t key);
    virtual float extractValue(float values[], size_t key, size_t max);
};

#endif