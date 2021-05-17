#include "controlled_payload.h"

bool ControlledPayload::isExpired(ControlledPayload *payload)
{

    if (payload->timestamp == NULL || payload->timestamp == 0)
    {
        return true;
    }

    unsigned long now = millis();
    unsigned long delta = now - payload->timestamp;
    return delta >= ControlledPayload::EXPIRATION_TIME;
}

ControlledPayload::ControlledPayload(unsigned int target, String payload)
{

    this->timestamp = millis();
    this->target = target;
    this->payload = payload;
}

unsigned int ControlledPayload::getTarget()
{
    return this->target;
}

bool ControlledPayload::onTarget(int target)
{
    return target == (int)this->target;
}

ControlledPayload::~ControlledPayload()
{
}

String ControlledPayload::getHoldings()
{
    return this->payload;
}

unsigned long ControlledPayload::getTimeStamp()
{
    return this->timestamp;
}