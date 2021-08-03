#include "all-weather.h"

AllWeather::~AllWeather()
{
}
AllWeather::AllWeather(Bootstrap *boots)
{
    this->boots = boots;
}

AllWeather::AllWeather(Bootstrap *boots, int identity)
{
    this->boots = boots;
    this->sendIdentity = identity;
}

AllWeather::AllWeather()
{
}

String AllWeather::name() {
    return this->deviceName;
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


bool AllWeather::readReady()
{
    if (!waitFor(SerialStorage::notSendingOfflineData, 1000))
    {
        return SerialStorage::notSendingOfflineData();
    }

    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal() , READ_THRESHOLD);
    return readAttempt >= skip;
}
size_t AllWeather::readSize()
{
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal() , READ_THRESHOLD);
    size_t expand = floor(boots->getMaxVal() / skip);
    return expand;
}

String AllWeather::serialResponseIdentity()
{
    return utils.receiveDeviceId(this->sendIdentity);
}

String AllWeather::constrictSerialIdentity()
{
   return utils.requestDeviceId(this->sendIdentity, serialMsgStr);
}

String AllWeather::getReadContent()
{
    if (this->hasSerialIdentity())
    {
        return constrictSerialIdentity();
    }

    return serialMsgStr;
}

String AllWeather::fetchReading()
{
    if (!readCompile)
    {
        return boots->fetchSerial(this->serialResponseIdentity());
    }
    return "";
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
    Serial1.println(content);
    Serial1.flush();
}

void AllWeather::loop()
{
    String completedSerialItem = boots->fetchSerial(this->serialResponseIdentity());
    if (!completedSerialItem.equals(""))
    {
        parseSerial(completedSerialItem);
    }
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

bool AllWeather::hasSerialIdentity()
{
    return this->sendIdentity > -1;
}

bool AllWeather::inValidMessageString(String message)
{
    return this->hasSerialIdentity() && !message.startsWith(this->serialResponseIdentity());
}

String AllWeather::replaceSerialResponceItem(String message)
{
    if (!this->hasSerialIdentity())
    {
        return message;
    }
    String replaced = message.replace(this->serialResponseIdentity() + " ", "");
    return replaced;
}

void AllWeather::parseSerial(String ourReading)
{
    if (inValidMessageString(ourReading))
    {
        Utils::log("ALL_WEATHER_MESSAGE", "Invalid Message String");
        return;
    }

    ourReading = replaceSerialResponceItem(ourReading);
    readCompile = true;
    utils.parseSerial(ourReading, PARAM_LENGTH, boots->getMaxVal(), VALUE_HOLD);
    readCompile = false;
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
    boots->startSerial();
}

void AllWeather::restoreDefaults()
{
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
