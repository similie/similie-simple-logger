#include "Particle.h"
#include "string.h"

#ifndef processor_h
#define processor_h

class Processor
{
private:
    const bool HAS_HEARTBEAT = false;
    const char *SEND_EVENT_NAME = "Al/Post/Raw";
    const char *SEND_EVENT_MAINTENANCE = "Al/Post/Maintan";

public:
    ~Processor();
    Processor();

    virtual bool hasHeartbeat();
    virtual const char *primaryPostName();
    static void parseMessage(String data, char *topic);
    virtual String getHeartbeatTopic();
    virtual String getPublishTopic(bool maintenance);
    static bool isConnected();
    virtual bool connected();
    virtual void publish(String topic, String payload);
    virtual void loop();
};

#endif