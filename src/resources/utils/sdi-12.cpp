#include "sdi-12.h"
/**
 * @description
 *
 * Works the Atmos 41 all-in-one weather sensor from Meter.
 * https://www.metergroup.com/environment/products/atmos-41-weather-station/
 *
 * Since particle does not support SDI-12, we use a 32u4/SAMD co-processor.
 * https://www.adafruit.com/product/2796
 * The source code we use can be found: https://github.com/similie/sdi12-SDI12Device-interface
 *
 */

/**
 * deconstructor
 */
SDI12Device::~SDI12Device()
{
}

/**
 * default constructor
 * @param Bootstrap boots - bootstrap object
 */
SDI12Device::SDI12Device(Bootstrap *boots, SDIParamElements *elements)
{
    this->boots = boots;
    this->childElements = elements;
} // AllWeatherElements

/**
 * constructor
 * @param Bootstrap boots - bootstrap object
 * @param int identity - numerical value used to idenify the device
 */
SDI12Device::SDI12Device(Bootstrap *boots, int identity, SDIParamElements *elements)
{
    this->boots = boots;
    this->sendIdentity = identity;
    this->childElements = elements;
}

/**
 * default constructor
 * @param Bootstrap boots - bootstrap object
 */
SDI12Device::SDI12Device(Bootstrap *boots)
{
    this->boots = boots;
} // AllWeatherElements

/**
 * constructor
 * @param Bootstrap boots - bootstrap object
 * @param int identity - numerical value used to idenify the device
 */
SDI12Device::SDI12Device(Bootstrap *boots, int identity)
{
    this->boots = boots;
    this->sendIdentity = identity;
}

