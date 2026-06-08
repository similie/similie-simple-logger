#include "Particle.h"
#include "string.h"
#include "device.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#ifndef gps_device_h
#define gps_device_h

class GpsDevice : public Device
{
private:
    Bootstrap *boots;
    String deviceName = "GPSDevice";
    void parseSerial(String ourReading);

public:
    ~GpsDevice();
    GpsDevice();
    GpsDevice(Bootstrap *boots);
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
};

#endif