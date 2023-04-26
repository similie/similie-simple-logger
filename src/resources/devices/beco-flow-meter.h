
#ifndef beco_flow_meter_h
#define beco_flow_meter_h

#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"

#include "resources/utils/utils.h"

#define FLOW_PIN_DEFAULT D1 // Stripe Green
// #define FLOW_PIN_DEFAULT D2 // Blue/Or Strip Blue // 5.85
#define CALIBRATION_FACTOR_DEFAULT 0.001
#define CALIBRATION_DIFFERENCE_DEFAULT 1.91 // 2.7
#define PARAM_LENGTH_FLOW 2
#define READ_COUNT_DEBOUNCE 110

struct BecoFlowStruct
{
    uint8_t version;
    unsigned long totalMilliLitres;
    double calibrationFactor;
    double calibrationDifference;
    double startingPosition;
};

class BecoFlowMeter : public Device
{
private:
    const size_t PARAM_LENGTH = PARAM_LENGTH_FLOW;
    String valueMap[PARAM_LENGTH_FLOW] =
        {
            "b_c_flow",
            "b_t_flow"};
    enum
    {
        c_flow,
        t_flow,
    };
    bool instantated = false;
    uint16_t saveAddressForFlow = -1;
    Bootstrap *boots;
    String deviceName = "BecoFlowMeter";
    void configurePin();
    unsigned long getToltalFlowMiliLiters();
    void setLifeTimeFlow();
    int clearTotalCount(String val);
    BecoFlowStruct getProm();
    void setInterrupt();
    void setConfiguration();
    void setTotal();
    // bool isDisconnected();
    void pulseCounter();
    void removeInterrupt();
    void setListeners();
    void processRead();
    void saveEEPROM(BecoFlowStruct storage);
    int setCalibrationFactor(String read);
    int setCalibrationDifference(String read);
    int setStartingPosition(String read);
    int getPin();
    void setFlow();
    void setDeviceAddress();
    void setIdentity(int identity);
    int sendIdentity = -1;
    double calibrationFactor = CALIBRATION_FACTOR_DEFAULT;
    double calibrationDifference = CALIBRATION_DIFFERENCE_DEFAULT;
    double startingPosition = 0;
    int flowPin = FLOW_PIN_DEFAULT;
    volatile byte pulseCount;
    float flowRate = 0.0;
    unsigned long countIteration = 0;
    unsigned int flowMilliLitres = 0;
    unsigned long currentFlow = 0;
    unsigned long totalMilliLitres = 0;
    unsigned long lastTime = 0;
    Utils utils;
    bool hasSerialIdentity();
    String getParamName(size_t index);
    String uniqueName();
    String appendIdentity();

public:
    ~BecoFlowMeter();
    BecoFlowMeter();
    BecoFlowMeter(Bootstrap *boots, int sendIdentity);
    BecoFlowMeter(Bootstrap *boots);
    BecoFlowMeter(Bootstrap *boots, int sendIdentity, int readPin);
    void setPin(int pin);
    String name();
    void read();
    void loop();
    uint8_t matenanceCount();
    uint8_t paramCount();
    void clear();
    void print();
    size_t buffSize();
    void init();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, uint8_t attempt_count);
};

#endif