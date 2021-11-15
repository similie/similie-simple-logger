#include "Particle.h"
#include "string.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#ifndef device_h
#define device_h

class Device
{
private:
    Bootstrap *boots;

public:
    virtual ~Device();
    Device();
    Device(Bootstrap *boots);
    virtual String name();
    virtual void read();
    virtual void loop();
    virtual uint8_t matenanceCount();
    virtual uint8_t paramCount();
    virtual void clear();
    virtual void print();
    virtual size_t buffSize();
    virtual void init();
    virtual void restoreDefaults();
    virtual void publish(JSONBufferWriter &writer, uint8_t attempt_count);
};

#endif