#include "Particle.h"
#include "string.h"
#include "device.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"
#ifndef battery_h
#define battery_h

class Battery : public Device
{
private:
    Bootstrap *boots;
    const char *percentname = "bat_percent";
    const char *voltsname = "battery_voltage";

public:
    ~Battery();
    Battery();
    Battery(Bootstrap *boots);
    void read();
    void loop();
    void clear();
    void print();
    void init();
    size_t buffSize();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif