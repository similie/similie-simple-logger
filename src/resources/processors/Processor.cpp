#include "processor.h"

Processor::~Processor()
{
}

Processor::Processor()
{
}

String Processor::getHeartbeatTopic()
{
    return String("");
}

String Processor::getPublishTopic()
{
    return String("");
}

void Processor::publish(String topic, String payload)
{
    Particle.publish(topic, payload);
}

bool Processor::connected()
{
    return Particle.connected();
}

void Processor::parseMessage(String data, char *topic)
{

    JSONValue outerObj = JSONValue::parseCopy(data.c_str());

    JSONObjectIterator iter(outerObj);
    while (iter.next())
    {
        Log.info("key=%s value=%s",
                 (const char *)iter.name(),
                 (const char *)iter.value().toString());
    }
}

void Processor::loop()
{
}
