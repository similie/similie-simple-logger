#include "flow-meter.h"

FlowMeter::~FlowMeter()
{
}

FlowMeter::FlowMeter()
{
}

FlowMeter::FlowMeter(Bootstrap *boots)
{
    this->boots = boots;
}

String FlowMeter::name()
{
    return this->deviceName;
}

void FlowMeter::restoreDefaults()
{
}

void FlowMeter::init()
{
    boots->startSerial();
}

void FlowMeter::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
}

void FlowMeter::parseSerial(String ourReading)
{
    Serial.println(ourReading);
}

void FlowMeter::read()
{
    Serial1.println("$GPS_1");
}

void FlowMeter::loop()
{
    String completedSerialItem = boots->fetchSerial("$GPS_1");
    if (!completedSerialItem.equals(""))
    {
        parseSerial(completedSerialItem);
    }
}

void FlowMeter::clear()
{
}

void FlowMeter::print()
{
}

size_t FlowMeter::buffSize()
{
    return 150;
}

uint8_t FlowMeter::paramCount()
{
    return 4;
}

uint8_t FlowMeter::matenanceCount()
{
    return 0;
}
