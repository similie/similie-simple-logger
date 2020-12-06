#include "mqttprocessor.h"
#include "certs.h"

const char *MQTT_PING_TOPIC = "devices/ping";
const char *MQTT_CONFIG_TOPIC = "devices/config";
const char *MQTT_CONFIG_REGISTRATION = "devices/config/register";
const char *AWS_HOST = "a2hreerobwhgvz-ats.iot.ap-southeast-1.amazonaws.com";

const String MQTT_PUBLISH_TOPIC = "devices/publish";
const String MQTT_HEARTBEAT_TOPIC = "devices/heartbeat";

void mqttCallback(char *topic, byte *payload, unsigned int length);

MQTT _client(strdup(AWS_HOST), MQTT_PORT, MQTT_KEEP_ALIVE, mqttCallback);

MqttProcessor::~MqttProcessor()
{
}

MqttProcessor::MqttProcessor(String deviceID)
{
    this->deviceID = deviceID;
    this->configTopic = String(MQTT_CONFIG_TOPIC) + "/" + String(this->deviceID);
    this->pingTopic = String(MQTT_PING_TOPIC) + "/" + String(this->deviceID);
    String listeners[] = {this->configTopic, this->pingTopic, "devices/state-change"};
    const size_t length = sizeof(listeners) / sizeof(String);
    for (size_t i = 0; i < length; i++)
    {
        this->mqttListeners[i] = listeners[i];
    }
}

String MqttProcessor::getHeartbeatTopic()
{
    return MQTT_HEARTBEAT_TOPIC;
}

String MqttProcessor::getPublishTopic()
{
    return MQTT_PUBLISH_TOPIC;
}

void MqttProcessor::publish(String topic, String payload)
{
    _client.publish(topic, payload);
}

bool MqttProcessor::connected()
{
    return _client.isConnected();
}

static void pingPong()
{
    char buf[200];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    writer.name("device").value(System.deviceID());
    writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
    writer.name("message").value("pong");
    writer.endObject();
    _client.publish(MQTT_PING_TOPIC, String(buf));
}

void MqttProcessor::parseMessage(String data, char *topic)
{
    String pingTopic = String(MQTT_PING_TOPIC) + "/" + String(System.deviceID());

    if (String(topic) == pingTopic)
    {
        return pingPong();
    }

    JSONValue outerObj = JSONValue::parseCopy(data.c_str());

    JSONObjectIterator iter(outerObj);
    while (iter.next())
    {
        Log.info("key=%s value=%s",
                 (const char *)iter.name(),
                 (const char *)iter.value().toString());
    }
}

void MqttProcessor::loop()
{
    this->mqttLoop();
}

bool MqttProcessor::controlConnect()
{

    u8_t connAttempt = 0;
    // const u8_t MAX_ATTEMPTS = 10;
    bool connected = !Particle.connected();

    while (!_client.isConnected() && Particle.connected())
    {
        Log.info("Attempting MQTT connection...");
        // Attempt to connect
        String connectionID = this->deviceID + "-" + String(this->connect_id);

        if (_client.connect(connectionID))
        {
            connected = true;
            this->enableSubsrictions();
            _client.publish(MQTT_CONFIG_REGISTRATION, this->deviceID);
            Log.info("connected");
        }
        else
        {
            connAttempt++;
            this->connect_id++;
            Log.info("Disconnect %d ", this->connect_id);
            _client.disconnect();
            Log.info("TLS ");
            this->enableMQTTTls();
            Log.info("AFTER TLS ");
            if (connAttempt >= REBOOT_THRESHOLD_VALUE)
            {
                connected = false;
                break;
            }
            Log.info("Attempt %d failed, try again in 5 seconds ", connAttempt);
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    return connected;
}

void MqttProcessor::mqttConnect()
{
    if (!this->controlConnect() && Particle.connected())
    {
        // rebootRequest(String("$"));
        System.reset();
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    Log.info("MQTT PAYLOAD %s", message.c_str());

    MqttProcessor::parseMessage(message, topic);
}

int MqttProcessor::enableMQTTTls()
{
    const char amazonIoTRootCaPem[] = AMAZON_IOT_ROOT_CA_PEM;
    const char clientKeyCrtPem[] = CELINT_KEY_CRT_PEM;
    const char clientKeyPem[] = CELINT_KEY_PEM;
    // Serial.println(amazonIoTRootCaPem);
    // enable tls. set Root CA pem, private key file.
    int ret;
    if ((ret = _client.enableTls(amazonIoTRootCaPem, sizeof(amazonIoTRootCaPem),
                                 clientKeyCrtPem, sizeof(clientKeyCrtPem),
                                 clientKeyPem, sizeof(clientKeyPem)) < 0))
    {
        Log.info("_client.enableTls failed with code: %d", ret);
        if (this->sslFailureCount < REBOOT_THRESHOLD_VALUE)
        {
            this->sslFailureCount++;
            return this->enableMQTTTls();
        }
        else
        {
            System.reset();
        }
    }
    else
    {
        this->sslFailureCount = 0;
    }

    return ret;
}

void MqttProcessor::enableSubsrictions()
{
    size_t length = sizeof(this->mqttListeners) / sizeof(String);
    for (size_t i = 0; i < length; i++)
    {
        String topic = this->mqttListeners[i];
        _client.subscribe(topic.c_str());
    }
}

void MqttProcessor::ApplyMqttSubscription()
{
    Log.info("Attempting MQTT Connection");
    int tls = this->enableMQTTTls();
    Serial.printlnf("_client.enableTls got code: %d", tls);

    this->mqttConnect();
    // publish/subscribe
    Log.info("MQTT Connected? %d", _client.isConnected());
    if (_client.isConnected())
    {
        Log.info("client connected");
        this->mqttSubscribed = true;
    }
}

void MqttProcessor::mqttLoop()
{

    if (!this->mqttSubscribed && Particle.connected())
    {
        this->ApplyMqttSubscription();
    }
    bool mConn = _client.isConnected();
    if (mConn)
    {
        _client.loop();
    }
    else if (this->mqttSubscribed && !mConn && Particle.connected())
    {
        Log.info("Attempting to connect via MQTT");
        mqttConnect();
    }
}
