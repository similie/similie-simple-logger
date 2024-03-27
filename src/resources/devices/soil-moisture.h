
#ifndef soil_moisture_h
#define soil_moisture_h
#define SOIL_MOISTURE_DEFAULT 0.0003879
#define MINERAL_SOIL_DEFAULT true
#include "resources/utils/sdi-12.h"

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
    bool mineral_soil = MINERAL_SOIL_DEFAULT;
    double multiple = SOIL_MOISTURE_DEFAULT;
    String valueMap[SOIL_MOISTURE_PARAMS] =
        {
            "vwc",
            "s_t"};

public:
    void setMultiple(double multiple)
    {
        this->multiple = multiple;
    }

    void setMineralSoil(bool mineral_soil)
    {
        this->mineral_soil = mineral_soil;
    }

    String getDeviceName()
    {
        return "SoilMoisture";
    }

    size_t getBuffSize()
    {

        return 75;
    }

    uint8_t getTotalSize()
    {
        return (uint8_t)SOIL_MOISTURE_PARAMS;
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

class SoilMoisture : public Device
{
private:
    SoilMoistureElements elements;
    SDI12Device *sdi;
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
    bool mineral_soil = MINERAL_SOIL_DEFAULT;
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
    uint8_t maintenanceCount();
    uint8_t paramCount();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, uint8_t attempt_count);
    void restoreDefaults();
};

#endif