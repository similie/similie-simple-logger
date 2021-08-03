#include "Particle.h"
#include "string.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#ifndef gps_device_h
#define gps_device_h

class RainGauge
{
private:
    Bootstrap *boots;
     String valueMap[2] =
        {
            "pre"};
    enum
    {
        precipitation
    };
    String deviceName = "RainGauge";
public:
    ~RainGauge();
    RainGauge();
    RainGauge(Bootstrap *boots);
    void nullifyPayload(const char *key);
    String name();
    //void storePayload(String payload, String topic);
    void read();
    void loop();
    u8_t matenanceCount();
    u8_t paramCount();
    void clear();
    void print();
    size_t buffSize();
    void init();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
   // void popOfflineCollection(Processor *processor, String topic, u8_t count);
};

#endif