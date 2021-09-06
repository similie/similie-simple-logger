#define SOIL_MOISTURE_DEFAULT 0.0003879
#define MINERAL_SOIL_DEFAULT true

#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/utils.h"
#include "resources/utils/serial_storage.h"
#include <stdint.h>

#ifndef soil_moisture_h
#define soil_moisture_h



struct VWCStruct
{
    uint8_t version;
    int minerals;
    double multiple;
};

class SoilMoisture : public Device
{
private:
    String valueMap[2] =
        {
            "vwc",
            "s_t"};
    enum
    {
        vwc,
        soil_temp,
    };
    Bootstrap *boots;
    String serialMsgStr = "0R0!";
    String deviceName = "SoilMoisture";
    Utils utils;
    uint16_t saveAddressForMoisture = -1;
    double multiple = SOIL_MOISTURE_DEFAULT;
    float multiplyValue(float value);
    int setMoistureCalibration(String read);
    int setMineralSoilCalibration(String read);
    void setDeviceAddress();
    void pullEpromData();
    void setFunctions();
    void parseSerial(String ourReading);
    bool readyRead = false;
    bool readCompile = false;
    bool mineral_soil = MINERAL_SOIL_DEFAULT;
    bool readReady();
    size_t readSize();
    u8_t maintenanceTick = 0;
    String ourReading = "";
    String getReadContent();
    String uniqueName();
    bool hasSerialIdentity();
    String constrictSerialIdentity();
    String serialResponseIdentity();
    String replaceSerialResponceItem(String message);
    bool validMessageString(String message);
    unsigned int READ_THRESHOLD = 12;
    static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    float VALUE_HOLD[SoilMoisture::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];
    size_t readAttempt = 0;
    int sendIdentity = -1;
    String paramName(size_t index);
    String fetchReading();
    float applySoilMoistureEquation(float value);
public:
    ~SoilMoisture();
    SoilMoisture(Bootstrap *boots);
    SoilMoisture(Bootstrap *boots, int identity);
    void read();
    void loop();
    void clear();
    void print();
    void init();
    String name();
    u8_t matenanceCount();
    u8_t paramCount();
    size_t buffSize();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
    float extractValue(float values[], size_t key);
    float extractValue(float values[], size_t key, size_t max);
};

#endif