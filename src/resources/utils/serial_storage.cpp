#include "serial_storage.h"

static bool sendingOffline = false;

/**
 * @constructor
 *
 * @param Processor * holdProcessor - the processor for sending a payload
 * @param Bootstrap * boots - the bootstrap object
 */
SerialStorage::SerialStorage(Processor *holdProcessor, Bootstrap *boots)
{
    this->holdProcessor = holdProcessor;
    this->boots = boots;
    invalidatePopArray();
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
bool SerialStorage::notSendingOfflineData()
{
    return !sendingOffline;
}

/**
 * @private
 *
 * getNewLineIndex
 *
 * gets the index of the last \n character
 *
 * @param String payload
 *
 * @return int
 *
 */
int SerialStorage::getNewLineIndex(String payload)
{
    int newLine = INVALID;
    // start at the end. It is more likely to find there
    // Serial.print("WHAT THE HELL ");Serial.println(payload.length());
    for (int i = payload.length() - 1; i >= 0; i--)
    {
        // Serial.print("INDEX ");Serial.print(i); Serial.print(" ");
        // Serial.println(payload.charAt(i));
        if (payload.charAt(i) == '\n' || payload.charAt(i) == '}')
        {

            newLine = payload.charAt(i) == '}' ? i + 1 : i;
            break;
        }
    }
    return newLine;
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
 * @return popContent {struct}
 *
 */
popContent SerialStorage::payloadRestorator(String payload)
{
    int newLine = getNewLineIndex(payload);
    popContent pop = {false};

    if (newLine == INVALID)
    {
        return pop;
    }

    short int topicIndex = firstSpaceIndex(payload, 1);
    if (topicIndex != INVALID)
    {
        String topic = payload.substring(0, topicIndex - 1);
        String send = payload.substring(topicIndex, newLine);
        pop.valid = true;
        pop.key = topic;
        pop.content = send;
    }
    return pop;
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
        short int startIndex = firstSpaceIndex(read, 2);
        if (startIndex != INVALID)
        {
            return read.substring(startIndex);
        }
    }

    return read;
}

bool SerialStorage::sendPop(popContent content)
{
    String SEND_TO = content.key;
    String SEND = content.content;
    Utils::log("SERIAL_POP_SEND", SEND_TO + " " + SEND);
    bool published = this->holdProcessor->publish(SEND_TO, SEND);
    return published;
}

/**
 * @private
 *
 * resetPopElement
 *
 * Sets a pop item to null at a specific index
 *
 * @param short int index
 *
 */

void SerialStorage::resetPopElement(short int index)
{
    popContent pop = {false, "", ""};
    popStore[index] = pop;
}

/**
 * @private
 *
 * checkPopSend
 *
 * Checks to see if there are any valid send items
 *
 */
void SerialStorage::checkPopSend()
{
    unsigned long now = millis();
    if (now - lastSend < sendWait)
    {
        return;
    }
    lastSend = now;

    if (!hasPopContent)
    {
        return;
    }

    short int index = findValidPopIndex();
    if (index == -1)
    {
        hasPopContent = false;
        return;
    }
    popContent pop = popStore[index];
    bool sent = sendPop(pop);
    // Utils::log("SENDING_POP_ELEMENT", pop.key + " " + pop.content + " " + String(sent));
    if (sent)
    {
        sendIndex = index + 1;
    }
    else
    {
        storePayload(pop.content, pop.key);
    }

    resetPopElement(index);
}

/**
 * @private
 *
 * invalidatePopArray
 *
 * Invalidates all active pop elements
 *
 */
short int SerialStorage::findValidPopIndex()
{
    short int index = -1;
    uint8_t cycles = 0;
    uint8_t startIndex = sendIndex;
    while (cycles < POP_COUNT_VALUE)
    {
        if (startIndex >= POP_COUNT_VALUE)
        {
            startIndex = 0;
        }
        popContent pop = popStore[startIndex];
        if (pop.valid)
        {
            index = startIndex;
            break;
        }
        startIndex++;
        cycles++;
    }
    return index;
}

/**
 * @private
 *
 * invalidatePopArray
 *
 * Invalidates all active pop elements
 *
 */
void SerialStorage::invalidatePopArray()
{
    for (uint8_t i = 0; i < POP_COUNT_VALUE; i++)
    {
        resetPopElement(i);
    }
    hasPopContent = false;
}

/**
 * @private
 *
 * storePayloadToSend
 *
 * Adds the popContent to the send array
 *
 * @param popContent {struct}
 *
 * @return bool
 *
 */
short int SerialStorage::storePayloadToSend(popContent content)
{

    short int index = -1;
    // Serial.print("GHEE "); Serial.print(content.valid); Serial.print(" "); Serial.println(content.key);

    if (!content.valid)
    {
        return index;
    }

    uint8_t cycles = 0;
    uint8_t startIndex = sendIndex;
    while (cycles < POP_COUNT_VALUE)
    {
        if (startIndex >= POP_COUNT_VALUE)
        {
            startIndex = 0;
        }
        popContent pop = popStore[startIndex];
        // Serial.print("HEHE "); Serial.print(startIndex); Serial.print(" ");Serial.println(pop.valid);
        if (pop.valid != true)
        {
            index = startIndex;
            break;
        }
        startIndex++;
        cycles++;
    }
    // Serial.print("GOT THIS SHIT ");Serial.println(index);
    return index;
}

/**
 * @public
 *
 * popOfflineCollection
 *
 * sends a payload over seral for storage
 *
 * @param uint8_t count
 *
 * @return void
 *
 */
void SerialStorage::popOfflineCollection()
{
    if (!boots->hasSerial())
    {
        return;
    }

    sendingOffline = true;
    delay(100);
    String send = "pop " + String(POP_COUNT_VALUE);
    Serial1.println(send);
    Serial1.flush();
    delay(100);
    sendingOffline = false;
}

// /**
//  * @private
//  *
//  * sendPopRead
//  *
//  * Sends a collected payload over the processor.
//  * A payload requires the to be space seperated by event payload.
//  *
//  * Ai/Post {device: me, payload: {}}
//  *
//  * @return void
//  *
// */
// bool SerialStorage::sendPopRead()
// {
//     short int index = firstSpaceIndex(popString, 1);
//     if (index == INVALID)
//     {
//         return false;
//     }
//     // String SEND_TO = popString.substring(0, index - 1);
//     // String SEND = popString.substring(index);
//     // Utils::log("SERIAL_POP_SEND", SEND_TO + " " + SEND);
//     // bool published = this->holdProcessor->publish(SEND_TO, SEND);
//     // Serial.println(published);
//     // return published;
// }

/**
 * @private
 *
 * processPop
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
    // Serial.print("GOT SOME SERIAL ");Serial.println(read);
    if (popString.endsWith("}"))
    {
        popContent pop = payloadRestorator(popString);
        short int index = storePayloadToSend(pop);
        if (index != INVALID)
        {
            popStore[index] = pop;
            hasPopContent = true;
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
 * @param uint8_t index - it can be an occurance of a space
 *
 * @return size_t
 *
 */
short int SerialStorage::firstSpaceIndex(String value, uint8_t index)
{
    size_t size = value.length();
    uint8_t count = 0;
    short int sendIndex = -1;
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
    if (!boots->hasSerial())
    {
        return;
    }

    sendingOffline = true;

    bool constrained = boots->isCoProcessorMemoryConstrained();
    Utils::log("PROCESSOR_" + boots->getProcessorName(), "constrained? " + String(constrained));
    // let's let everything clear from the buffer if there's
    // data being delivered
    Serial1.flush();
    delay(100);
    String send = topic + " " + payload + "\n";
    // String send = topic + " " + payload;
    size_t length = send.length();
    // there is a lot of noise this function picks up on the co-processor. I think it is related
    // with tight memory limitations with the 32u4 chip.
    uint8_t MAX_CHUNCK = 30;
    String push = "push ";
    if (constrained)
    {
        size_t i = 0;
        while (i < length)
        {
            // print push to tag the payload as a storage event
            Serial1.print(push);
            for (size_t j = 0; j < MAX_CHUNCK && i < length; j++)
            {
                Serial1.write(send.charAt(i));
                i++;
            }

            if (i < length)
            {
                Serial1.write('\0');
                delay(100);
            }
        }
    }
    else
    {
        Serial1.print(push + send);
    }

    delay(50);
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
    if (!pop.equals(""))
    {
        processPop(pop);
    }

    checkPopSend();
}