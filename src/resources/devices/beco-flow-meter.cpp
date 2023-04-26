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
    clear();
}

BecoFlowMeter::BecoFlowMeter(Bootstrap *boots, int sendIdentity)
{
    pulseCount = 0;
    this->boots = boots;
    this->sendIdentity = sendIdentity;
    clear();
}

BecoFlowMeter::BecoFlowMeter(Bootstrap *boots, int sendIdentity, int readPin)
{
    pulseCount = 0;
    this->flowPin = readPin;
    this->boots = boots;
    this->sendIdentity = sendIdentity;
    clear();
}

String BecoFlowMeter::name()
{
    return this->deviceName;
}

void BecoFlowMeter::restoreDefaults()
{
}

void BecoFlowMeter::init()
{
    instantated = true;
    setDeviceAddress();
    setLifeTimeFlow();
    setConfiguration();
    setListeners();
    configurePin();
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
            writer.name(utils.stringConvert(param)).value(currentFlow);
            break;
        case t_flow:
            writer.name(utils.stringConvert(param)).value(totalMilliLitres);
            break;
        }
    }

    setFlow();
    // setInterrupt();
    // String append = appendIdentity();
    // Utils::log("SENDIND_FLOW_DATA_ML", String::format("Identity=%s has total %.6f", append, totalMilliLitres));
}

void BecoFlowMeter::read()
{
}

void BecoFlowMeter::loop()
{
}

void BecoFlowMeter::clear()
{
    // pulseCount = 0;
    // flowRate = 0.0;
    // flowMilliLitres = 0;
    // totalMilliLitres = 0;
    // lastTime = 0;
    currentFlow = 0;
    // setInterrupt();
}

void BecoFlowMeter::print()
{
    // Print the flow rate for this second in litres / minute

    Serial.print("Flow rate: ");
    Serial.print(flowRate); // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t"); // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(currentFlow);
    Serial.print("mL");

    Serial.print("\t"); // Print tab space
    Serial.print(totalMilliLitres);
    Serial.print("mL");

    Serial.print("\t"); // Print tab space
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
}

size_t BecoFlowMeter::buffSize()
{
    return 60;
}

uint8_t BecoFlowMeter::paramCount()
{
    return 4;
}

uint8_t BecoFlowMeter::matenanceCount()
{
    return 0;
}

void BecoFlowMeter::setPin(int pin)
{
    flowPin = pin;
}

void BecoFlowMeter::configurePin()
{
    pinMode(getPin(), INPUT_PULLUP);
    // digitalWrite(getPin(), HIGH);
    setInterrupt();
}

///////////////////////////
/// Privates
///////////////////////////

