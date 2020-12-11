#include "Particle.h"
#include "string.h"
#include "device.h"
#include "resources/bootstrap/bootstrap.h"
#include <Adafruit_GPS.h>

#define GPSECHO true

#ifndef serial_gps_h
#define serial_gps_h

#define GPSSerial Serial1

class SerialGps : public Device
{
private:
    Bootstrap *boots;

public:
    ~SerialGps();
    SerialGps();
    SerialGps(Bootstrap *boots);
    void read();
    void loop();
    u8_t matenanceCount();
    u8_t paramCount();
    void clear();
    void print();
    size_t buffSize();
    void init();
    void publish(JSONBufferWriter &writer, u8_t attempt_count);
};

#endif