void SDI12Device::setElements(SDIParamElements *elements)
{
    this->childElements = elements;
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
String SDI12Device::name()
{
    return getElements()->getDeviceName();
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
float SDI12Device::extractValue(float values[], size_t key, size_t max)
{
    return getElements()->extractValue(values, key, max);
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
float SDI12Device::extractValue(float values[], size_t key)
{
    size_t MAX = readSize();
    return extractValue(values, key, MAX);
}

void SDI12Device::runSingleSample()
{
    if (!SINGLE_SAMPLE)
    {
        return;
    }
    if (READ_OVER_WIRE)
    {
        readWire();
    }
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
void SDI12Device::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
    runSingleSample();
    size_t MAX = readSize();
    String *valuemap = getElements()->getValueMap();
    Utils::log("PUBLISHING_VALUES", "Param Count: " + String(getElements()->getTotalSize()));
    for (size_t i = 0; i < getElements()->getTotalSize(); i++)
    {
        if (i == null_val)
        {
            continue;
        }
        String param = valuemap[i];
        // set the param as having failed
        if (param.equals(NULL) || param.equals(" ") || param.equals(""))
        {
            maintenanceTick++;
            param = "_FAILURE_" + String(maintenanceTick);
        }
        float paramValue = extractValue(getElements()->getMappedValue(i), i, MAX);
        if (isnan(paramValue))
        {
            paramValue = NO_VALUE;
        }

        if (paramValue == NO_VALUE)
        {
            maintenanceTick++;
        }
        // Utils::log("SETTING PARAM " + param, String(paramValue));
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
bool SDI12Device::readReady()
{
    if (!isConnected())
    {
        return false;
    }

    if (SINGLE_SAMPLE)
    {
        return true;
    }

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
size_t SDI12Device::readSize()
{

    if (SINGLE_SAMPLE)
    {
        return 1;
    }

    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal(), READ_THRESHOLD);
    size_t expand = floor(boots->getMaxVal() / skip);
    return expand;
}

Bootstrap *SDI12Device::getBoots()
{
    return boots;
}

/**
 * @public
 *
 * serialResponseIdentity
 *
 * Returns a response identity based on a given send identity
 * for serialized requests.
 *
 * @return String
 */
String SDI12Device::serialResponseIdentity()
{
    return utils.receiveDeviceId(this->sendIdentity);
}

String SDI12Device::getCmd()
{
    return utils.getConvertedAddressCmd(serialMsgStr, sendIdentity);
}

SDIParamElements *SDI12Device::getElements()
{
    return this->childElements;
}

/**
 * @public
 *
 * constrictSerialIdentity
 *
 * Returns a request identity based on a given send identity along with
 * the serial command the system is requiring for seralized requests.
 *
 * @return String
 */
String SDI12Device::constrictSerialIdentity()
{
    return utils.requestDeviceId(sendIdentity, getCmd());
}

/**
 * @private
 *
 * constrictSerialIdentity
 *
 * Returns a request string for serial requests
 *
 * @return String
 */
String SDI12Device::getReadContent()
{
    if (this->hasSerialIdentity())
    {
        return constrictSerialIdentity();
    }

    return utils.getConvertedAddressCmd(getCmd(), sendIdentity);
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
String SDI12Device::fetchReading()
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
 * readSerial
 *
 * Called by the device manager when a read request is required
 *
 * @return void
 */
void SDI12Device::readSerial()
{

    if (READ_OVER_WIRE)
    {
        return;
    }

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

bool SDI12Device::isConnected()
{
    if (!READ_ON_LOW_ONLY)
    {
        return true;
    }
    return digitalRead(DEVICE_CONNECTED_PIN) == LOW;
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
String SDI12Device::getWire(String content)
{
    return this->boots->getCommunicator().sendAndWaitForResponse(Bootstrap::coProcessorAddress, content, this->boots->defaultWireWait(), WIRE_TIMEOUT);
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
void SDI12Device::readWire()
{
    readAttempt++;
    if (!readReady())
    {
        return;
    }
    readAttempt = 0;
    String identity = this->serialResponseIdentity();
    String content = String(getReadContent() + "\n");
    String response = getWire(content);
    if (boots->wireContainsError(response))
    {
        return Utils::log("READ_WIRE_ERROR", response);
    }
    Utils::log("WIRE_SDI_CONTENT_RESPONSE", response);
    parseSerial(response);
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
void SDI12Device::read()
{
    if (READ_OVER_WIRE)
    {
        if (SINGLE_SAMPLE)
        {
            return;
        }
        return readWire();
    }
    return readSerial();
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
void SDI12Device::loop()
{

    if (READ_OVER_WIRE)
    {
        return;
    }

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
void SDI12Device::clear()
{
    for (size_t i = 0; i < getElements()->getTotalSize(); i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            getElements()->setMappedValue(NO_VALUE, i, j);
        }
    }
}

/**
 * @private
 *
 * clear
 *
 * Has a specific serial numerical identity. An ID of -1 indicates
 * There is no ID specified in the constructor.
 *
 * @return bool
 */
bool SDI12Device::hasSerialIdentity()
{
    return utils.hasSerialIdentity(this->sendIdentity);
}

/**
 * @private
 *
 * uniqueName
 *
 * What's it's unique name for cloud functions
 *
 * @return String
 */
String SDI12Device::uniqueName()
{
    if (this->hasSerialIdentity())
    {
        return this->name() + String(this->sendIdentity);
    }
    return this->name();
}
/**
 * @private
 *
 * replaceSerialResponseItem
 *
 * Strips out the serialized identity from the collected string
 *
 * @param String message - serial message
 *
 * @return bool
 */
String SDI12Device::replaceSerialResponseItem(String message)
{
    if (!hasSerialIdentity())
    {
        return message;
    }
    String replaced = message.replace(serialResponseIdentity() + " ", "");
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
void SDI12Device::parseSerial(String ourReading)
{
    if (!READ_OVER_WIRE)
    {
        if (utils.inValidMessageString(ourReading, this->sendIdentity))
        {
            return Utils::log("ALL_WEATHER_MESSAGE", "Invalid Message String");
        }

        ourReading = replaceSerialResponseItem(ourReading);
    }
    readCompile = true;
    utils.parseSerial(ourReading, getElements()->getTotalSize(), boots->getMaxVal(), getElements()->valueHold);
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
void SDI12Device::print()
{
    for (size_t i = 0; i < getElements()->getTotalSize(); i++)
    {
        for (size_t j = 0; j < readSize(); j++)
        {
            Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(getElements()->getValueMap()[i]), j, getElements()->getMappedValue(i, j));
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
void SDI12Device::init()
{
    if (READ_ON_LOW_ONLY)
    {
        pinMode(DEVICE_CONNECTED_PIN, INPUT);
    }

    if (READ_OVER_WIRE)
    {
        return;
    }

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
void SDI12Device::restoreDefaults()
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
size_t SDI12Device::buffSize()
{
    return getElements()->getBuffSize(); // 600;
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
uint8_t SDI12Device::paramCount()
{
    return getElements()->getTotalSize();
}

/**
 * @public
 *
 * maintenanceCount
 *
 * How many sensors appear non operational. The system will use this to
 * determine if device is unplugged or there are a few busted sensors.
 *
 * @return uint8_t
 */
uint8_t SDI12Device::maintenanceCount()
{
    if (!isConnected())
    {
        Utils::log("DEVICE DISCONNECTED", "SKIPPING READ");
        return paramCount();
    }
    uint8_t maintenance = this->maintenanceTick;
    maintenanceTick = 0;
    return maintenance;
}
