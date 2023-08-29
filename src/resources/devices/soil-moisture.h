
#ifndef soil_moisture_h
#define soil_moisture_h
#define SOIL_MOISTURE_DEFAULT 0.0003879
#define MINERAL_SOIL_DEFAULT true
#include "sdi-12.h"

#define SOIL_MOISTURE_PARAMS 2

struct VWCStruct
{
    uint8_t version;
    int minerals;
    double multiple;
};

class SoilMoistureElements : public SDIParamElements
{
private:
    Utils utils;
    String deviceName = "SoilMoisture";
    size_t buffSize = 75;
    uint8_t totalSize = (uint8_t)SOIL_MOISTURE_PARAMS;
    String valueMap[SOIL_MOISTURE_PARAMS] =
        {
            "vwc",
            "s_t"};

public:
    SoilMoistureElements() : SDIParamElements(deviceName, valueMap, totalSize, buffSize)
    { 
    }
};

class SoilMoisture : public SDI12Device
{
private:
    SoilMoistureElements elements;
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
    // bool readyRead = false;
    // bool readCompile = false;
    bool mineral_soil = MINERAL_SOIL_DEFAULT;
    // bool readReady();
    // size_t readSize();
    // uint8_t maintenanceTick = 0;
    // String ourReading = "";
    // String getReadContent();
    // String uniqueName();
    // bool hasSerialIdentity();
    // String constrictSerialIdentity();
    // String serialResponseIdentity();
    // String replaceSerialResponceItem(String message);
    // bool validMessageString(String message);
    // unsigned int READ_THRESHOLD = 12;
    // static const size_t PARAM_LENGTH = sizeof(valueMap) / sizeof(String);
    // float VALUE_HOLD[SoilMoisture::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];
    // size_t readAttempt = 0;
    // int sendIdentity = -1;
    // String paramName(size_t index);
    // String fetchReading();
    float applySoilMoistureEquation(float value);

public:
    ~SoilMoisture();
    SoilMoisture(Bootstrap *boots);
    SoilMoisture(Bootstrap *boots, int identity);
    // void read();
    // void loop();
    // void clear();
    // void print();
    void init();
    // String name();
    // uint8_t maintenanceCount();
    // uint8_t paramCount();
    // size_t buffSize();
    void restoreDefaults();
    // void publish(JSONBufferWriter &writer, uint8_t attempt_count);
    float extractValue(float values[], size_t key);
    float extractValue(float values[], size_t key, size_t max);
};

#endif