#include "rain-gauge.h"

RainGauge::~RainGauge()
{
}

RainGauge::RainGauge()
{
}

RainGauge::RainGauge(Bootstrap *boots)
{
    this->boots = boots;
}

String RainGauge::name() 
{
    return this->deviceName;
}

void RainGauge::restoreDefaults()
{

}

void RainGauge::countChange()
{
    Utils::log("RAIN_GAUGE_INTERRUPT", "GOT TAPPED");
}

void boomo()
{
 Utils::log("RAIN_GAUGE_INTERRUPT", "GOT TAPPED");
}

void RainGauge::setInterrupt()
{
    //attachInterrupt(RAIN_GAUGE_PIN, &RainGauge::countChange, this, CHANGE);
    attachInterrupt(RAIN_GAUGE_PIN, boomo, CHANGE);
}

void RainGauge::setPin()
{
    pinMode(RAIN_GAUGE_PIN, INPUT_PULLDOWN);
}

void RainGauge::init()
{
   setPin();
   setInterrupt();
}
 
void RainGauge::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
  
}

void RainGauge::read()
{
    
}

void RainGauge::loop()
{
}

void RainGauge::clear()
{
    
}

void RainGauge::print()
{
   
}

size_t RainGauge::buffSize()
{
    return 70;
}

u8_t RainGauge::paramCount()
{
    return 1;
}

u8_t RainGauge::matenanceCount()
{
    return 0;
}
