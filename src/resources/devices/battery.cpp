#include "battery.h"

Battery::~Battery()
{
}
Battery::Battery(Bootstrap *boots)
{
}

Battery::Battery()
{
}

String Battery::name()  
{
    return deviceName;
}

void Battery::restoreDefaults()
{
}

void Battery::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
    FuelGauge fuel;
    writer.name(percentname).value(round(System.batteryCharge()));
    writer.name(voltsname).value(fuel.getVCell());
}

void Battery::read()
{
}

void Battery::loop()
{
}

void Battery::clear()
{
}

void Battery::print()
{
    Log.info("BATTERY POWER %.2f", System.batteryCharge());
}

void Battery::init()
{
}

size_t Battery::buffSize()
{
    return 80;
}

u8_t Battery::paramCount()
{
    return PARAM_LENGTH;
}

u8_t Battery::matenanceCount()
{
    return 0;
}
