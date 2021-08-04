#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"
#include "math.h"

#define DIGITAL_DEFAULT false

#define DEF_DISTANCE_READ_DIG_CALIBRATION 0.01724137931
#define DEF_DISTANCE_READ_AN_CALIBRATION 0.335

#ifndef wl_device_h
#define wl_device_h

#define DIG_PIN D8  // stripe blue off port1
#define AN_PIN A1 // stripe blue line off port0

const size_t WL_PARAM_SIZE = 1;

struct WLStruct {
    uint8_t version;
    double calibration;
    char digital;
};



class WlDevice : public Device
{
private:
    bool digital = DIGITAL_DEFAULT;
    uint16_t saveAddressForWL = 0;
    double currentCalibration = (DIGITAL_DEFAULT) ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
    void configSetup();
    void restoreDefaults();
    bool isDigital();
    int setDigitalCloud(String read);
    int setCalibration(String read);
    void saveEEPROM(WLStruct storage);
    String appendIdentity();
    WLStruct config;
    String uniqueName();
    bool hasSerialIdentity();
    void setCloudFunctions();
    Bootstrap *boots;
    String deviceName = "wl";
    int sendIdentity = -1;
    //String readParams[WL_PARAM_SIZE] = {"wl_pw", "hydrometric_level"};
    // String readParams[WL_PARAM_SIZE] = {"wl_pw"}; // water tank  or wl
    String readParams[WL_PARAM_SIZE] = {"wl"}; // river level or hydrometric level

    Utils utils;
    u8_t maintenanceTick = 0;
    int readWL();
    const size_t PARAM_LENGTH = sizeof(readParams) / sizeof(String);
    int VALUE_HOLD[sizeof(readParams) / sizeof(String)][Bootstrap::OVERFLOW_VAL];

    enum
    {
        // wl_pw
        hydrometric_level
        //wl_cm
    };

public:
    ~WlDevice();
    WlDevice(Bootstrap *boots);
    WlDevice(Bootstrap *boots, int sendIdentity);
    static char setDigital(bool value);
    static bool isDigital(char value);
    static void setPin(bool digital);
    void read();
    WLStruct getProm();
    String name();
    void loop();
    void clear();
    void print();
    void init();
    u8_t matenanceCount();
    u8_t paramCount();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif