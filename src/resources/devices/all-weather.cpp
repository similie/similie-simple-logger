#include "all-weather.h"

AllWeather::~AllWeather()
{
}
AllWeather::AllWeather(Bootstrap *boots)
{
    this->boots = boots;
}

AllWeather::AllWeather()
{
}

void AllWeather::popOfflineCollection(Processor *processor, String topic, u8_t count)
{
    this->holdProcessor = processor;
    Serial1.println("pop 5");
}

size_t AllWeather::firstSpaceIndex(String value, u8_t index)
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

String AllWeather::getPopStartIndex(String read)
{
    if (read.startsWith("pop"))
    {
        size_t startIndex = firstSpaceIndex(read, 2);
        return read.substring(startIndex);
    }

    return read;
}

bool AllWeather::sendPopRead()
{

    size_t index = firstSpaceIndex(popString, 1);
    String SEND_TO = popString.substring(0, index - 1);
    bool published = this->holdProcessor->publish(SEND_TO, popString.substring(index));
    return published;
}

void AllWeather::processPop(String read)
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

    ourReading = "";
}

void AllWeather::payloadRestorator(String payload)
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

void AllWeather::storePayload(String payload, String topic)
{
    this->sendingOfflinePayload = true;
    String send = topic + " " + payload + "\n";

    size_t length = send.length();
    size_t offset = 1;
    u8_t MAX_CHUNCK = 100;
    char buffer[MAX_CHUNCK + offset];
    String push = "push ";
    size_t i = 0;
    while (i < length)
    {
        size_t local = 0;
        for (size_t j = 0; j < push.length(); j++)
        {
            buffer[j] = push.charAt(j);
            // Serial.write(push.charAt(j));
            Serial1.write(push.charAt(j));
            local++;
        }

        for (size_t j = 0; j < MAX_CHUNCK - local && i < length; j++)
        {
            buffer[j] = send.charAt(i);
            // Serial.write(send.charAt(i));
            Serial1.write(send.charAt(i));
            local++;
            i++;
        }
        // Serial.write('\0');
        Serial1.write('\0');
        buffer[local + offset] = 0;
    }

    Serial1.flush();
    delay(300);
    this->sendingOfflinePayload = false;
}

void AllWeather::nullifyPayload(const char *key)
{
}

float AllWeather::extractValue(float values[], size_t key, size_t max)
{
    switch (key)
    {
    case gust_wind_speed:
    case strike_distance:
        return utils.getMax(values, max);
    case precipitation:
    case strikes:
        return utils.getSum(values, max);
    default:
        return utils.getMedian(values, max);
    }
}

float AllWeather::extractValue(float values[], size_t key)
{

    size_t MAX = readSize();
    return extractValue(values, key, MAX);
}

void AllWeather::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
    print();
    size_t MAX = readSize();
    for (size_t i = 0; i < AllWeather::PARAM_LENGTH; i++)
    {
        if (i == null_val)
        {
            continue;
        }
        String param = valueMap[i];
        if (param.equals(NULL) || param.equals(" ") || param.equals(""))
        {
            param = "_FAILURE_";
        }
        float paramValue = extractValue(VALUE_HOLD[i], i, MAX);
        if (paramValue == NO_VALUE)
        {
            maintenanceTick++;
        }

        writer.name(param.c_str()).value(paramValue);
    }
}

size_t AllWeather::skipMultiple(unsigned int size)
{
    size_t start = 1;
    for (size_t i = 1; i < boots->getMaxVal(); i++)
    {
        unsigned int test = size * i;
        float multiple = (float)test / READ_THRESHOLD;
        if (multiple >= 1)
        {
            start = i;
            break;
        }
    }
    return start;
}

bool AllWeather::readReady()
{
    if (this->sendingOfflinePayload)
    {
        return !this->sendingOfflinePayload;
    }

    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = skipMultiple(size);
    return readAttempt >= skip;
}
size_t AllWeather::readSize()
{
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = skipMultiple(size);
    size_t expand = floor(boots->getMaxVal() / skip);
    return expand;
}

String AllWeather::getReadContent()
{
    return serialMsgStr;
}

void AllWeather::read()
{
    readAttempt++;

    if (!readReady())
    {
        return;
    }
    readAttempt = 0;
    String content = getReadContent();
    //Serial.println(content);
    Serial1.println(content);
    Serial1.flush();
}

void AllWeather::loop()
{
    if (!readCompile)
    {
        readSerial();
    }

    parseSerial();
}

void AllWeather::clear()
{
    for (size_t i = 0; i < AllWeather::PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            VALUE_HOLD[i][j] = NO_VALUE;
        }
    }
}

void AllWeather::parseSerial()
{
    if (readyRead)
    {
        readyRead = false;
        if (ourReading.startsWith("pop"))
        {
            return this->processPop(ourReading);
        }

        readCompile = true;

        //float values[17];
        size_t j = 1;
        // we had paramsize +1
        for (size_t i = 0; i < PARAM_LENGTH; i++)
        {
            char c = ourReading.charAt(j);
            j++;
            if (c == '+' || c == '-')
            {
                String buff = "";
                char d = ' ';
                while (d != '+' && d != '-' && d != '\n' && d != '\0')
                {
                    d = ourReading.charAt(j);
                    buff += String(d);
                    j++;
                    d = ourReading.charAt(j);
                }
                if (utils.invalidNumber(buff))
                {
                    continue;
                }

                if (utils.containsChar('.', buff))
                {
                    float value = buff.toFloat();
                    if (c == '-')
                    {
                        value = value * -1;
                    }
                    utils.insertValue(value, VALUE_HOLD[i], boots->getMaxVal());
                }
                else
                {
                    float value = buff.toInt();
                    if (c == '-')
                    {
                        value = value * -1;
                    }
                    utils.insertValue(value, VALUE_HOLD[i], boots->getMaxVal());
                }
            }
        }
        // counter++;
        // Serial.print("COUNT ");
        // Serial.println(counter);
        readCompile = false;
        ourReading = "";
    }
}

int AllWeather::readSerial()

{
    int avail = Serial1.available();
    for (int i = 0; i < avail; i++)
    {
        char inChar = Serial1.read();
        //Serial.print(inChar);
        if (inChar == '\n' || inChar == '\0')
        {
            // Serial.println("READY READ TRUE");
            readyRead = true;
            return 1;
        }
        if (inChar != '\r')
        {
            ourReading += String(inChar);
        }
    }

    return 0;
}

void AllWeather::print()
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < readSize(); j++)
        {
            //  Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(valueMap[i]), j, VALUE_HOLD[i][j]);
        }
    }
}

void AllWeather::init()
{

    Serial1.begin(9600);
}

size_t AllWeather::buffSize()
{
    return 600;
}

u8_t AllWeather::paramCount()
{
    return PARAM_LENGTH;
}

u8_t AllWeather::matenanceCount()
{
    u8_t maintenance = this->maintenanceTick;
    maintenanceTick = 0;
    return maintenance;
}
