#include "device.h"

Device::~Device()
{
}
Device::Device(Bootstrap *boots)
{
}

Device::Device()
{
}

void Device::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
}

u8_t Device::paramCount()
{
    return 0;
}

u8_t Device::matenanceCount()
{
    return 0;
}

void Device::read()
{
}

void Device::loop()
{
}

void Device::clear()
{
}

void Device::print()
{
}

void Device::init()
{
}

size_t Device::buffSize()
{
    return 300;
}