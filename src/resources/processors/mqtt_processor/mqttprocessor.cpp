#include "mqttprocessor.h"
#include "certs.h"
#include "CellularHelper.h"

const char *MQTT_PING_TOPIC = "devices/ping";
const char *MQTT_CONFIG_TOPIC = "devices/config";
const char *MQTT_CONFIG_REGISTRATION = "devices/config/register";
const char *AWS_HOST = "a2hreerobwhgvz-ats.iot.ap-southeast-1.amazonaws.com";

const String MQTT_PUBLISH_TOPIC = "devices/publish";
const String MQTT_MAINTENANCE_TOPIC = "devices/maintenance";
const String MQTT_HEARTBEAT_TOPIC = "devices/heartbeat";

void mqttCallback(char *topic, byte *payload, unsigned int length);

//MQTT _client(strdup(AWS_HOST), MQTT_PORT, MQTT_KEEP_ALIVE, mqttCallback);
MQTT _client(strdup(AWS_HOST), MQTT_PORT, mqttCallback);

MqttProcessor::~MqttProcessor()
{
}

MqttProcessor::MqttProcessor(Bootstrap *boots)
{
    this->boots = boots;
    MqttProcessor();
}

MqttProcessor::MqttProcessor()
{
    this->deviceID = System.deviceID();
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

String MqttProcessor::getPublishTopic(bool maintenance)
{
    if (maintenance)
    {
        return MQTT_MAINTENANCE_TOPIC;
    }
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

bool MqttProcessor::isConnected()
{
    return _client.isConnected();
}

void MqttProcessor::pingPong()
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

void MqttProcessor::configProcessor(String data)
{
    JSONValue outerObj = JSONValue::parseCopy(data.c_str());
    // JSONArrayIterator iter(outerObj);
    JSONObjectIterator iter(outerObj);
    // for (size_t ii = 0; iter.next(); ii++)
    // {
    //     Log.info("THIS IS THE ARRAY %u  and value %s", ii, (const char *)iter.value().toString());
    //     JSONValue objectDetails = JSONValue::parseCopy((const char *)iter.value().toString());
    //     JSONObjectIterator deetsIter(outerObj);
    //     while (deetsIter.next())
    //     {
    //         Log.info("Boomo %s: Gazzomo %s", (const char *)deetsIter.name(), (const char *)deetsIter.value().toString());
    //     }
    // }
    String id;
    String entity;
    String value;
    while (iter.next())
    {
        String key = String((const char *)iter.name());
        if (key == "_id")
        {
            id = String((const char *)iter.value().toString().data());
        }
        else if (key == "entity")
        {
            entity = String((const char *)iter.value().toString().data());
        }
        else if (key == "value")
        {
            value = String((const char *)iter.value().toString().data());
        }
    }

    Log.info("Boomo %s: Gazzomo %s BABBOMO %s", id.c_str(), entity.c_str(), value.c_str());

    // char buf[200];
    // memset(buf, 0, sizeof(buf));
    // JSONBufferWriter writer(buf, sizeof(buf) - 1);
    // writer.beginObject();
    // writer.name("device").value(System.deviceID());
    // writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
    // writer.name("message").value("pong");
    // writer.endObject();
    // _client.publish(MQTT_CONFIG_TOPIC, String(buf));
}

void MqttProcessor::parseMessage(String data, char *topic)
{
    String dId = String(System.deviceID());
    String pingTopic = String(MQTT_PING_TOPIC) + "/" + dId;
    String configTopic = String(MQTT_CONFIG_TOPIC) + "/" + dId;

    String _topic = String(topic);

    if (_topic == pingTopic)
    {
        return MqttProcessor::pingPong();
    }
    else if (_topic == configTopic)
    {
        return MqttProcessor::configProcessor(data);
    }

    JSONValue outerObj = JSONValue::parseCopy(data.c_str());

    // outerObj.toString

    /*

    {
  "_id": 234,
  "values": [{
    "entity": "digital",
    "value": true
  }, {
    "entity": "publish",
    "value": 2
  }]
}

    */

    JSONObjectIterator iter(outerObj);
    while (iter.next())
    {
        Log.info("key=%s value=%s",
                 (const char *)iter.name(),
                 (const char *)iter.value().toString().data());

        // if (iter.name() == "values")
        // {
        //     Log.info("RUNINNG VALUES ");
        //     JSONValue arrayObj = JSONValue::parseCopy(iter.value().toString().data());
        //     JSONArrayIterator arrIter(arrayObj);
        //     for (size_t ii = 0; arrIter.next(); ii++)
        //     {
        //         Log.info("THIS IS THE ARRAY %u  and value %s", ii, (const char *)arrIter.value().toString());
        //         JSONValue objectDetails = JSONValue::parseCopy((const char *)arrIter.value().toString());
        //         JSONObjectIterator deetsIter(outerObj);
        //         while (deetsIter.next())
        //         {
        //             Log.info("Boomo %s: Gazzomo %s", (const char *)deetsIter.name(), (const char *)deetsIter.value().toString());
        //         }
        //     }
        // }
    }
}

void MqttProcessor::disconnectProtocol()
{
    Cellular.disconnect();
    lights.delayBlink(CONNECT_EVENT_HOLD_DELAY * 4, RgbController::PANIC);
    lights.control(false);
    connectEventFail = 0;
    connect();
}

void MqttProcessor::connectEvent()
{

    if (connectEventFail >= CONNECT_EVENT_THRESHOLD)
    {
        disconnectProtocol();
    }
    else
    {
        connectEventFail++;
        lights.delayBlink(CONNECT_EVENT_HOLD_DELAY, RgbController::THRASH);
        lights.control(false);
    }
}

void MqttProcessor::maintainContact()
{

    if (Cellular.connecting())
    {
        connectEvent();
    }
    else if (!serviceConnected && !Cellular.ready())
    {
        Log.info("Serice disconnected");
        disconnectProtocol();
    }
    else if (serviceConnected && !Cellular.ready())
    {
        serviceConnected = false;
    }
    else
    {
        if (Cellular.ready() && !serviceConnected)
        {
            serviceConnected = true;
        }
        if (_client.isConnected())
        {
            lights.breath();
        }
    }
}

void MqttProcessor::loop()
{
    //this->maintainContact();
    this->mqttLoop();
}

bool MqttProcessor::controlConnect()
{

    u8_t connAttempt = 0;
    // const u8_t MAX_ATTEMPTS = 10;
    bool connected = !Cellular.ready();

    while (!_client.isConnected() && Cellular.ready())
    {

        Log.info("Attempting MQTT connection...");
        // Attempt to connect
        String connectionID = this->deviceID + "-" + String(this->connect_id);
        //lights.control(false);
        if (_client.connect(connectionID))
        {
            // lights.breath();
            // lights.rapidFlash();
            connected = true;
            this->enableSubsrictions();
            _client.publish(MQTT_CONFIG_REGISTRATION, this->deviceID);
            Log.info("Connected");
        }
        else
        {
            // lights.panic();
            // lights.rapidFlash();
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
            //lights.delayBlink(CONNECT_EVENT_HOLD_DELAY, RgbController::THRASH);
            delay(5000);
        }
    }
    return connected;
}

void MqttProcessor::reboot()
{
    boots->resetBeachCount();
    System.reset();
}

void MqttProcessor::mqttConnect()
{
    if (!this->controlConnect() && Cellular.ready())
    {
        reboot();
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
            reboot();
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

    if (!this->mqttSubscribed && Cellular.ready())
    {

        this->ApplyMqttSubscription();
    }
    bool mConn = _client.isConnected();
    if (mConn)
    {

        _client.loop();
    }
    else if (this->mqttSubscribed && !mConn && Cellular.ready())
    {
        Log.info("Attempting to connect via MQTT");
        mqttConnect();
    }
}

bool MqttProcessor::hasHeartbeat()
{
    return this->HAS_HEARTBEAT;
}

int MqttProcessor::responseCallback(int type, const char *buf, int len, void *param)
{
    CellularHelperCommonResponse *presp = (CellularHelperCommonResponse *)param;

    return presp->parse(type, buf, len);
}

//firmware\hal\src\electron\modem\mdm_hal.cpp
// void MqttProcessor::ModemHardReset()
// {
//     Log.info("[ Modem  Hard reset ]");
//     const unsigned delay = 100;
//     HAL_GPIO_Write(RESET_UC, 0);
//     HAL_Delay_Milliseconds(delay);
//     HAL_GPIO_Write(RESET_UC, 1);
// }

void MqttProcessor::connect()
{
    delay(5000);
    if (boots->isBeached())
    {
        boots->beach();
    }
    //cellular_off(NULL);

    //cellular_on(NULL);
    // delay(5000);
    Log.info("ACTIVATING");
    // Cellular.off();

    //CellularHelperLocationResponse resp;

    // Cellular.command("AT+URAT=1,0\r\n");
    //delay(5000);
    // Cellular.connect();
    // Cellular.on();
    // // delay(15000);
    // // Cellular.off();
    // // delay(30000);
    // // Cellular.clearCredentials();
    // // Cellular.setCredentials(APN);
    // //  ModemHardReset();
    // // Log.info("TURNING ON CELULAR ");ModemHardReset();
    // // Cellular.on();
    // // // resp.resp = Cellular.command(responseCallback, (void *)&resp, 10000, "AT\r\n", 10000 / 1000);
    // // // Log.info("TURNING ON AT SHIT %d ", resp.resp);
    // // // delay(5000);]
    // // // Cellular.disconnect();
    // delay(5000);
    // Cellular.clearCredentials();
    // Cellular.setCredentials(APN);
    // // // Cellular.off();
    // // // delay(5000);
    // // // Cellular.clearCredentials();
    // // // // // // // // Connects to a cellular network by APN only
    // // // Cellular.setCredentials(APN);
    // // // Cellular.on();
    // delay(5000);
    // // // for (size_t i = 0; i < 50; i++)
    // // // {

    // // //     if (RESP_OK == Cellular.command(30000, "AT\r\n"))
    // // //     {
    // // //         Log.log("JUIST BOOMO ME SOME AT");
    // // //         break;
    // // //     }
    // // //     else
    // // //     {
    // // //         Log.log("SHIT AINT AT ALL BOOMO %u", i);
    // // //     }

    // // //     delay(2000);
    // // // }
    // // // delay(10000);
    // // // resp.resp = Cellular.command(responseCallback, (void *)&resp, 10000, "AT+CFUN=0\r\n", 10000 / 1000);
    // // // Log.info("TURNING ON CELULAR %d %llX ", resp.resp, RESP_OK);
    // // // delay(5000);
    // Cellular.connect();

    // // // delay(MODEM_ON_WAIT_TIME_MS);
    // // // // Cellular.setActiveSim(EXTERNAL_SIM);
    // Cellular.setActiveSim(EXTERNAL_SIM);
    // Cellular.connect();

    // // // waitFor(Cellular.ready);
    // waitFor(Cellular.ready, 20000);

    if (Cellular.ready())
    {
        ApplyMqttSubscription();
        waitFor(_client.isConnected, 20000);
    }

    // CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
    // int bars = CellularHelperClass::rssiToBars(rssiQual.rssi);

    // Log.info("rssi=%d, qual=%d, bars=%d", rssiQual.rssi, rssiQual.qual, bars);

    // CellularHelperEnvironmentResponseStatic<8> envResp;

    // CellularHelper.getEnvironment(CellularHelper.ENVIRONMENT_SERVING_CELL_AND_NEIGHBORS, envResp);
    // if (envResp.resp != RESP_OK)
    // {
    //     // We couldn't get neighboring cells, so try just the receiving cell
    //     CellularHelper.getEnvironment(CellularHelper.ENVIRONMENT_SERVING_CELL, envResp);
    // }
    // envResp.logResponse();

    // CellularHelperLocationResponse loc = CellularHelper.getLocation();

    // Log.info("WE ARE HERE BIITCHES, %s", loc.toString().c_str());
    // // waitFor(Cellular.ready, 10000);
    // CellularGlobalIdentity cgi = {0};
    // cgi.size = sizeof(CellularGlobalIdentity);
    // cgi.version = CGI_VERSION_LATEST;

    // cellular_result_t res = cellular_global_identity(&cgi, NULL);
    // if (res == SYSTEM_ERROR_NONE)
    // {
    //     Log.info("cid=%d lac=%u mcc=%d mnc=%d", cgi.cell_id, cgi.location_area_code, cgi.mobile_country_code, cgi.mobile_network_code);
    // }
    // else
    // {
    //     Log.info("cellular_global_identity failed %d", res);
    // }
}
