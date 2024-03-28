
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

    /**
     *
     * @brief applySoilMoistureEquation
     *
     * Processes the equation for soil calibration
     *
     * @param value
     * @return float
     */
    float applySoilMoistureEquation(float value)
    {
        const double MINERAL_SOIL_MULTIPLE = multiple;                                           // 3.879e-4; // pow(3.879, -4); //  0.0003879;
        const double SOILESS_MEDIA_MULTIPLE[] = {0.0000000006771, 0.000005105, 0.01302, 10.848}; // {6.771e-10, 5.105e-6, 1.302e-2, 10.848}; // 0.00000000006771;
        if (mineral_soil)
        {
            return roundf(((MINERAL_SOIL_MULTIPLE * value) - 0.6956) * 100);
        }
        else
        {
            double eq = ((SOILESS_MEDIA_MULTIPLE[0] * pow(value, 3.0)) - (SOILESS_MEDIA_MULTIPLE[1] * pow(value, 2.0)) + (SOILESS_MEDIA_MULTIPLE[2] * value)) - SOILESS_MEDIA_MULTIPLE[3];
            return roundf(eq * 100);
        }
    }

    /**
     * @private
     *
     * multiplyValue
     *
     * Returns the selected value with the configured multiplyer
     *
     * @return float
     */
    float multiplyValue(float value)
    {
        return ((value == NO_VALUE) ? NO_VALUE : applySoilMoistureEquation(value));
    }

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

    float extractValue(float values[], size_t key, size_t max) override
    {
        switch (key)
        {
        case vwc:
            return multiplyValue(utils.getMedian(values, max));
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