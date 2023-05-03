#include "Processor.h"

/**
 *
 * The processor class can inherited for producing additional means of sending data payloads such as MQTT.
 * This default class uses particle's api for sending data. Future classes will use other protocols
 *
 */

/**
 * @deconstuctor
 */
Processor::~Processor()
{
}

/**
 * Default Constructor
 */
Processor::Processor()
{
}

/**
 * @public
 *
 * getHeartbeatTopic
 *
 * What's the hearbeat topic name we send
 *
 * @return String
 */
String Processor::getHeartbeatTopic()
{
    return String(SEND_EVENT_HEARTBEAT);
}

/**
 * @public
 *
 * getPublishTopic
 *
 * What's the publish topic name we send
 *
 * @return String
 */
String Processor::getPublishTopic(bool maintenance)
{
    if (maintenance)
    {
        return String(this->SEND_EVENT_MAINTENANCE);
    }

    return String(this->SEND_EVENT_NAME);
}

/**
 * @public
 *
 * publish
 *
 * Sends the payload over the network
 *
 * @param topic - the topic name
 * @param payload - the actual stringified data
 *
 * @return bool - true if successfull
 */
bool Processor::publish(String topic, String payload)
{
    bool success = Particle.publish(topic, payload);
    return success;
}

/**
 * @public
 *
 * connected
 *
 * Is the device connected to the network it sends to
 *
 * @return bool - true if connected
 */
bool Processor::connected()
{
    return Particle.connected();
}

/**
 * @public
 *
 * isConnected
 *
 * Is the device connected to the network it sends to.
 * Same as above. For API compatibiity
 *
 * @return bool - true if connected
 */
bool Processor::isConnected()
{
    return Particle.connected();
}

/**
 * @public
 *
 * parseMessage
 *
 * Parses a given message based on a message topic
 *
 * @param String data - the message string
 * @param char *topic - the topic that was sent
 *
 * @return void
 */
void Processor::parseMessage(String data, char *topic)
{
    JSONValue outerObj = JSONValue::parseCopy(data.c_str());
    JSONObjectIterator iter(outerObj);
    while (iter.next())
    {
        Log.info("key=%s value=%s",
                 (const char *)iter.name(),
                 (const char *)iter.value().toString());
    }
}

/**
 * @public
 *
 * loop
 *
 * Loops with the main loop. Some processors require
 * a constant checkin, including particle in manual mode
 *
 * @return void
 */
void Processor::loop()
{

    if (!MANUAL_MODE)
    {
        return;
    }

    manageManualModel();
}

/**
 * @public
 *
 * hasHeartbeat
 *
 * Is the heartbeat event ready
 *
 * @return bool
 */
bool Processor::hasHeartbeat()
{
    return this->HAS_HEARTBEAT;
}

/**
 * @public
 *
 * primaryPostName
 *
 * Sends the the primary send event name
 *
 * @return bool
 */
const char *Processor::primaryPostName()
{
    return this->SEND_EVENT_NAME;
}

/**
 * @public
 *
 * connect
 *
 * Connect to the send network
 *
 * @return void
 */
void Processor::connect()
{
    if (!MANUAL_MODE)
    {
        return;
    }

    Particle.connect();
}

/**
 * @private
 *
 * manageManualModel
 *
 * Manages the manual mode if required in the loop
 *
 * @return void
 */
/* IF IN MANUAL MODE */
void Processor::manageManualModel()
{
    if (waitFor(Particle.connected, 500))
    {
        Particle.process();
        return;
    }

    Particle.connect();
}
