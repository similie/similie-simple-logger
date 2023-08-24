#include "soil-moisture.h"

/**
 * @description
 *
 * Works the the Terros 11 all-in-one weather sensor from Meter.
 * https://www.metergroup.com/environment/products/teros-11/
 *
 * Since particle does not support SDI-12, we use a 32u4 co-processor.
 * https://www.adafruit.com/product/2796
 * The source code we use can be found: https://github.com/similie/sdi12-allweather-interface
 *
 */

/**
 * Deconstructor
 */
SoilMoisture::~SoilMoisture()
{
    setElements(&elements);
}

/**
 * Constructor
 *
 * @param Bootstrap * boots - the bootstrap object
 */
SoilMoisture::SoilMoisture(Bootstrap *boots) : SDI12Device(boots)
{
    setElements(&elements);
}

/**
 * Constructor
 *
 * @param Bootstrap * boots - the bootstrap object
 * @param int identity - the device ID that makes it unique in
 *      a multidevice environment
 */
SoilMoisture::SoilMoisture(Bootstrap *boots, int identity) : SDI12Device(boots, identity)
{
    setElements(&elements);
}

/**
 * @private
 *
 * setMoistureCalibration
 *
 * Cloud function for setting the device calibration
 *
 * @param String read - payload from the cloud
 *
 * @return int
 */
int SoilMoisture::setMoistureCalibration(String read)
{
    double val = Utils::parseCloudFunctionDouble(read, this->uniqueName());
    if (saveAddressForMoisture != -1)
    {
        VWCStruct store = {1, mineral_soil, val};
        EEPROM.put(saveAddressForMoisture, store);
    }
    multiple = val ? true : false; //  multiple = val;
    return 1;
}

/**
 * @private
 *
 * setMoistureCalibration
 *
 * Cloud function for setting the device calibration
 *
 * @param String read - payload from the cloud
 *
 * @return int
 */
int SoilMoisture::setMineralSoilCalibration(String read)
{
    int val = atoi(read);
    if (val < 0 || val > 1)
    {
        return 0;
    }
    if (saveAddressForMoisture != -1)
    {
        VWCStruct store = {1, val, multiple};
        EEPROM.put(saveAddressForMoisture, store);
    }
    mineral_soil = val ? true : false; //  multiple = val;
    return 1;
}

// /**
//  * @public
//  *
//  * name
//  *
//  * Returns the device name
//  *
//  * @return String
//  */
// String SoilMoisture::name()
// {
//     return this->deviceName;
// }

float SoilMoisture::applySoilMoistureEquation(float value)
{
    const double MINERAL_SOIL_MULTIPLE = multiple;                                           // 3.879e-4; // pow(3.879, -4); //  0.0003879;
    const double SOILESS_MEDIA_MULTIPLE[] = {0.0000000006771, 0.000005105, 0.01302, 10.848}; // {6.771e-10, 5.105e-6, 1.302e-2, 10.848}; // 0.00000000006771;
    if (mineral_soil)
    {
        return roundf(((MINERAL_SOIL_MULTIPLE * value) - 0.6956) * 100);
    }
    else
    {
        double eq = ((SOILESS_MEDIA_MULTIPLE[0] * pow(value, 3.0)) - (SOILESS_MEDIA_MULTIPLE[1] * pow(value, 2.0)) + (SOILESS_MEDIA_MULTIPLE[2] * value)) - SOILESS_MEDIA_MULTIPLE[3];
        return roundf(eq * 100);
    }
}

/**
 * @private
 *
 * multiplyValue
 *
 * Returns the selected value with the configured multiplyer
 *
 * @return float
 */
float SoilMoisture::multiplyValue(float value)
{
    return ((value == NO_VALUE) ? NO_VALUE : applySoilMoistureEquation(value));
}

/**
 * @private
 *
 * extractValue
 *
 * Applies any specific action or function to a specific parameter
 *
 * @param float values[] - the values of the param type
 * @param size_t key - the integer value of the param
 * @param size_t max - the max number of reads taken
 *
 * @return float
 */
float SoilMoisture::extractValue(float values[], size_t key, size_t max)
{
    switch (key)
    {
    case vwc:
        return multiplyValue(utils.getMedian(values, max));
    default:
        return utils.getMedian(values, max);
    }
}

/**
 * @private
 *
 * extractValue
 *
 * Applies any specific action or function to a specific parameter. Overloaded
 * as wrapper to extractValue above.
 *
 * @param float values[] - the values of the param type
 * @param size_t key - the integer value of the param
 *
 * @return float
 */
float SoilMoisture::extractValue(float values[], size_t key)
{
    size_t MAX = readSize();
    return extractValue(values, key, MAX);
}

// /**
//  * @private
//  *
//  * paramName
//  *
//  * Converts the parameter value into the string value of the
//  * param name
//  *
//  * @param size_t key - the integer value of the param
//  *
//  * @return String
//  */
// String SoilMoisture::paramName(size_t index)
// {
//     String param = valueMap[index];
//     if (param.equals(NULL) || param.equals(" ") || param.equals(""))
//     {
//         param = "_FAILURE_";
//     }
//     else if (this->hasSerialIdentity())
//     {
//         param += "_" + String(sendIdentity);
//     }
//     return param;
// }

