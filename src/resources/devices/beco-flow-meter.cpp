#include "beco-flow-meter.h"

BecoFlowMeter::~BecoFlowMeter()
{
}

BecoFlowMeter::BecoFlowMeter()
{
    pulseCount = 0;
}

BecoFlowMeter::BecoFlowMeter(Bootstrap *boots)
{
    pulseCount = 0;
    this->boots = boots;
    resetAll();
}

BecoFlowMeter::BecoFlowMeter(Bootstrap *boots, int sendIdentity)
{
    pulseCount = 0;
    this->boots = boots;
    this->sendIdentity = sendIdentity;
    resetAll();
}

BecoFlowMeter::BecoFlowMeter(Bootstrap *boots, int sendIdentity, int readPin)
{
    pulseCount = 0;
    this->flowPin = readPin;
    this->boots = boots;
    this->sendIdentity = sendIdentity;
    resetAll();
}

String BecoFlowMeter::name()
{
    return this->deviceName;
}

void BecoFlowMeter::restoreDefaults()
{
    resetAll();
    BecoFlowStruct storage = getProm();
    storage.totalMilliLitres = 0;
    storage.startingPosition = 0;
    storage.calibrationFactor = CALIBRATION_FLOW_FACTOR_DEFAULT;
    storage.offsetMultiple = READ_OFFSET_SEND_MULTIPLE;
    saveEEPROM(storage);
}

void BecoFlowMeter::init()
{

    instantated = true;
    setDeviceAddress();
    setLifeTimeFlow();
    setConfiguration();
    setListeners();
    configurePin();
    //  delay(100);
    pulseReady = false;
}

void BecoFlowMeter::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
    processRead();
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        String param = getParamName(i);
        switch (i)
        {
        case c_flow:
            writer.name(utils.stringConvert(param)).value(currentFlow * offsetMultiple);
            break;
        case t_flow:
            writer.name(utils.stringConvert(param)).value(totalMilliLitres * offsetMultiple);
            break;
        }
    }
    // need to think about pulse count. We are executing during a syncronous process
    // this should be zero'd out before the loop is executed
    pulseCount = 0;
    currentFlow = 0;
    setFlow();
}

bool BecoFlowMeter::pulseReadCrossedDebounce()
{
    return millis() - debounce > READ_COUNT_DEBOUNCE + READ_COUNT_DEBOUNCE_SLOT_BUFFER;
}

bool BecoFlowMeter::pulseReadCrossedDebounceSlotBuffer()
{
    unsigned long span = millis() - debounce;
    return span >= READ_COUNT_DEBOUNCE && span <= READ_COUNT_DEBOUNCE + READ_COUNT_DEBOUNCE_SLOT_BUFFER;
}

bool BecoFlowMeter::pulseDebounceRead()
{

    if (debounce == 0)
    {
        return false;
    }

    u_int8_t val = digitalRead(getPin());
    return pulseReadCrossedDebounce() || val == HIGH;
}

void BecoFlowMeter::read()
{
}

void BecoFlowMeter::incrementRead()
{
    if (pulseReadCrossedDebounceSlotBuffer())
    {
        pulseCount++;
    }
    else
    {
        pulseCount = 0;
    }
    pulseReady = false;
    debounce = 0;
}

void BecoFlowMeter::loop()
{

    if (!pulseReady)
    {
        return;
    }

    if (debounce == 0)
    {
        debounce = millis();
    }

    if (pulseDebounceRead())
    {
        return incrementRead();
    }

    // Serial.print("BOUNCE TIME ");
    // Serial.print(millis() - debounce);
    // Serial.print(" HIGH ");
    // Serial.print(HIGH);
    // Serial.print(" LOW ");
    // Serial.print(LOW);
    // Serial.println(digitalRead(BECO_FLOW_PIN_DEFAULT));
    Utils::log("BECO_PULSE_PRINT", String(pulseCount));
}

void BecoFlowMeter::clear()
{
    currentFlow = 0;
    // pulseCount = 0;
    // debounce = 0;
    // pulseReady = false;
}

void BecoFlowMeter::print()
{
    Utils::log("BECO_FLOW_RATE", String("Output Liquid Quantity: ") + String(currentFlow) + String("mL\t") + String("Total FLow: ") + String(totalMilliLitres) + String("L"));
}

size_t BecoFlowMeter::buffSize()
{
    return 60;
}

uint8_t BecoFlowMeter::paramCount()
{
    return 2;
}

uint8_t BecoFlowMeter::maintenanceCount()
{
    return exceedsMax(totalMilliLitres) ? paramCount() : 0;
}