void BecoFlowMeter::setConfiguration()
{
    BecoFlowStruct storage = getProm();
    if (!Utils::validConfigIdentity(storage.version))
    {
        return;
    }

    if (storage.calibrationFactor != 0 && storage.calibrationFactor < 200 && !isnan(storage.calibrationFactor))
    {
        calibrationFactor = storage.calibrationFactor;
    }

    if (storage.startingPosition != 0 && storage.totalMilliLitres < storage.startingPosition && !isnan(storage.startingPosition))
    {
        startingPosition = storage.startingPosition;
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
 *
 *
 */
void BecoFlowMeter::setInterrupt()
{
    if (!instantated)
    {
        return;
    }
    attachInterrupt(getPin(), &BecoFlowMeter::pulseCounter, this, FALLING);
}

// bool BecoFlowMeter::isDisconnected()
// {
//     return pulseCount >= 12;
// }

void BecoFlowMeter::setTotal()
{
    if (totalMilliLitres < startingPosition)
    {
        totalMilliLitres = startingPosition;
    }
    totalMilliLitres += currentFlow;
}

void BecoFlowMeter::processRead()
{
    currentFlow = pulseCount;
    pulseCount = 0;
    setTotal();
    // if ((millis() - lastTime) > 1000) // Only process counters once per second
    // {
    // if (isDisconnected())
    // {
    //     lastTime = millis();
    //     String append = appendIdentity();
    //     Utils::log("FLOW_METER_DISCONNECT", append + " " + String(pulseCount));
    //     pulseCount = 0;
    //     return;
    // }

    // removeInterrupt();
    // // Because this loop may not complete in exactly 1 second intervals we calculate
    // // the number of milliseconds that have passed since the last execution and use
    // // that to scale the output. We also apply the calibrationFactor to scale the output
    // // based on the number of pulses per second per units of measure (litres/minute in
    // // this case) coming from the sensor.
    // float tmp = (1000.0 / (millis() - lastTime)) * pulseCount;
    // countIteration++;
    // Serial.print("ITERATION: ");
    // Serial.print(countIteration); // Print the integer part of the variable
    // Serial.print(" ");
    // Serial.print("\t"); // Print tab space

    // Serial.print("PULSE: ");
    // Serial.print(pulseCount); // Print the integer part of the variable
    // Serial.print(" ");
    // Serial.print("\t"); // Print tab space

    // Serial.print("TMP: ");
    // Serial.print(tmp); // Print the integer part of the variable
    // Serial.print(" ");
    // Serial.print("\t"); // Print tab space

    // // As we have a add operation we need to avoid the positive value when no pulse is found
    // if (tmp > 0)
    // {
    //     flowRate = tmp / calibrationFactor + calibrationDifference;
    // }
    // else
    // {
    //     flowRate = 0;
    // }

    // // Note the time this processing pass was executed. Note that because we've
    // // disabled interrupts the millis() function won't actually be incrementing right
    // // at this point, but it will still return the value it was set to just before
    // // interrupts went away.
    // lastTime = millis();
    // // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // // convert to millilitres.
    // flowMilliLitres = (flowRate / 60) * 1000;

    // currentFlow += flowMilliLitres;
    // // Add the millilitres passed in this second to the cumulative total
    // totalMilliLitres += flowMilliLitres;

    // print();
    // // this serial print was used to get the FREQUENCY value to calculate the other values.
    // // Serial.print("F: ");
    // // Serial.println(tmp);
    // // Reset the pulse counter so we can start incrementing again
    // pulseCount = 0;

    //     setInterrupt();
    // }
}

void BecoFlowMeter::removeInterrupt()
{
    detachInterrupt(getPin());
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
    totalMilliLitres = 0;
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
    double val = Utils::parseCloudFunctionDouble(read, uniqueName());
    if (val == 0)
    {
        return 0;
    }

    startingPosition = val;
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
    double val = Utils::parseCloudFunctionDouble(read, uniqueName());
    if (val == 0)
    {
        return 0;
    }

    calibrationFactor = val;

    BecoFlowStruct storage = getProm();
    storage.calibrationFactor = calibrationFactor;
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
int BecoFlowMeter::setCalibrationDifference(String read)
{
    double val = Utils::parseCloudFunctionDouble(read, uniqueName());
    if (val == 0)
    {
        return 0;
    }

    calibrationDifference = val;

    BecoFlowStruct storage = getProm();
    storage.calibrationDifference = calibrationDifference;
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

void BecoFlowMeter::setFlow()
{
    BecoFlowStruct storage = getProm();
    if (storage.totalMilliLitres == totalMilliLitres)
    {
        return;
    }
    storage.totalMilliLitres = totalMilliLitres;
    storage.calibrationDifference = calibrationDifference;
    storage.calibrationFactor = calibrationFactor;
    saveEEPROM(storage);
}

unsigned long BecoFlowMeter::getToltalFlowMiliLiters()
{
    BecoFlowStruct storage = getProm();
    if (!Utils::validConfigIdentity(storage.version))
    {
        return 0;
    }
    return storage.totalMilliLitres;
}

void BecoFlowMeter::setListeners()
{
    String appendage = appendIdentity();
    // Particle.function("setCalibrationFactor" + appendage, &BecoFlowMeter::setCalibrationFactor, this);
    // Particle.function("setCalibrationDifference" + appendage, &BecoFlowMeter::setCalibrationDifference, this);
    Particle.function("setStartingPosition" + appendage, &BecoFlowMeter::setStartingPosition, this);
    Particle.function("clearTotalFlow" + appendage, &BecoFlowMeter::clearTotalCount, this);

    // Particle.variable("getTotalFlow" + appendage, totalMilliLitres);
    // Particle.variable("getCalibrationFactor" + appendage, calibrationFactor);
    // Particle.variable("getCalibrationDifference" + appendage, calibrationDifference);
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
 *
 *
 */

void BecoFlowMeter::pulseCounter()
{
    pulseCount++;
}
