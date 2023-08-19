#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"

#ifndef relay_h
#define relay_h
#define DEFAULT_RELAY_PIN D7
class Relay : Device
{
private:
    Bootstrap *boots;
    int pin = DEFAULT_RELAY_PIN;
    bool invert = false;
    u8_t POW_ON = invert ? LOW : HIGH;
    u8_t POW_OFF = invert ? HIGH : LOW;
    void buildOutputs();

public:
    ~Relay();
    Relay();
    Relay(Bootstrap *boots);
    Relay(int pin);
    Relay(int pin, bool invert);
    String name();
    void read();
    void loop();
    uint8_t maintenanceCount();
    uint8_t paramCount();
    void clear();
    void print();
    size_t buffSize();
    void init();
    void restoreDefaults();

    void publish(JSONBufferWriter &writer, uint8_t attempt_count);
    bool on();
    bool off();
};

#endif