void BecoFlowMeter::setPin(int pin)
{
    flowPin = pin;
}

void BecoFlowMeter::configurePin()
{
    pinMode(getPin(), INPUT_PULLUP);
    delay(100);
    setInterrupt();
    debounce = 0;
}

///////////////////////////
/// Privates
///////////////////////////

/**
 * @brief resetAll - resets the values back to zero*
 */
void BecoFlowMeter::resetAll()
{
    currentFlow = 0;
    pulseCount = 0;
    debounce = 0;
    totalMilliLitres = 0;
    pulseReady = false;
}

/**
 * @brief exceedsMax
 * We ensure the value does not exceed the maximum value stored in eeprom
 *
 * @param value
 * @return boolean
 */
boolean BecoFlowMeter::exceedsMax(unsigned long value)
{
    return isnan(value) || value == UINT_MAX || value == ULLONG_MAX;
}

/**
 * @brief setConfiguration
 * builds the internal config on startup
 *
 */
void BecoFlowMeter::setConfiguration()
{
    BecoFlowStruct storage = getProm();
    if (!Utils::validConfigIdentity(storage.version))
    {
        return;
    }

    if (storage.calibrationFactor != 0 && storage.calibrationFactor < 200 && !exceedsMax(storage.calibrationFactor))
    {
        calibrationFactor = storage.calibrationFactor;
    }

    if (storage.startingPosition != 0 && storage.totalMilliLitres < storage.startingPosition && !exceedsMax(storage.startingPosition))
    {
        startingPosition = storage.startingPosition;
    }

    if (storage.offsetMultiple != 0 && !exceedsMax(storage.offsetMultiple))
    {
        offsetMultiple = storage.offsetMultiple;
    }
}

/**
 * @private
 *
 * getPin
 *
 * Gets the configured pin
 *
 * @return int
 */
int BecoFlowMeter::getPin()
{
    return this->flowPin;
}

void BecoFlowMeter::setLifeTimeFlow()
{
    totalMilliLitres = getToltalFlowMiliLiters();
}

/**
 * @private
 * @brief setInterrupt
 * applies the interrupt to the configured pin
 */
void BecoFlowMeter::setInterrupt()
{
    if (!instantated)
    {
        return;
    }
    attachInterrupt(getPin(), &BecoFlowMeter::pulseCounter, this, FALLING);
}

/**
 * @private
 * @brief setTotal
 * Determins if there is a comman to set the total value or not
 */
void BecoFlowMeter::setTotal()
{
    if (totalMilliLitres < startingPosition)
    {
        totalMilliLitres = startingPosition;
    }
    totalMilliLitres += currentFlow;
}

/**
 * @private
 * @brief processRead
 * Wrapper for processing a read-call event
 */
void BecoFlowMeter::processRead()
{
    currentFlow = pulseCount * calibrationFactor;
    pulseCount = 0;
    setTotal();
    print();
}

/**
 * @private
 *
 * getProm
 *
 * Returns the wl configuration structure
 *
 * @return WlDevice
 */
BecoFlowStruct BecoFlowMeter::getProm()
{
    BecoFlowStruct prom;
    if (saveAddressForFlow == -1)
    {
        return prom;
    }
    EEPROM.get(saveAddressForFlow, prom);
    return prom;
}

/**
 * @private
 *
 * clearTotalCount
 *
 * Cloud function for clearing to totalizer
 * @param String read - payload from the particle API
 *
 * @return int
 */
int BecoFlowMeter::clearTotalCount(String val)
{
    if (!val.equals("DELETE"))
    {
        return 0;
    }
    BecoFlowStruct flow = getProm();
    flow.totalMilliLitres = 0;
    flow.startingPosition = 0;
    saveEEPROM(flow);
    resetAll();
    return 1;
}

/**
 * @private
 *
 * setOffsetMultiple
 *
 * Cloud function for setting an offset value for the meter
 * @param String read - payload from the particle API
 *
 * @return int
 */
int BecoFlowMeter::setOffsetMultiple(String read)
{
    int val = Utils::parseCloudFunctionInteger(read, uniqueName());
    if (val <= 0)
    {
        return 0;
    }

    offsetMultiple = (unsigned long)val;
    BecoFlowStruct storage = getProm();
    storage.offsetMultiple = offsetMultiple;
    saveEEPROM(storage);
    return 1;
}

/**
 * @private
 *
 * setCalibration
 *
 * Cloud function for setting the starting value for the meter
 * @param String read - payload from the particle API
 *
 * @return int
 */
int BecoFlowMeter::setStartingPosition(String read)
{
    int val = Utils::parseCloudFunctionInteger(read, uniqueName());
    if (val <= 0)
    {
        return 0;
    }

    startingPosition = (unsigned long)val;
    BecoFlowStruct storage = getProm();
    storage.startingPosition = startingPosition;
    saveEEPROM(storage);
    return 1;
}
/**
 * @private
 *
 * setCalibration
 *
 * Cloud function for setting the calibration value
 * @param String read - payload from the particle API
 *
 * @return int
 */
int BecoFlowMeter::setCalibrationFactor(String read)
{
    int val = Utils::parseCloudFunctionInteger(read, uniqueName());
    if (val <= 0)
    {
        return 0;
    }

    calibrationFactor = (unsigned long)val;

    BecoFlowStruct storage = getProm();
    storage.calibrationFactor = calibrationFactor;
    saveEEPROM(storage);
    return 1;
}

/**
 * @private
 *
 * saveEEPROM
 *
 * @param WLStruct storage - the config payload needing storage
 *
 * @return void
 */
void BecoFlowMeter::saveEEPROM(BecoFlowStruct storage)
{
    if (saveAddressForFlow == -1)
    {
        return;
    }
    storage.version = 1;
    EEPROM.put(saveAddressForFlow, storage);
}

/**
 * @brief setFlow
 * Stores the total in ML into EEPROM
 */
void BecoFlowMeter::setFlow()
{
    BecoFlowStruct storage = getProm();
    if (storage.totalMilliLitres == totalMilliLitres || exceedsMax(totalMilliLitres))
    {
        return;
    }
    storage.totalMilliLitres = totalMilliLitres;
    // storage.calibrationFactor = calibrationFactor;
    saveEEPROM(storage);
}

/**
 * @brief getToltalFlowMiliLiters
 * resolves the total flow from the EEPROM storage on startup
 *
 * @return unsigned long
 */
unsigned long BecoFlowMeter::getToltalFlowMiliLiters()
{
    BecoFlowStruct storage = getProm();
    if (!Utils::validConfigIdentity(storage.version) || exceedsMax(storage.totalMilliLitres))
    {
        return 0;
    }
    return storage.totalMilliLitres;
}

/**
 * @brief setListeners
 * Appliles the cloud functions to the device
 */
void BecoFlowMeter::setListeners()
{
    String appendage = appendIdentity();
    Particle.function("setCalibrationFactor" + appendage, &BecoFlowMeter::setCalibrationFactor, this);
    Particle.function("setStartingPosition" + appendage, &BecoFlowMeter::setStartingPosition, this);
    Particle.function("setOffsetMultiple" + appendage, &BecoFlowMeter::setOffsetMultiple, this);
    Particle.function("clearTotalFlow" + appendage, &BecoFlowMeter::clearTotalCount, this);
    Particle.variable("getOffsetMultiple" + appendage, offsetMultiple);
    Particle.variable("getTotalFlow" + appendage, totalMilliLitres);
    Particle.variable("getCalibrationFactor" + appendage, calibrationFactor);
}

void BecoFlowMeter::setIdentity(int identity)
{
    sendIdentity = identity;
}

/**
 * @private
 *
 * appendIdentity
 *
 * If there is an integer identity, it concats the id as a string
 *
 * @return String
 */
String BecoFlowMeter::appendIdentity()
{
    return this->hasSerialIdentity() ? String(this->sendIdentity) : "";
}

/**
 * @private
 *
 * uniqueName
 *
 * If there is an integer identity, it concats the id with the name, otherwise
 * it simply returns the given name
 *
 * @return String
 */
String BecoFlowMeter::uniqueName()
{
    if (this->hasSerialIdentity())
    {
        return this->name() + String(this->sendIdentity);
    }
    return this->name();
}

String BecoFlowMeter::getParamName(size_t index)
{
    String param = valueMap[index];
    if (this->hasSerialIdentity())
    {
        return param + String::format("_%d", sendIdentity);
    }
    return param;
}

/**
 * @private
 *
 * restoreDefaults
 *
 * Restores the default values
 *
 * @return void
 */
bool BecoFlowMeter::hasSerialIdentity()
{
    return utils.hasSerialIdentity(this->sendIdentity);
}

void BecoFlowMeter::setDeviceAddress()
{
    saveAddressForFlow = boots->registerAddress(this->uniqueName(), sizeof(BecoFlowStruct));
    Utils::log("BECO_FLOW_METER_BOOTSTRAP_ADDRESS", String(saveAddressForFlow));
}

/**
 * @brief pulseCounter
 * Applies the ready value for when the pulse occurs.
 * Picked up in the loop function
 */
void BecoFlowMeter::pulseCounter()
{
    pulseReady = true;
}
