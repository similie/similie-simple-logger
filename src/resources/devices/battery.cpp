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
    return 100;
}
