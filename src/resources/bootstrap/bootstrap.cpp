#include "bootstrap.h"
#include "resources/utils/utils.h"

bool publishHeartbeat = false;
bool readReleased = false;
bool publishReleased = false;
bool staticBootstrapped = false;

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

Timer publishtimer(Bootstrap::ONE_MINUTE, releasePublishRead);
Timer readtimer(Bootstrap::ONE_MINUTE, releaseRead);
Timer heartBeatTimer(Bootstrap::HEARTBEAT_TIMER, releaseHeartbeat);
Timer beachedTimer(Bootstrap::BEACH_TIMEOUT_RESTORE, Bootstrap::beachReset, true);

// Timer memoryPrinter(10000, printMemory);

Bootstrap::~Bootstrap()
{
    int interval = (int)publicationIntervalInMinutes;
    this->READ_TIMER = (MINUTE_IN_SECONDS * interval) / MAX_SEND_TIME * MILISECOND;
    this->PUBLISH_TIMER = interval * MINUTE_IN_SECONDS * MILISECOND;
    this->currentCalibration = digital ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
}


void Bootstrap::init()
{
    this->batteryController();
    // memoryPrinter.start();
    // Cellular.setCredentials("internet");

    Time.zone(TIMEZONE);
    Particle.syncTime();
    Particle.keepAlive(30);
    Particle.variable("digital", digital);
    Particle.variable("publicationInterval", publishedInterval);
    Particle.variable("currentCalibration", currentCalibration);

    beachedTimer.start();
    heartBeatTimer.start();
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

unsigned int Bootstrap::getPublishTime()
{
    return this->PUBLISH_TIMER;
}

unsigned int Bootstrap::getReadTime()
{
    return this->READ_TIMER;
}

void Bootstrap::buildSendInterval(int interval)
{
    strappingTimers = true;
    publicationIntervalInMinutes = (uint8_t)interval;
    publishedInterval = interval;
    this->READ_TIMER = (unsigned int)(MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
    this->PUBLISH_TIMER = (unsigned int)(publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND);

    if (readtimer.isActive())
    {
        readtimer.stop();
        readtimer.reset();
    }
    if (publishtimer.isActive())
    {
        publishtimer.stop();
        publishtimer.reset();
    }
    publishtimer.changePeriod(PUBLISH_TIMER);
    readtimer.changePeriod(READ_TIMER);
    readtimer.start();
    publishtimer.start();

    EpromStruct config = getsavedConfig();
    config.pub = publicationIntervalInMinutes;
    putSavedConfig(config);
    strappingTimers = false;
    Log.info("MY READ TIMER IS SET FOR %u and the publish time is %u with interval %d", READ_TIMER, PUBLISH_TIMER, interval);
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

void Bootstrap::resetBeachCount()
{
    beachReset();
}

void Bootstrap::beachReset()
{
    Log.info("BEACH RESET >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %u", Bootstrap::BEACH_ADDRESS);
    BeachStruct rebeach = {0, 0};
    EEPROM.put(Bootstrap::BEACH_ADDRESS, rebeach);
}

uint8_t Bootstrap::beachCount()
{
    BeachStruct beachAttempts;
    EEPROM.get(Bootstrap::BEACH_ADDRESS, beachAttempts);
    if (beachAttempts.version != 0)
    {
        beachReset();
        return 0;
    }
    return beachAttempts.count;
}

bool Bootstrap::isBeached()
{
    uint8_t bCount = beachCount();
    uint8_t beachedIncrement = bCount + 1;
    BeachStruct beachBase = {0, beachedIncrement};

    EEPROM.put(Bootstrap::BEACH_ADDRESS, beachBase);
    Log.info("GOT THIS BEACH COUNT %u of %u and increment %u", bCount, BEACHED_THRSHOLD, beachBase.count);
    return bCount >= BEACHED_THRSHOLD;
}
void Bootstrap::beach()
{

    u8_t fail = 0;
    u8_t FAIL_POINT = 4;

    int value = 0;
    char response[64] = "";
    Cellular.command(Utils::simCallback, response, BEACH_LISTEN_TIME, "AT+COPS=0,2\r\n");

    while (value != RESP_OK && fail < FAIL_POINT)
    {
        fail++;
        char response[64] = "";
        value = Cellular.command(Utils::simCallback, response, BEACH_LISTEN_TIME, "AT+COPS=6\r\n");
        Log.info("Beach resonse at %d at cycle %u of %u", value, fail, FAIL_POINT);
        if (value == RESP_OK)
        {
            break;
        }
        else
        {
            delay(1000);
        }
    }

    Log.info("BEACHED AS");
    Cellular.command("AT+COPS=0,2\r\n");
    resetBeachCount();
    delay(2000);
    // System.reset();
}

/*
* putSavedConfig: Stores our config struct 
*/
void Bootstrap::putSavedConfig(EpromStruct config)
{
    config.version = 0;
    // EEPROM.clear();
    Log.info("PUTTING version %d publish: %d, digital: %c", config.version, config.pub, config.digital);
    EEPROM.put(EPROM_ADDRESS, config);
}

void Bootstrap::bootstrap()
{
    // delay(5000);

    if (isBeached())
    {
        beach();
    }

    // uncomment if you need to clear eeprom on load
    // EEPROM.clear();

    EpromStruct values = getsavedConfig();
    Log.info("BOOTSTRAPPING version %d publish: %u, digital: %c", values.version, values.pub, values.digital);
    // a default version is 2 or 0 when instantiated.
    buildSendInterval((int)values.pub);
    //buildSendInterval(2);
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

    EpromStruct defObject = {2, publicationIntervalInMinutes, currentCalibration, '!'};
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

    // manageManualModel();

    if (strappingTimers)
    {
        return;
    }

    if (!beachedTimer.isActive())
    {
        beachedTimer.start();
    }

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