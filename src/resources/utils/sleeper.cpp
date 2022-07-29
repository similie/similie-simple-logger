#include "sleeper.h"
#include "utils.h"
Sleeper::Sleeper()
{
}

Sleeper::~Sleeper()
{
}

/**
 * @brief pulls the fule values from the device board
 *
 * @return float
 */
float Sleeper::getVCell()
{
    return fuel.getVCell();
}

bool Sleeper::checkSleepValues()
{
    if (needsSleep())
    {
        return goodNight();
    }
    return false;
}

/**
 * @brief checks the delta between the current hour and the shift thresholds
 *  for a given timeband.
 *
 * @param currentHour
 * @param shift
 * @return u_int8_t
 */
u_int8_t Sleeper::getShiftDelta(int currentHour, u_int8_t shift)
{
    if (shift > currentHour)
    {
        return 0;
    }

    return currentHour - shift;
}

/**
 * @brief once we have a timeshift what are the number of hours we should
 *  sleep before trying again
 *
 * @param deltaShift
 * @param span
 * @return u_int8_t
 */
u_int8_t Sleeper::hourShift(u_int8_t deltaShift, u_int8_t span)
{
    if (deltaShift > span)
    {
        return 0;
    }
    return span - deltaShift;
}

/**
 * @brief we are looking for the number of hours we should sleep before trying again
 *
 * @param int currentHour
 * @return unsigned long
 */
unsigned long Sleeper::timeCalToSunRise(int currentHour)
{

    /**
     * |_0_|_6_|_12_|_18_| Check Intervals
     * | 12| 6 |  6 | 18 | Wait Time for values greater than or equal
     * Delta to that time interval
     */

    if (currentHour >= EVENING)
    {
        return getHours(hourShift(getShiftDelta(currentHour, EVENING), NIGHT_SHIFT));
    }
    else if (currentHour >= NOON)
    {
        return getHours(hourShift(getShiftDelta(currentHour, NOON), AFTERNOON_BUMP));
    }
    else if (currentHour >= EARLY_SUNRISE)
    {
        return getHours(hourShift(getShiftDelta(currentHour, EARLY_SUNRISE), AFTERNOON_BUMP));
    }
    // here we are past midnight
    else
    {
        return getHours(hourShift(getShiftDelta(currentHour, MIDNIGHT), MORNING_SHIFT));
    }

    return getHours(12);
}

/**
 * @brief If a device goes to sleep we want it to wake when there is the
 * appropriate solar radiation.
 *
 * @return unsigned long
 */
unsigned long Sleeper::timeToSunRise()
{

    if (!Time.isValid())
    {
        return 0;
    }
    return timeCalToSunRise(Time.hour());
}

/**
 * @brief gets the number of minutes in miliseconds from an integer value
 * @note mainly used for testing. Realworld uses hours
 *
 * @param minutes
 * @return unsigned long
 */
unsigned long Sleeper::getMinutes(u_int8_t minutes)
{
    return MINUTE_BASE * minutes;
}

/**
 * @brief get the number of house in miliseconds from an integer value
 *
 * @param hours
 * @return unsigned long
 */
unsigned long Sleeper::getHours(u_int8_t hours)
{
    return HOUR_BASE * hours;
}

/**
 * @brief wrapping function to get how long we are going to sleep
 *
 * @return unsigned long
 */
unsigned long Sleeper::getDuration()
{
    // test
    if (DEBUG)
    {
        return getMinutes(1);
    }

    unsigned long sunRiseTime = timeToSunRise();
    if (sunRiseTime != 0)
    {
        return sunRiseTime;
    }
    // default to 8 hours
    return getHours(8);
}

/**
 * @brief Simple sleep config with the applied duration
 *
 * @return SystemSleepConfiguration
 */
SystemSleepConfiguration Sleeper::sleepConfig()
{
    unsigned long duration = getDuration();
    Utils::log("APPLYING_SLEEP_DURRATION", "HOURS: " + String(duration / HOUR_BASE));
    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(duration);
    return config;
}

/**
 * @brief checks to see if we need to trigger a sleep state
 *
 * @return true
 * @return false
 */
bool Sleeper::needsSleep()
{
    if (threshold == 0)
    {
        return false;
    }

    float volts = getVCell();
    if (volts > 0)
    {
        return getVCell() < (float)threshold;
    }
    return false;
}

/**
 * @brief Puts the device to sleep
 *
 * @return boolean
 */
bool Sleeper::goodNight()
{
    waitForTrue(&Sleeper::cryForHelp, this, 10000);
    // doens't seem to consistently send of we don't apply the delay
    delay(2000);
    Utils::log("SLEEP_THRESHOLD_VOLTAGE_VIOLATED", "VALUE:: " + String(threshold));
    System.sleep(sleepConfig());
    return true;
}

/**
 *
 * @brief public function that's run off the main loop
 *
 * @return true
 * @return false
 */
bool Sleeper::verify()
{ // now many cycles befor we check our voltage
    if (checkCycles < CYCLE_CHECK)
    {
        checkCycles++;
        return false;
    }
    checkCycles = 0;
    // return waitForTrue(&Sleeper::cryForHelp, this, 1000);
    // return testHourInterval();
    return checkSleepValues();
}

bool Sleeper::testHourInterval()
{
    for (u_int8_t i = 0; i < 24; i++)
    {
        unsigned long value = timeCalToSunRise(i);
        u_int8_t hours = value / HOUR_BASE;
        Serial.print("TESTING HOUR: ");
        Serial.print(i);
        Serial.print(" VALUE: ");
        Serial.print(value);
        Serial.print(" AS ");
        Serial.println(hours);
    }

    return true;
}

/**
 * @brief public function for setting our threshold
 *
 * @param value
 */
void Sleeper::set(double value)
{
    threshold = value;
}

/**
 * @brief public function for clearing our threshold
 *
 */
void Sleeper::clear()
{
    threshold = 0;
}

/**
 * @brief Waits for a specific time
 *
 * @todo refracture as generic Utils function. This is a copy from DeviceManager
 *
 * @param func
 * @param binding
 * @param time
 * @return true
 */
bool Sleeper::waitForTrue(bool (Sleeper::*func)(), Sleeper *binding, unsigned long time)
{
    bool valid = false;
    unsigned long then = millis();
    while (!valid && (millis() - time) < then)
    {
        valid = (binding->*func)();
    }
    return valid;
}

String Sleeper::getHelpMessage()
{
    char buf[150];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    writer.name("device").value(System.deviceID());
    writer.name("threshold").value(threshold);
    writer.name("volts").value(getVCell());
    writer.name("date").value(Time.timeStr());
    writer.name("for").value(getDuration() / HOUR_BASE);
    writer.endObject();
    return String(buf);
}
/**
 * @public
 *
 * cryForHelp
 * @todo: get the processor instance so that way we can
 *
 * Sends the payload over the network
 *
 * @param topic - the topic name
 * @param payload - the actual stringified data
 *
 * @return bool - true if successfull
 */
bool Sleeper::cryForHelp()
{
    if (Particle.connected())
    {
        bool success = Particle.publish(HELP_MESSAGE, getHelpMessage());
        return success;
    }
    return false;
}