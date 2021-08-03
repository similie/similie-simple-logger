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
    ~Device();
    Device();
    Device(Bootstrap *boots);
    virtual void nullifyPayload(const char *key);
    //virtual void storePayload(String payload, String topic);
    virtual String name();
    virtual void read();
    virtual void loop();
    virtual u8_t matenanceCount();
    virtual u8_t paramCount();
    virtual void clear();
    virtual void print();
    virtual size_t buffSize();
    virtual void init();
    virtual void restoreDefaults();
    virtual void publish(JSONBufferWriter &writer, u8_t attempt_count);
   // virtual void popOfflineCollection(Processor *processor, String topic, u8_t count);
};

#endif