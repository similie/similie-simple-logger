#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#ifndef flow_meter_h
#define flow_meter_h

class FlowMeter : public Device
{
private:
    Bootstrap *boots;
    String deviceName = "FlowMeter";
    void parseSerial(String ourReading);

public:
    ~FlowMeter();
    FlowMeter();
    FlowMeter(Bootstrap *boots);
    String name();
    void read();
    void loop();
    uint8_t matenanceCount();
    uint8_t paramCount();
    void clear();
    void print();
    size_t buffSize();
    void init();
    void restoreDefaults();
    void publish(JSONBufferWriter &writer, uint8_t attempt_count);
};

#endif