// /**
//  * @private
//  *
//  * readReady
//  *
//  * Are we ready for another reading? I everything from the prior
//  * reading event done?
//  *
//  * @return bool
//  */
// bool SoilMoisture::readReady()
// {
//     if (!waitFor(SerialStorage::notSendingOfflineData, 1000))
//     {
//         return SerialStorage::notSendingOfflineData();
//     }
//     unsigned int size = boots->getReadTime() / 1000;
//     size_t skip = utils.skipMultiple(size, boots->getMaxVal(), READ_THRESHOLD);
//     return readAttempt >= skip;
// }

// /**
//  * @private
//  *
//  * readSize
//  *
//  * We do not read every read attemt. How are we breaking down the reading index
//  * based on the bootstrapped configuration intervals
//  *
//  * @return size_t
//  */
// size_t SoilMoisture::readSize()
// {
//     unsigned int size = boots->getReadTime() / 1000;
//     size_t skip = utils.skipMultiple(size, boots->getMaxVal(), READ_THRESHOLD);
//     size_t expand = floor(boots->getMaxVal() / skip);
//     return expand;
// }

// /**
//  * @private
//  *
//  * serialResponseIdentity
//  *
//  * Required to pull data off the serial bus. The only way to know the data payload
//  * is intended for this specific instance is through it's ID
//  *
//  * @return String
//  */
// String SoilMoisture::serialResponseIdentity()
// {
//     return utils.receiveDeviceId(this->sendIdentity);
// }

// /**
//  * @private
//  *
//  * constrictSerialIdentity
//  *
//  * Required to send data over the serial bus. The only way to know the data payload
//  * is intended for this specific instance is through it's ID
//  *
//  * @return String
//  */
// String SoilMoisture::constrictSerialIdentity()
// {
//     return utils.requestDeviceId(this->sendIdentity, serialMsgStr);
// }

// /**
//  * @private
//  *
//  * getReadContent
//  *
//  * If there is an integer ID, wrap it with the serial command we want to execute
//  *
//  * @return String
//  */
// String SoilMoisture::getReadContent()
// {
//     if (this->hasSerialIdentity())
//     {
//         return constrictSerialIdentity();
//     }

//     return serialMsgStr;
// }

// /**
//  * @private
//  *
//  * fetchReading
//  *
//  * Runs off the loop looking for a playload that matches it's identity
//  *
//  * @return String
//  */
// String SoilMoisture::fetchReading()
// {
//     if (!readCompile)
//     {
//         return boots->fetchSerial(this->serialResponseIdentity());
//     }
//     return "";
// }

// /**
//  * @private
//  *
//  * hasSerialIdentity
//  *
//  * Does the device have a numerical identity?
//  *
//  * @return bool
//  */
// bool SoilMoisture::hasSerialIdentity()
// {
//     return this->sendIdentity > -1;
// // }

// /**
//  * @private
//  *
//  * hasSerialIdentity
//  *
//  * Does the device have a numerical identity?
//  *
//  * @return bool
//  */
// String SoilMoisture::replaceSerialResponceItem(String message)
// {
//     if (!this->hasSerialIdentity())
//     {
//         return message;
//     }
//     String replaced = message.replace(this->serialResponseIdentity() + " ", "");
//     return replaced;
// }

// /**
//  * @private
//  *
//  * parseSerial
//  *
//  * Parses the serial data intended for this device
//  *
//  * @return void
//  */
// void SoilMoisture::parseSerial(String ourReading)
// {

//     if (utils.inValidMessageString(ourReading, this->sendIdentity))
//     {
//         Utils::log("SOIL_MOSTURE", "Invalid Message String");
//         return;
//     }

//     ourReading = replaceSerialResponceItem(ourReading);

//     readCompile = true;
//     // utils.parseSerial(ourReading, PARAM_LENGTH, boots->getMaxVal(), VALUE_HOLD);
//     readCompile = false;
// }

// /**
//  * @private
//  *
//  * uniqueName
//  *
//  * What's it's unique name for cloud functions
//  *
//  * @return String
//  */
// String SoilMoisture::uniqueName()
// {
//     if (this->hasSerialIdentity())
//     {
//         return this->name() + String(this->sendIdentity);
//     }
//     return this->name();
// }

/**
 * @private
 *
 * pullEpromData
 *
 * Pulls the config data from EEPROM
 *
 * @return void
 */
void SoilMoisture::pullEpromData()
{
    VWCStruct pulled;
    EEPROM.get(saveAddressForMoisture, pulled);
    if (pulled.version == 1 && !isnan(pulled.multiple))
    {
        multiple = pulled.multiple;
    }
    if (pulled.version == 1 && !isnan(pulled.minerals))
    {
        mineral_soil = pulled.minerals ? true : false;
    }
    Utils::log("SOIL_MOISTURE_BOOTSTRAP_MULTIPLIER", String(mineral_soil));
}

