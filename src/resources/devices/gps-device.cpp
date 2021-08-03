#include "gps-device.h"

GpsDevice::~GpsDevice()
{
}

GpsDevice::GpsDevice()
{
}

GpsDevice::GpsDevice(Bootstrap *boots)
{
    this->boots = boots;
}

String GpsDevice::name() 
{
    return this->deviceName;
}

void GpsDevice::restoreDefaults()
{

}

void GpsDevice::init()
{
    boots->startSerial();
}
 
void GpsDevice::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
  
}

void GpsDevice::read()
{
    
}

void GpsDevice::loop()
{
}

void GpsDevice::clear()
{
    
}

void GpsDevice::print()
{
   
}

size_t GpsDevice::buffSize()
{
    return 150;
}

u8_t GpsDevice::paramCount()
{
    return 4;
}

u8_t GpsDevice::matenanceCount()
{
    return 0;
}
