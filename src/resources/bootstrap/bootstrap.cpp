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
    this->pullRegistration();
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
    this->serialInit();
}


size_t Bootstrap::epromSize() 
{
    return EEPROM.length()-1;
}

uint16_t Bootstrap::deviceInitAddress() 
{
    return this->deviceStartAddress + sizeof(EpromStruct) + 1;
}

uint16_t Bootstrap::getNextDeviceAddress() {
    uint16_t start  = deviceInitAddress();
    return 0;
}

void Bootstrap::pullRegistration() 
{
  uint16_t start = this->deviceInitAddress();
}

void addNewDeviceToStructure(DeviceStruct device) 
{

}

DeviceStruct Bootstrap::getDeviceByName(String name)
{
    for (size_t i = 0; i < MAX_DEVICES; i++) {
      DeviceStruct device = this->devices[i];
      String dName = String(device.name);
      if (dName.equals(name)) {
          return device;
      }
    }
    // now build it
    uint16_t next = getNextDeviceAddress();
    DeviceStruct device = {};
    this->addNewDeviceToStructure(device);
    //this->devices.push(device);
    return device;
}

uint16_t Bootstrap::registerAddress(String name) 
{
    return 0;
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

int Bootstrap::getStorageAddress(size_t size) 
{
    int send = this->nextAddress;
    this->nextAddress += size + 8;
    return send;
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
    processSerial();

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


/**
 * Serial Controller Elements
 */

/** 
 * @private serialInit
 * 
 * Starts the Serial1 litener
 * 
*/
void Bootstrap::serialInit() {
   // SERIAL_COMMS_BAUD
   if (!SERIAL_COMMS_BAUD) {
       return;
   }

   Serial1.begin(SERIAL_COMMS_BAUD);
}

/** 
 * @private serialBuilder
 * 
 * Reads content off the serial bus 
 * 
*/
int Bootstrap::serialBuilder() {
  int avail = Serial1.available();
    for (int i = 0; i < avail; i++)
    {
        char inChar = Serial1.read();
        //Serial.print(inChar);
        if (inChar == '\n' || inChar == '\0')
        {
            return 1;
        }
        if (inChar != '\r')
        {
            serialReadContent += String(inChar);
        }
    }
    return 0;
}

/** 
 * @private checkHasId
 * 
 * Checks to see if the payload is preppended with an ID tag
 * @return boolean true if it has an id tags
 * 
 * 
*/

bool Bootstrap::checkHasId() {
    //Log.info(serialReadContent);
    return serialReadContent.indexOf(" ") != -1;
}

/** 
 * @private processSerial
 * 
 * Runs the serial Loop
 * 
*/
void Bootstrap::processSerial() 
{
    if (!SERIAL_COMMS_BAUD) {
       return;
    }
    
    if (serialBuilder() && checkHasId()) {
        // now store
        Serial.println(serialReadContent);
        storeSerialContent();
        serialReadContent = "";
    }
}


/** 
 * @private storeSerialContent
 * 
 *  Stores and verifies fully processed serial element
 * 
*/
void Bootstrap::storeSerialContent() 
{  
    
    if (serialStoreIndex >= serial_buffer_length) {
        serialStoreIndex = 0;
    } else {
        serialStoreIndex++;
    }
    pushSerial(serialReadContent);
    serialReadContent = "";
}

/** 
 * @private pushSerial
 * 
 *  Pushes a fully processed serial element
 * @param String serial string to store
 * 
*/
void Bootstrap::pushSerial(String serial) 
{
    serial_buffer[serialStoreIndex] = serial;
}

/** 
 * @private pushSerial
 * 
 *  Finds a serial based on a specfic ID
 * @param String serial ID
 * 
*/
String Bootstrap::popSerial(size_t index) 
{

    String send = serial_buffer[serialStoreIndex] ;
    serial_buffer[serialStoreIndex] = "";
    return send;
}

/** 
 * @private isCorrectIdentity
 * 
 *  Finds a serial based on a specfic ID
 * @param String identity serial ID
 * @param size_t index to check
 * 
 * @return true if the idenity matches the string index
 * 
*/
bool Bootstrap::isCorrectIdentity(String identity, size_t index) {
    String check = serial_buffer[serialStoreIndex];
    return !check.equals("") && check.startsWith(identity);
}

/** 
 * @private indexCounter
 * 
 *  Finds a serial based on a specfic ID
 * @param size_t start index to check
 * 
 * @return size_t of the index is 
 * 
*/
size_t Bootstrap::indexCounter(size_t startIndex) {
    if (startIndex >= serial_buffer_length) {
       return startIndex % serial_buffer_length;
    } else {
       return startIndex;
    }
}



/**
 * @public fetchSerial 
 * 
 * Fetches the last requested item from a device
 * @param String idenity of the device requesting at payload 
 * 
 * */
String Bootstrap::fetchSerial(String identity) 
{
    size_t i = 0;
    size_t j = indexCounter(serialStoreIndex);
    while (i < serial_buffer_length) {
        if (isCorrectIdentity(identity, j)) {
            return popSerial(j);
        }
        i++;
        j = indexCounter(serialStoreIndex + i);
    }
    return "";
}