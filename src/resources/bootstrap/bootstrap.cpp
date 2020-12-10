#include "bootstrap.h"

bool publishHeartbeat = false;
bool readReleased = false;
bool publishReleased = false;
bool staticBootstrapped = false;

unsigned int TIMER_STALL = 60000;

uint32_t freememLast = 0;

void releaseRead()
{
    readReleased = true;
}

void releasePublishRead()
{
    publishReleased = true;
}

void releaseHeartbeat()
{
    publishHeartbeat = true;
}

void printMemory()
{
    uint32_t freemem = System.freeMemory();
    int delta = (int)freememLast - (int)freemem;
    Log.info("MEMORY CHANGE current: %lu, last %lu, delta: %d", freemem, freememLast, delta);
    freememLast = freemem;
}

Timer readtimer(TIMER_STALL, releaseRead);
Timer publishtimer(TIMER_STALL, releasePublishRead);
Timer heartBeatTimer(TIMER_STALL, releaseHeartbeat);

Timer memoryPrinter(10000, printMemory);

Bootstrap::~Bootstrap()
{
    //this->MAX_VALUE_THRESHOLD = (size_t)MAX_SEND_TIME;
    this->digital = DIGITAL_DEFAULT;
    this->publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
    this->READ_TIMER = (MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
    this->PUBLISH_TIMER = publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND;
    this->BEAT_TIMER = HEARTBEAT_TIMER;
    this->currentCalibration = DIGITAL_DEFAULT ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
}

void Bootstrap::init()
{
    this->batteryController();
    memoryPrinter.start();
    Time.zone(TIMEZONE);
    Particle.syncTime();
    Particle.variable("digital", digital);
    Particle.variable("publicationInterval", publicationIntervalInMinutes);
    Particle.variable("currentCalibration", currentCalibration);
    this->bootstrap();
}

bool Bootstrap::hasMaintenance()
{
    return this->maintenaceMode;
}

void Bootstrap::setMaintenance(bool maintain)
{
    this->maintenaceMode = maintain;
}

bool Bootstrap::isDigital()
{
    return this->digital;
}

bool Bootstrap::publishTimerFunc()
{
    return publishReleased;
}

bool Bootstrap::heatbeatTimerFunc()
{
    return publishHeartbeat;
}
bool Bootstrap::readTimerFun()
{
    return readReleased;
}

void Bootstrap::setPublishTimer(bool time)
{
    publishReleased = time;
}

void Bootstrap::setHeatbeatTimer(bool time)
{
    publishHeartbeat = time;
}
void Bootstrap::setReadTimer(bool time)
{
    readReleased = time;
}

bool Bootstrap::isStrapped()
{
    return this->bootstrapped;
}

size_t Bootstrap::getMaxVal()
{
    return this->MAX_VALUE_THRESHOLD;
}

void Bootstrap::buildSendInterval(int interval)
{
    publicationIntervalInMinutes = interval;
    READ_TIMER = (MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
    PUBLISH_TIMER = publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND;
    Log.info("MY READ TIMER IS SET FOR %d and the publish time is %d with interval %d", READ_TIMER, PUBLISH_TIMER, interval);

    if (readtimer.isActive())
    {
        readtimer.stop();
    }
    if (publishtimer.isActive())
    {
        publishtimer.stop();
    }
    readtimer.changePeriod(READ_TIMER);
    publishtimer.changePeriod(PUBLISH_TIMER);
    readtimer.start();
    publishtimer.start();

    EpromStruct config = getsavedConfig();
    config.pub = publicationIntervalInMinutes;
    putSavedConfig(config);
}

void Bootstrap::setDigital(bool digital)
{
    EpromStruct config = getsavedConfig();
    this->digital = digital;
    config.digital = digitalChar(digital);
    putSavedConfig(config);
}

double Bootstrap::getCalibration()
{
    return this->currentCalibration;
}
/*
* setCalibration: cloud function that calibration value
*/
void Bootstrap::setCalibration(double val)
{
    EpromStruct config = getsavedConfig();
    config.calibration = val;
    this->currentCalibration = val;
    this->putSavedConfig(config);
}

EpromStruct Bootstrap::getsavedConfig()
{
    EpromStruct values;
    EEPROM.get(EPROM_ADDRESS, values);
    if (values.version != 0)
    {
        EpromStruct defObject = {2, 1, currentCalibration, '!'};
        values = defObject;
    }
    return values;
}
/*
* putSavedConfig: Stores our config struct 
*/
void Bootstrap::putSavedConfig(EpromStruct config)
{
    config.version = 0;
    EEPROM.clear();
    Log.info("PUTTING version %d publish: %d, digital: %d", config.version, config.pub, config.digital);
    EEPROM.put(EPROM_ADDRESS, config);
}

void Bootstrap::bootstrap()
{
    delay(5000);
    // uncomment if you need to clear eeprom on load
    // EEPROM.clear();
    EpromStruct values = getsavedConfig();
    Log.info("BOOTSTRAPPING version %d publish: %d, digital: %d", values.version, values.pub, values.digital);
    // a default version is 2 or 0 when instantiated.
    buildSendInterval(values.pub);
    digital = isDigital(values.digital);
    const double calibration = values.calibration;
    currentCalibration = calibration;
    bootstrapped = true;
    staticBootstrapped = true;
}
/*
* restoreDefaults: cloud function that clears all config values
*/
void Bootstrap::restoreDefaults()
{
    EEPROM.clear();
    buildSendInterval(DEFAULT_PUB_INTERVAL);
    digital = DIGITAL_DEFAULT;
    if (digital)
    {
        currentCalibration = DEF_DISTANCE_READ_AN_CALIBRATION;
    }
    else
    {
        currentCalibration = DEF_DISTANCE_READ_AN_CALIBRATION;
    }

    EpromStruct defObject = {2, (uint8_t)publicationIntervalInMinutes, currentCalibration, '!'};
    putSavedConfig(defObject);
}

void Bootstrap::batteryController()
{
    PMIC pmic;
    pmic.begin();
    pmic.disableCharging();
}

void Bootstrap::timers()
{
    if (!readtimer.isActive())
    {
        readtimer.start();
    }
    if (!publishtimer.isActive())
    {
        publishtimer.start();
    }

    if (!heartBeatTimer.isActive())
    {
        heartBeatTimer.start();
    }
}
/*
* We represent the digital bool as y or n so as to 
* have a value for the default
*/
char Bootstrap::digitalChar(bool value)
{
    return value ? 'y' : 'n';
}

/*
* isDigital : Tells us if a config value is digital
*/
bool Bootstrap::isDigital(char value)
{
    if (value == 'y')
    {
        return true;
    }
    else if (value == 'n')
    {
        return false;
    }
    else
    {
        return DIGITAL_DEFAULT;
    }
}