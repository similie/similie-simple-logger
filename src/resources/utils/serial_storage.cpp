#include "serial_storage.h"

static bool sendingOffline = false;

/**
 * @constructor
 * 
 * @param Processor * holdProcessor - the processor for sending a payload
 * @param Bootstrap * boots - the bootstrap object
 */
SerialStorage::SerialStorage(Processor *holdProcessor,  Bootstrap *boots)
{
    this->holdProcessor = holdProcessor;
    this->boots = boots;
}

/** 
 * @deconstructor
*/
SerialStorage::~SerialStorage() 
{

}

/** 
 * @public 
 * 
 * clearDeviceStorage
 * 
 * Static function for clearing EEPROM
 * 
 * @return void
 * 
*/
void SerialStorage::clearDeviceStorage() 
{
    delay(10000);
    EEPROM.clear();
    Utils::log("EPROM_CLEARED", String(millis()));
}

/** 
 * @public 
 * 
 * notSendingOfflineData
 * 
 * Static function if the system is busy
 * 
 * @return bool
 * 
*/
bool SerialStorage::notSendingOfflineData() {
    return !sendingOffline;
}

/** 
 * @private 
 * 
 * payloadRestorator
 * 
 * Preps a payload fresh off the serial bus for being sent
 * 
 * @param String payload 
 * 
 * @return void
 * 
*/
void SerialStorage::payloadRestorator(String payload)
{
    u8_t newLine = payload.length();
    for (u8_t i = 0; i < payload.length(); i++)
    {
        if (payload.charAt(i) == '\n')
        {
            newLine = i;
        }
    }
    size_t topicIndex = firstSpaceIndex(payload, 1);

    String topic = payload.substring(0, topicIndex - 2);
    String send = payload.substring(topicIndex, newLine);
    Utils::log("PAYLOAD_RESTORATION_EVENT", send);
    // Serial.println(send);
    storePayload(send, topic);
}

/** 
 * @private 
 * 
 * getPopStartIndex
 * 
 * Strips a pop tag from a serial payload string
 * 
 * @param String read 
 * 
 * @return String
 * 
*/
String SerialStorage::getPopStartIndex(String read)
{
    if (read.startsWith("pop"))
    {
        size_t startIndex = firstSpaceIndex(read, 2);
        return read.substring(startIndex);
    }

    return read;
}

/** 
 * @public 
 * 
 * popOfflineCollection
 * 
 * sends a payload over seral for storage
 * 
 * @param u8_t count
 * 
 * @return void
 * 
*/
void SerialStorage::popOfflineCollection(u8_t count)
{
    sendingOffline = true;
    delay(100);
    String send = "pop " + String(count);
    Serial1.println(send);
    Serial1.flush();
    delay(100);
    sendingOffline = false;
}

/** 
 * @private 
 * 
 * sendPopRead
 * 
 * Sends a collected payload over the processor.
 * A payload requires the to be space seperated by event payload.
 * 
 * Ai/Post {device: me, payload: {}}
 * 
 * @param u8_t count
 * 
 * @return void
 * 
*/
bool SerialStorage::sendPopRead()
{
    size_t index = firstSpaceIndex(popString, 1);
    String SEND_TO = popString.substring(0, index - 1);
    String SEND = popString.substring(index);
    Utils::log("SERIAL_POP_SEND" , SEND_TO + " " + SEND);
    bool published = this->holdProcessor->publish(SEND_TO, SEND);
    return published;
}

/** 
 * @private 
 * 
 * sendPopRead
 * 
 * Returned serial payloads come in chunks appended with "pop". 
 * This function collects the payload until it finds the end chuck
 * 
 * @param String read
 * 
 * @return void
 * 
*/
void SerialStorage::processPop(String read)
{
    popString += getPopStartIndex(read);
    if (popString.endsWith("}"))
    {
        if (!sendPopRead())
        {
            this->payloadRestorator(popString);
        }
        popString = "";
    }
}

/** 
 * @private 
 * 
 * firstSpaceIndex
 * 
 * Sends back the index of the first space found with a given index
 * 
 * @param String value
 * @param u8_t index - it can be an occurance of a space
 * 
 * @return size_t
 * 
*/
size_t SerialStorage::firstSpaceIndex(String value, u8_t index)
{
    size_t size = value.length();
    u8_t count = 0;
    size_t sendIndex = -1;
    for (size_t i = 0; i < size; i++)
    {
        char holdValue = value.charAt(i);
        if (holdValue == ' ')
        {
            count++;
        }

        if (count == index)
        {
            sendIndex = i + 1;
            break;
        }
    }

    return sendIndex;
}

/** 
 * @public 
 * 
 * storePayload
 * 
 * Sends a payload over serial to be stored
 * 
 * @param String value
 * @param String topic 
 * 
 * @return size_t
 * 
*/
void SerialStorage::storePayload(String payload, String topic)
{
    sendingOffline = true;
    String send = topic + " " + payload + "\n";
    size_t length = send.length();
    u8_t MAX_CHUNCK = 100;
    String push = "push ";
    size_t i = 0;   
    while (i < length)
    {
        size_t local = 0;
        for (size_t j = 0; j < push.length(); j++)
        {
            // Serial.write(push.charAt(j));
            Serial1.write(push.charAt(j));
            local++;
        }

        for (size_t j = 0; j < MAX_CHUNCK - local && i < length; j++)
        {
            // Serial.write(send.charAt(i));
            Serial1.write(send.charAt(i));
            local++;
            i++;
        }

        if (i < length) {
            Serial1.write('\0');
            delay(100);
        }
    }
    Serial1.flush();
    delay(100);
    sendingOffline = false;
}

/** 
 * @public 
 * 
 * loop
 * 
 * Works off main loop. Gets the serial content from bootstrap
 * 
 * @return void
 * 
*/
void SerialStorage::loop() 
{
    String pop = this->boots->fetchSerial("pop");
    if (!pop.equals("")) {
        processPop(pop);
    }
}