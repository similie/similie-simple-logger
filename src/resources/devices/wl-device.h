#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"
#include "math.h"

#ifndef wl_device_h
#define wl_device_h

#define DIG_PIN D8  // stripe blue off port1
#define AN_PIN A1 // stripe blue line off port0

const size_t WL_PARAM_SIZE = 1;

class WlDevice : public Device
{
private:
    Bootstrap *boots;
    String deviceName = "wl";
    int sendIdentity = -1;
    //String readParams[WL_PARAM_SIZE] = {"wl_pw", "hydrometric_level"};
    // String readParams[WL_PARAM_SIZE] = {"wl_pw"}; // water tank  or wl
    String readParams[WL_PARAM_SIZE] = {"hydrometric_level"}; // river level or hydrometric level

    Utils utils;
    u8_t maintenanceTick = 0;
    int readWL();
    static const size_t PARAM_LENGTH = sizeof(readParams) / sizeof(String);
    int VALUE_HOLD[WlDevice::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];

    enum
    {
        // wl_pw
        hydrometric_level
        //wl_cm
    };

public:
    ~WlDevice();
    WlDevice();
    WlDevice(Bootstrap *boots);
    WlDevice(Bootstrap *boots, int sendIdentity);
    void read();
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