/**
 * @private
 *
 * setDeviceAddress
 *
 * Pulls the device configuration address from bootstrap
 *
 * @return void
 */
void SoilMoisture::setDeviceAddress()
{
    saveAddressForMoisture = boots->registerAddress(this->uniqueName(), sizeof(VWCStruct));
    Utils::log("SOIL_MOISTURE_BOOTSTRAP_ADDRESS", String(saveAddressForMoisture));
}

/**
 * @private
 *
 * setFunctions
 *
 * Binds the cloud functions for this device instance
 *
 * @return void
 */
void SoilMoisture::setFunctions()
{

    Particle.function("set" + this->uniqueName(), &SoilMoisture::setMoistureCalibration, this);
    Particle.function("setMineral" + this->uniqueName(), &SoilMoisture::setMineralSoilCalibration, this);
    Particle.variable(this->uniqueName(), multiple);
    Particle.variable("mineral" + this->uniqueName(), mineral_soil);
}

/**
 * @public
 *
 * init
 *
 * The devices startup function
 *
 * @return void
 */
void SoilMoisture::init()
{
    boots->startSerial();
    setDeviceAddress();
    pullEpromData();
    setFunctions();
}

// /**
//  * @public
//  *
//  * print
//  *
//  * Called during a print event
//  *
//  * @return void
//  */
// void SoilMoisture::print()
// {
//     for (size_t i = 0; i < PARAM_LENGTH; i++)
//     {
//         for (size_t j = 0; j < readSize(); j++)
//         {
//             //  Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(valueMap[i]), j, VALUE_HOLD[i][j]);
//         }
//     }
// }

// /**
//  * @public
//  *
//  * buffSize
//  *
//  * Number of bytes required for this device's payload
//  *
//  * @return size_t
//  */
// size_t SoilMoisture::buffSize()
// {
//     return 75;
// }

// /**
//  * @public
//  *
//  * paramCount
//  *
//  * How many parameters are required for this device
//  *
//  * @return uint8_t
//  */
// uint8_t SoilMoisture::paramCount()
// {
//     return PARAM_LENGTH;
// }

// /**
//  * @public
//  *
//  * maintenanceCount
//  *
//  * Number of params with invalid data
//  *
//  * @return uint8_t
//  */
// uint8_t SoilMoisture::maintenanceCount()
// {
//     uint8_t maintenance = this->maintenanceTick;
//     maintenanceTick = 0;
//     return maintenance;
// }

// /**
//  * @public
//  *
//  * read
//  *
//  * Executed during a reading event
//  *
//  * @return void
//  */
// void SoilMoisture::read()
// {
//     readAttempt++;
//     if (!readReady())
//     {
//         return;
//     }
//     readAttempt = 0;
//     String content = getReadContent();
//     // Utils::log("SERIAL_SEND_EVENT", content);
//     Serial1.println(content);
//     delay(100);
//     Serial1.flush();
// }

// /**
//  * @public
//  *
//  * read
//  *
//  * Executed during a loop event
//  *
//  * @return void
//  */
// void SoilMoisture::loop()
// {
//     String completedSerialItem = boots->fetchSerial(this->serialResponseIdentity());
//     if (!completedSerialItem.equals(""))
//     {
//         parseSerial(completedSerialItem);
//     }
// }

// /**
//  * @public
//  *
//  * read
//  *
//  * Executed during a clear event
//  *
//  * @return void
//  */
// void SoilMoisture::clear()
// {
//     for (size_t i = 0; i < SoilMoisture::PARAM_LENGTH; i++)
//     {
//         for (size_t j = 0; j < boots->getMaxVal(); j++)
//         {
//             VALUE_HOLD[i][j] = NO_VALUE;
//         }
//     }
// }

// /**
//  * @public
//  *
//  * publish
//  *
//  * Called when the device manager wants to publish the content
//  *
//  * @param JSONBufferWriter  &writer - the json object class
//  * @param uint8_t attempt_count - the number of attempts when reading made
//  * when reading
//  *
//  * @return void
//  */
// void SoilMoisture::publish(JSONBufferWriter &writer, uint8_t attempt_count)
// {
//     print();
//     size_t MAX = readSize();
//     for (size_t i = 0; i < SoilMoisture::PARAM_LENGTH; i++)
//     {
//         String param = paramName(i);
//         float paramValue = extractValue(VALUE_HOLD[i], i, MAX);
//         if (isnan(paramValue))
//         {
//             paramValue = NO_VALUE;
//         }

//         if (paramValue == NO_VALUE)
//         {
//             maintenanceTick++;
//         }
//         writer.name(param.c_str()).value(paramValue);
//     }
// }

/**
 * @public
 *
 * restoreDefaults
 *
 * Restores the default configuration
 *
 * @return void
 */
void SoilMoisture::restoreDefaults()
{
    mineral_soil = MINERAL_SOIL_DEFAULT;
    VWCStruct store = {1, mineral_soil};
    EEPROM.put(saveAddressForMoisture, store);
}