#include "rika-airquality.h"
/**
 * @description
 *
 * Works with Rika air quality sensors
 * https://www.rikasensor.com/
 *
 * We use a 32u4/SAMD co-processor.
 * https://www.adafruit.com/product/2796
 *
 */

/**
 * deconstructor
 */
RikaAirQuality::~RikaAirQuality()
{
}

RikaAirQuality::RikaAirQuality(Bootstrap *boots, String config, int address)
{
    this->boots = boots;
    this->config = config;
    this->address = address;
}

/**
 * default constructor
 * @param Bootstrap boots - bootstrap object
 */
RikaAirQuality::RikaAirQuality(Bootstrap *boots)
{
    this->boots = boots;
}

RikaAirQuality::RikaAirQuality(Bootstrap *boots, const char *config, int address)
{
    this->boots = boots;
    this->config = String(config);
    this->address = address;
}

/**
 * @public
 *
 * name
 *
 * Returns the device name
 *
 * @return String
 */
String RikaAirQuality::name()
{
    return this->deviceName;
}

/**
 * @private
 * @brief
 *
 * getWire
 *
 * pulls the wire for a given content
 *
 * @return String
 */
String RikaAirQuality::getWire(String content)
{
    return this->boots->getCommunicator().sendAndWaitForResponse(Bootstrap::coProcessorAddress, content, this->boots->defaultWireWait(), WAIT_TIMEOUT);
}

/**
 * @public
 *
 * publish
 *
 * Called when the system is ready to publish a data payload. This function pulls
 * together all the data that has been collected and places it in the writer object
 *
 * @param JSONBufferWriter &writer - the json writer object
 * @param uint8_t attempt_count - the number or reads attempted
 *
 * @return void
 */
void RikaAirQuality::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
    String content = getReadContent();
    String wireContent = getWire(getReadContent());
    if (boots->getCommunicator().containsError(wireContent))
    {
        return Utils::log("RIKA_AIR_QUALITY_ERROR", wireContent);
    }
    parseReadContent(wireContent);
    processValues(writer);
}

/**
 * @private
 *
 * getReadContent
 *
 * Returns a request string for serial requests
 *
 * @return String
 */
String RikaAirQuality::getReadContent()
{
    return REQUEST_TAG + " " + String(address) + "\n";
}

/**
 * @public
 *
 * read
 *
 * Called by the device manager when a read request is required
 *
 * @return void
 */
void RikaAirQuality::read()
{
}

size_t RikaAirQuality::populateStringContent()
{
    size_t length = strlen(config);
    paramHoldSize = 0;
    String set = "";
    for (size_t i = 0; i < length; i++)
    {
        char c = config[i];
        if (c == ';' || i == length - 1)
        {
            if (i == length - 1)
            {
                set += String(c);
            }
            paramHold[paramHoldSize] = set;
            set = "";
            paramHoldSize++;
            continue;
        }
        set += String(c);
    }
    return paramHoldSize;
}

void RikaAirQuality::printStringContent()
{
    for (size_t i = 0; i < paramHoldSize; i++)
    {
        String value = paramHold[i];
        Utils::log("SHOWING_VALUES", value);
    }
}

size_t RikaAirQuality::applyStringValue(size_t index, String value)
{
    while (index < STRING_CONVERT_SIZE)
    {
        String read = stringConvertBuffer[index];
        if (read.equals(""))
        {
            index++;
            continue;
        }

        int parsed = String(read).toInt();
        if (parsed == 0)
        {
            index++;
            continue;
        }

        if (value.equals(""))
        {
            index++;
        }
        break;
    }
    return index;
}

void RikaAirQuality::zeroOutParamValueHold()
{
    for (size_t i = 0; i < paramHoldSize; i++)
    {
        paramValues[i].param = "";
        paramValues[i].value = NO_VALUE;
    }
}

void RikaAirQuality::stuffValues()
{
    size_t index = 0;
    for (size_t i = 0; i < paramHoldSize; i++)
    {
        String param = paramHold[i];
        index = applyStringValue(index, param);
        if (param.equals(""))
        {
            continue;
        }

        if (index >= STRING_CONVERT_SIZE)
        {
            break;
        }
        String value = stringConvertBuffer[index];
        if (value.equals(""))
        {
            continue;
        }
        uint16_t parsed = String(value).toInt();
        ReadValues values = {param, parsed};
        paramValues[valueStore] = values;
        valueStore++;
        index++;
    }
}

int RikaAirQuality::getIndexForParam(String value)
{
    for (uint8_t i = 0; i < RIKA_MAX_PARAM_SIZE; i++)
    {
        String param = valueMap[i];
        if (param.equals(value))
        {
            return i;
        }
    }
    return -1;
}

float RikaAirQuality::convertValue(ReadValues *values)
{
    int param = getIndexForParam(values->param);
    uint8_t high = values->value >> 8;
    // uint8_t high = values->value >> 8; // high byte (0x12)
    // uint8_t low = values->value & 0x00FF; // low byte (0x34)
    if (param == -1)
    {
        return NO_VALUE;
    }

    switch (param)
    {
    case temp:
    case hum:
    case press:
        return values->value / 10;
    case so2:
    case no2:
    case o3:
    case ch4:
        return high / 10;
    /** pm2_5,pm10,co2 */
    default:
        return values->value;
    }
}

void RikaAirQuality::processValues(JSONBufferWriter &writer)
{
    for (size_t i = 0; i < valueStore; i++)
    {
        ReadValues value = paramValues[i];
        if (value.param.equals(""))
        {
            continue;
        }
        float converted = convertValue(&value);
        Utils::log(value.param.c_str(), String(converted));
        writer.name(value.param.c_str()).value(converted);
    }
}

void RikaAirQuality::parseReadContent(String results)
{
    Utils::log("RS485_RESPONSE", results);
    zeroOutParamValueHold();
    utils.splitStringToValues(results, stringConvertBuffer, STRING_CONVERT_SIZE);
    valueStore = 0;
    stuffValues();
}

/**
 * @public
 *
 * loop
 *
 * Cycles with the main loop
 *
 * @return void
 */
void RikaAirQuality::loop()
{
}

/**
 * @public
 *
 * clear
 *
 * Clears any data stored for reads
 *
 * @return void
 */
void RikaAirQuality::clear()
{
}

/**
 * @public
 *
 * parseSerial
 *
 * print
 *
 * Prints all the read data
 *
 * @return void
 */
void RikaAirQuality::print()
{
}

/**
 * @public
 *
 * init
 *
 * called on the system init
 *
 *
 * @return void
 */
void RikaAirQuality::init()
{
    populateStringContent();
}

/**
 * @public
 *
 * restoreDefaults
 *
 * Called by the controller when a global defaul is called.
 * Not required for this device.
 *
 * @return void
 */
void RikaAirQuality::restoreDefaults()
{
}

/**
 * @public
 *
 * buffSize
 *
 * Returns the expected max size for all the payload data
 *
 * @return size_t
 */
size_t RikaAirQuality::buffSize()
{
    return 50; // 600;
}

/**
 * @public
 *
 * paramCount
 *
 * Returns the number of parameters that the device will have
 *
 * @return size_t
 */
uint8_t RikaAirQuality::paramCount()
{
    return paramHoldSize;
}

/**
 * @public
 *
 * maintenanceCount
 *
 * How many sensors appear non operational. The system will use this to
 * determin if device is unplugged or there are a few busted sensors.
 *
 * @return uint8_t
 */
uint8_t RikaAirQuality::maintenanceCount()
{
    uint8_t maintenance = this->maintenanceTick;
    maintenanceTick = 0;
    return maintenance;
}
