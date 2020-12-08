#include "Particle.h"
#include "string.h"
#include "MQTT-TLS.h"
#include "resources/processors/processor.h"

#define MQTT_PORT 8883
#define MQTT_KEEP_ALIVE 30
#define REBOOT_THRESHOLD_VALUE 4

#ifndef MqttProcessor_h
#define MqttProcessor_h

class MqttProcessor : public Processor
{
private:
    const char *mqttHost;
    String deviceID;
    String pingTopic;
    String configTopic;
    String mqttListeners[3];
    const bool HAS_HEARTBEAT = true;
    bool mqttSubscribed = false;
    bool controlConnect();
    int enableMQTTTls();
    unsigned int connect_id;
    void enableSubsrictions();
    void ApplyMqttSubscription();
    void mqttLoop();
    static void pingPong();
    static void configProcessor(String data);
    size_t sslFailureCount = 0;

public:
    ~MqttProcessor();
    MqttProcessor();
    static void parseMessage(String data, char *topic);
    String getHeartbeatTopic();
    String getPublishTopic();
    // static void mqttCallback(char *topic, byte *payload, unsigned int length);
    bool hasHeartbeat();
    void mqttConnect();
    static bool isConnected();
    bool connected();
    void publish(String topic, String payload);
    void loop();
};

#endif