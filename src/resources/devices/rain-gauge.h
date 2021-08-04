#include "Particle.h"
#include "string.h"
#include <stdint.h>
#include "device.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/utils/utils.h"

// #define RAIN_GAUGE_PIN D7 // Blue when using Port1
#define RAIN_GAUGE_PIN A1 // Stripe Blue when using Port1

#ifndef rain_gauge_h
#define rain_gauge_h

class RainGauge : public Device
{
private:
    double perTipMultiple = 0.2; // mm
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
    String name();
    void setPin();
    void countChange(void);
    void setInterrupt();
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
};

#endif