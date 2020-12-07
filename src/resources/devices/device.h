#include "Particle.h"
#include "string.h"
#include "resources/bootstrap/bootstrap.h"
#ifndef device_h
#define device_h

class Device
{
private:
    Bootstrap *boots;

public:
    ~Device();
    Device();
    Device(Bootstrap *boots);
    virtual void read();
    virtual void loop();
    virtual void clear();
    virtual void print();
    virtual void init();
    virtual void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif