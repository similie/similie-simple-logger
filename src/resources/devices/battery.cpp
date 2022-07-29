#include "battery.h"
/**
 * @deconstructor
 */
Battery::~Battery()
{
}

/**
 * @constructor
 *
 * @parm Bootstrap * boots
 */
Battery::Battery(Bootstrap *boots)
{
}

/**
 * @constructor
 */
Battery::Battery()
{
}

/**
 * @public
 *
 * name
 *
 * Returns the device name
 * @return String
 */
String Battery::name()
{
    return deviceName;
}

/**
 * @public
 *
 * restoreDefaults
 *
 * Called when defaults should be restored
 *
 * @return void
 */
void Battery::restoreDefaults()
{
}

/**
 * @public
 *
 * publish
 *
 * Called during a publish event
 *
 * @return void
 */
void Battery::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
    writer.name(percentname).value(round(getNormalizedSoC()));
    writer.name(voltsname).value(getVCell());
}

float Battery::getNormalizedSoC()
{
    return fuel.getNormalizedSoC();
}

inline float Battery::batteryCharge()
{
    return System.batteryCharge();
}

float Battery::getVCell()
{
    return fuel.getVCell();
}

/**
 * @public
 *
 * read
 *
 * Called during a read event
 *
 * @return void
 */
void Battery::read()
{
}

/**
 * @public
 *
 * loop
 *
 * Called during a loop event
 *
 * @return void
 */
void Battery::loop()
{
}

/**
 * @public
 *
 * clear
 *
 * Called during a clear event
 *
 * @return void
 */
void Battery::clear()
{
}

/**
 * @public
 *
 * print
 *
 * Called during a print event
 *
 * @return void
 */
void Battery::print()
{
    Log.info("BATTERY POWER %.2f", System.batteryCharge());
}

/**
 * @public
 *
 * init
 *
 * Called at setup
 *
 * @return void
 */
void Battery::init()
{
}

/**
 * @public
 *
 * buffSize
 *
 * Returns the payload size the device requires for sending data
 *
 * @return size_t
 */
size_t Battery::buffSize()
{
    return 80;
}

/**
 * @public
 *
 * paramCount
 *
 * Returns the number of params returned
 *
 * @return uint8_t
 */
uint8_t Battery::paramCount()
{
    return PARAM_LENGTH;
}

/**
 * @public
 *
 * matenanceCount
 *
 * Is the device functional
 *
 * @return uint8_t
 */
uint8_t Battery::matenanceCount()
{
    return 0;
}
