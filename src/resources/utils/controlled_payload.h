#include "Particle.h"

#ifndef controlled_payload_h
#define controlled_payload_h

class ControlledPayload
{
private:
    unsigned int target = 0;
    String payload;
    unsigned long timestamp = 0;

public:
    ~ControlledPayload();
    ControlledPayload();
    ControlledPayload(unsigned int target, String payload);
    const static unsigned long EXPIRATION_TIME = 180000;
    static bool isExpired(ControlledPayload *payload);
    String getHoldings();
    bool onTarget(int target);
    unsigned int getTarget();
    unsigned long getTimeStamp();
};

#endif