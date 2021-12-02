#include "Particle.h"
#include "string.h"
#include "resources/utils/utils.h"
#ifndef processor_h
#define processor_h

class Processor
{
private:
    const bool HAS_HEARTBEAT = true;
    /**
     * We send to different events to load balanance
     */
    // const char *SEND_EVENT_NAME = "Al/Post/Red";
    // const char *SEND_EVENT_NAME = "Al/Post/White";
    const char *SEND_EVENT_NAME = "Al/Post/Blue";
    // const char *SEND_EVENT_NAME = "Al/Post/Black";
    const char *SEND_EVENT_MAINTENANCE = "Al/Post/Maintain";
    const char *SEND_EVENT_HEARTBEAT = "Al/Post/Heartbeat";
    void manageManualModel();
    const bool MANUAL_MODE = false;

public:
    ~Processor();
    Processor();
    Utils utils;
    virtual bool hasHeartbeat();
    virtual const char *primaryPostName();
    static void parseMessage(String data, char *topic);
    virtual String getHeartbeatTopic();
    virtual String getPublishTopic(bool maintenance);
    static bool isConnected();
    virtual bool connected();
    virtual bool publish(String topic, String payload);
    virtual void loop();
    virtual void connect();
};

#endif