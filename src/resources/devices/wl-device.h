#include "Particle.h"
#include "string.h"
#include "device.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"
#include "math.h"

#ifndef wl_device_h
#define wl_device_h

#define DIG_PIN D8
#define AN_PIN A3

const size_t WL_PARAM_SIZE = 1;

class WlDevice : public Device
{
private:
    Bootstrap *boots;
    String readParams[WL_PARAM_SIZE] = {"hydrometric_level"};
    Utils utils;
    int readWL();
    static const size_t PARAM_LENGTH = sizeof(readParams) / sizeof(String);
    int VALUE_HOLD[WlDevice::PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];

    enum
    {
        hydrometric_level,
        wl_cm
    };

public:
    ~WlDevice();
    WlDevice();
    WlDevice(Bootstrap *boots);
    void read();
    void loop();
    void clear();
    void print();
    void init();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif