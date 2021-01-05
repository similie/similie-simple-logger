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

void AllWeather::read()
{
    readAttempt++;

    if (!readReady())
    {
        return;
    }
    readAttempt = 0;
    Serial1.println(serialMsgStr);
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
        Serial.println(ourReading);
        readyRead = false;
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
        if (inChar == '\n' || inChar == '\0')
        {
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
            Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(valueMap[i]), j, VALUE_HOLD[i][j]);
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
