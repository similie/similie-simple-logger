#include "serial_storage.h"

static bool sendingOffline = false;

SerialStorage::SerialStorage()
{

}

SerialStorage::SerialStorage(Processor *holdProcessor,  Bootstrap *boots)
{
    this->holdProcessor = holdProcessor;
    this->boots = boots;
}



SerialStorage::~SerialStorage() 
{

}


bool SerialStorage::notSendingOfflineData() {
    return !sendingOffline;
}


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
    Serial.println(send);
    storePayload(send, topic);
}

String SerialStorage::getPopStartIndex(String read)
{
    if (read.startsWith("pop"))
    {
        size_t startIndex = firstSpaceIndex(read, 2);
        return read.substring(startIndex);
    }

    return read;
}

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

bool SerialStorage::sendPopRead()
{

    size_t index = firstSpaceIndex(popString, 1);
    String SEND_TO = popString.substring(0, index - 1);
    String SEND = popString.substring(index);
    Utils::log("SERIAL_POP_SEND" , SEND_TO + " " + SEND);
    
    bool published = this->holdProcessor->publish(SEND_TO, SEND);
    return published;
}

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


void SerialStorage::loop() 
{
    String pop = this->boots->fetchSerial("pop");
    if (!pop.equals("")) {
        // Log.info(pop);
        processPop(pop);
    }

}