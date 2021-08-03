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

void RainGauge::init()
{
   
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
