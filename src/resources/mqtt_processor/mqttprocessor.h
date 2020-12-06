#include "Particle.h"
#include "string.h"
#include "MQTT-TLS.h"

#define MQTT_PORT 8883
#define MQTT_KEEP_ALIVE 120
#define REBOOT_THRESHOLD_VALUE 4

#ifndef MqttProcessor_h
#define MqttProcessor_h

// #define MQTT_PUBLISH_TOPIC "devices/publish";
// #define MQTT_PING_TOPIC "devices/ping";
// #define MQTT_HEARTBEAT_TOPIC "devices/heartbeat";
// #define MQTT_CONFIG_TOPIC "devices/config";
// #define MQTT_CONFIG_REGISTRATION = "devices/config/register";

class MqttProcessor
{
private:
    const char *mqttHost;
    String deviceID;
    String pingTopic;
    String configTopic;
    String mqttListeners[3];
    bool mqttSubscribed = false;
    bool controlConnect();
    int enableMQTTTls();
    unsigned int connect_id;
    void enableSubsrictions();
    void ApplyMqttSubscription();
    void mqttLoop();
    size_t sslFailureCount = 0;

public:
    ~MqttProcessor();
    MqttProcessor(String deviceID);
    static void parseMessage(String data, char *topic);
    String getHeartbeatTopic();
    String getPublishTopic();
    // static void mqttCallback(char *topic, byte *payload, unsigned int length);
    void mqttConnect();
    bool connected();
    void publish(String topic, String payload);
    void loop();
};

#endif