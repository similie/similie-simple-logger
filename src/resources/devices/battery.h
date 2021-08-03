#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#include <stdint.h>
#ifndef battery_h
#define battery_h

class Battery : public Device
{
private:
    Bootstrap *boots;
    const char *percentname = "bat";
    const char *voltsname = "b_v";
    const u8_t PARAM_LENGTH = 2;
    String deviceName = "Battery";
public:
    ~Battery();
    Battery();
    Battery(Bootstrap *boots);
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
};

#endif