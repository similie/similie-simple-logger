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

/**
 * default constructor
 * @param Bootstrap boots - bootstrap object
 */
RikaAirQuality::RikaAirQuality(Bootstrap *boots)
{
    this->boots = boots;
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
 * @public
 *
 * extractValue
 *
 * Returns a float value for the specific parameter key
 *
 * @param float value[] - all the read values taken since the last publish event
 * @param size_t key - the key being selected for
 * @param size_t max - max size of the search scope
 *
 * @return float
 */
float RikaAirQuality::extractValue(float values[], size_t key, size_t max)
{

    switch (key)
    {
    case pm2_5:
    case pm10:
        return utils.getMedian(values, max);
    default:
        return utils.getMedian(values, max) / 10;
    }
}

/**
 * @public
 *
 * extractValue
 *
 * Overload for the above function @see extractValue(float values[], size_t key, size_t max)
 *
 * @param float value[] - all the read values taken since the last publish event
 * @param size_t key - the key being selected for
 *
 * @return float
 */
float RikaAirQuality::extractValue(float values[], size_t key)
{
    size_t MAX = readSize();
    return extractValue(values, key, MAX);
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
    // print();
    size_t MAX = readSize();
    for (size_t i = 0; i < RikaAirQuality::PARAM_LENGTH; i++)
    {
        String param = valueMap[i];
        // set the param as having failed
        if (param.equals(NULL) || param.equals(" ") || param.equals(""))
        {
            param = "_FAILURE_";
        }
        float paramValue = extractValue(VALUE_HOLD[i], i, MAX);
        if (isnan(paramValue))
        {
            paramValue = NO_VALUE;
        }

        if (paramValue == NO_VALUE)
        {
            maintenanceTick++;
        }

        writer.name(param.c_str()).value(paramValue);
    }
}

/**
 * @private
 *
 * readReady
 *
 * Returns true with the read attempt is ready to move forward
 *
 * @return bool
 */
bool RikaAirQuality::readReady()
{
    if (!waitFor(SerialStorage::notSendingOfflineData, 1000))
    {
        return SerialStorage::notSendingOfflineData();
    }
    /**
     * We don't want the all weather system to take a reading every interval,
     * so we limit our read attempts
     */
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal(), READ_THRESHOLD);
    return readAttempt >= skip;
}

/**
 * @private
 *
 * readSize
 *
 * Returns the numbers of reads that are available
 *
 * @return size_t
 */
size_t RikaAirQuality::readSize()
{
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal(), READ_THRESHOLD);
    size_t expand = floor(boots->getMaxVal() / skip);
    return expand;
}

/**
 * @public
 *
 * serialResponseIdentity
 *
 * Returns a reponse identity based on a given send identity
 * for seralized requests.
 *
 * @return String
 */
String RikaAirQuality::serialResponseIdentity()
{
    return this->REQUEST_TAG;
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
    return serialResponseIdentity();
}

/**
 * @private
 *
 * fetchReading
 *
 * Called on the main loop to attempt to pull serial requests
 * for this specific device.
 *
 * @return String
 */
String RikaAirQuality::fetchReading()
{
    if (!readCompile)
    {
        return boots->fetchSerial(this->serialResponseIdentity());
    }
    return "";
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
    readAttempt++;
    if (!readReady())
    {
        return;
    }
    readAttempt = 0;
    String content = getReadContent();
    Serial1.println(content);
    delay(100);
    Serial1.flush();
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
    String completedSerialItem = boots->fetchSerial(this->serialResponseIdentity());
    if (!completedSerialItem.equals(""))
    {
        parseSerial(completedSerialItem);
    }
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
    for (size_t i = 0; i < RikaAirQuality::PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            VALUE_HOLD[i][j] = NO_VALUE;
        }
    }
}

/**
 * @private
 *
 * replaceSerialResponceItem
 *
 * Strips out the serialized identity from the collected string
 *
 * @param String message - serial message
 *
 * @return bool
 */
String RikaAirQuality::replaceSerialResponceItem(String message)
{
    String replaced = message.replace(this->serialResponseIdentity() + " ", "");
    return replaced;
}

/**
 * @private
 *
 * parseSerial
 *
 * Parses the serial data that converts it to actual numbers
 * used during a read event
 *
 * @param String message - serial message
 *
 * @return bool
 */
void RikaAirQuality::parseSerial(String ourReading)
{

    if (!ourReading.startsWith(serialResponseIdentity()))
    {
        return Utils::log("RIKA_AIR_QUALITY", "Invalid Message String");
    }

    ourReading = replaceSerialResponceItem(ourReading);
    readCompile = true;
    // @here
    utils.parseSplitReadSerial(ourReading, PARAM_LENGTH, boots->getMaxVal(), valueMap, VALUE_HOLD);
    readCompile = false;
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
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < readSize(); j++)
        {
            Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(valueMap[i]), j, VALUE_HOLD[i][j]);
        }
    }
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
    boots->startSerial();
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
    return 250; // 600;
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
    return PARAM_LENGTH;
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
