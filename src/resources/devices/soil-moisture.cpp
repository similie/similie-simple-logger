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

/**
 *
 * @brief applySoilMoistureEquation
 *
 * Processes the equation for soil calibration
 *
 * @param value
 * @return float
 */
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