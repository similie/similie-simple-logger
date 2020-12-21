#include "Particle.h"
#include "string.h"
#include "MQTT-TLS.h"
#include "resources/processors/processor.h"
#include "resources/utils/rgb_controller.h"
#include "resources/bootstrap/bootstrap.h"

#define MQTT_PORT 8883
#define MQTT_KEEP_ALIVE 30
#define REBOOT_THRESHOLD_VALUE 4

#ifndef MqttProcessor_h
#define MqttProcessor_h

class MqttProcessor : public Processor
{
private:
    const char *mqttHost;
    const char *APN = "internet";
    bool serviceConnected = false;
    const unsigned long MODEM_ON_WAIT_TIME_MS = 4000;
    Bootstrap *boots;
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
    RgbController lights;
    void maintainContact();
    static int responseCallback(int type, const char *buf, int len, void *param);
    void connectEvent();
    void disconnectProtocol();
    size_t connectEventFail = 0;
    const size_t CONNECT_EVENT_THRESHOLD = 12;
    const u8_t CONNECT_EVENT_HOLD_DELAY = 5;
    void reboot();

public:
    ~MqttProcessor();
    MqttProcessor();
    MqttProcessor(Bootstrap *boots);

    static void parseMessage(String data, char *topic);
    String getHeartbeatTopic();
    String getPublishTopic(bool maintenance);
    // static void mqttCallback(char *topic, byte *payload, unsigned int length);
    bool hasHeartbeat();
    void mqttConnect();
    static bool isConnected();
    bool connected();
    void publish(String topic, String payload);
    void loop();
    void connect();
    void ModemHardReset();
};

#endif