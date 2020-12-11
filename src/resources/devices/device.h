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
    virtual u8_t matenanceCount();
    virtual u8_t paramCount();
    virtual void clear();
    virtual void print();
    virtual size_t buffSize();
    virtual void init();
    virtual void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif