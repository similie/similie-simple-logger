#include "Particle.h"
#include "string.h"

#ifndef processor_h
#define processor_h

class Processor
{
private:
public:
    ~Processor();
    Processor();
    static void parseMessage(String data, char *topic);
    virtual String getHeartbeatTopic();
    virtual String getPublishTopic();
    static bool connected();
    virtual void publish(String topic, String payload);
    virtual void loop();
};

#endif