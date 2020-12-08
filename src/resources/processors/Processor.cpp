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
    return String(this->SEND_EVENT_NAME);
}

void Processor::publish(String topic, String payload)
{
    Particle.publish(topic, payload);
}

bool Processor::connected()
{
    return Particle.connected();
}

bool Processor::isConnected()
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

bool Processor::hasHeartbeat()
{
    return this->HAS_HEARTBEAT;
}

const char *Processor::primaryPostName()
{
    return this->SEND_EVENT_NAME;
}
