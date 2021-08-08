#include "bootstrap.h"
#include "resources/utils/utils.h"

bool publishHeartbeat = false;
bool readReleased = false;
bool publishReleased = false;
bool staticBootstrapped = false;
// for memory debugging
uint32_t freememLast = 0;
/**
* releaseRead
* 
* tells the loop that a read needs to occur
* @return void
*/
void releaseRead()
{
    readReleased = true;
}
/**
* releasePublishRead
* 
* tells the loop that a publish needs to occur
* @return void
*/
void releasePublishRead()
{
    publishReleased = true;
}
/*
* releasePublishRead
* 
* tells the loop that a heartbeat event needs to occur
* @return void
*/
void releaseHeartbeat()
{
    publishHeartbeat = true;
}
/**
* printMemory
* 
* Prints the available memory in console for device debugging
* @return void
*/
void printMemory()
{
    uint32_t freemem = System.freeMemory();
    int delta = (int)freememLast - (int)freemem;
    Log.info("MEMORY CHANGE current: %lu, last %lu, delta: %d", freemem, freememLast, delta);
    freememLast = freemem;
}
// timer setup. This are the heartbeat of the system. Triggers system events
Timer publishtimer(Bootstrap::ONE_MINUTE, releasePublishRead);
Timer readtimer(Bootstrap::ONE_MINUTE, releaseRead);
Timer heartBeatTimer(Bootstrap::HEARTBEAT_TIMER, releaseHeartbeat);
Timer beachedTimer(Bootstrap::BEACH_TIMEOUT_RESTORE, Bootstrap::beachReset, true);
// Timer memoryPrinter(10000, printMemory);
/**
* @constructor Bootstrap
*/
Bootstrap::~Bootstrap()
{
    int interval = (int)publicationIntervalInMinutes;
    this->READ_TIMER = (MINUTE_IN_SECONDS * interval) / MAX_SEND_TIME * MILISECOND;
    this->PUBLISH_TIMER = interval * MINUTE_IN_SECONDS * MILISECOND;
}

/**
* @public init
*
* Called during the setup of the primary application
* @return void
*/
void Bootstrap::init()
{
    setFunctions();
    this->setMetaAddresses();
    this->pullRegistration();
    this->batteryController();
    // memoryPrinter.start();
    // Cellular.setCredentials("internet");
    Time.zone(TIMEZONE);
    Particle.syncTime();
    Particle.keepAlive(30);
    Particle.variable("publicationInterval", publishedInterval);

    beachedTimer.start();
    heartBeatTimer.start();
    this->bootstrap();
    this->serialInit();
}

/**
* @private setMaintenanceMode
*
* Received cloud function maintenance mode request
* @param String read - the value from the cloud 
* @return int
*/
int Bootstrap::setMaintenanceMode(String read)
{
  int val = (int)atoi(read);
  if (val < 0 || val > 1)
  {
    return -1;
  }
  setMaintenance((bool)val);
  return val;
}
/**
* @private setFunctions
*
* setup for cloud functions
* @return void
*/
void Bootstrap::setFunctions() {
    Particle.function("setMaintenanceMode", &Bootstrap::setMaintenanceMode, this);
}
/**
* @public epromSize
*
* the total size of eprom memory
* @return size_t - total memory
*/
size_t Bootstrap::epromSize() 
{
    return EEPROM.length()-1;
}
/**
* @private deviceInitAddress
*
* this is the address to start storing device eprom data
* @return uint16_t - first address after the bootstrap required space
*/
uint16_t Bootstrap::deviceInitAddress() 
{
    return this->deviceStartAddress + 1;
}


/**
* @private exceedsMaxAddressSize
*
* Checks to see of the EEPROM address size is too excessive
* MAX_EEPROM_ADDRESS 8197 MAX_U16 65535
* @return bool 
*/
bool Bootstrap::exceedsMaxAddressSize(uint16_t address)
{
    return address > 0 && address < MAX_EEPROM_ADDRESS && address < Bootstrap::epromSize();
}
/**
* @private collectDevices
*
* Pulls all devices registered in EEPROM
* @return void 
*/
void Bootstrap::collectDevices() 
{
    Utils::log("COLLECTING_REGISTERED_DEVICES", String(deviceMeta.count));
    for (uint8_t i = 0; i < MAX_DEVICES && i < this->deviceMeta.count; i++) {
        /*
        * Device meta address only contains the details for the device where it actually 
        * stores the address pool details
        */
        uint16_t address = deviceMetaAdresses[i];
        Utils::log("DEVICE_REGISTRATION_ADDRESSES" , "VALUE:: " + String(address));
        // DeviceStruct device;
        EEPROM.get(address, devices[i]);
        String deviceDetails =  String(devices[i].version) + " " + String(devices[i].size) + " " + String(devices[i].name) + " " + String(devices[i].address);
        Utils::log("DEVICE_STORE_PULLED" , deviceDetails);
    }

}

/**
* @private maxAddressIndex
*
* gets max current registered
* @return uint16_t 
*/
int Bootstrap::maxAddressIndex()
{
    int maxAddressIndex = -1;
    uint16_t maxAddress = deviceInitAddress();
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        uint16_t address = devices[i].address;
        Serial.print(address);Serial.print(" ");Serial.println(maxAddress);
        if (this->exceedsMaxAddressSize(address) && address >= maxAddress) {
            maxAddress = address;
            maxAddressIndex = i;
        }
    }
    return maxAddressIndex;
}

/**
* @private getDeviceByName
*
* adds an unregistered device to the meta structure
* @param String name - the device name
* @param uint16_t size - the size of data being requested
* @return DeviceStruct - a device for registration 
*/
DeviceStruct Bootstrap::getDeviceByName(String name,  uint16_t size)
{
    unsigned long dName = this->machineName(name);
    // delay(10000);
    // const char * dName = name.c_str();
    for (size_t i = 0; i < MAX_DEVICES && i < this->deviceMeta.count; i++) {
      DeviceStruct device = this->devices[i];
      //Log.info("SEARCHING FOR REGISTERED DEVICE %lu %lu", dName, device.name);
      Utils::log("SEARCHING_FOR_REGISTERED_DEVICE", String(dName) + " " + String(device.name) + " " + String(dName == device.name));
      if (dName == device.name) {
          return device;
      }
    }
    // now build it
    uint16_t next = getNextDeviceAddress();
    Utils::log("THIS_IS_THE_NEXT_DEVICE_ADRESS_FOR " + name, String(next));
    DeviceStruct device = { 1, size, dName, next};
    this->addNewDeviceToStructure(device);
    return device;
}
/**
* @public registerAddress
*
* adds an unregistered device to the meta structure
* @param String name - the device name
* @param uint16_t size - the size of data being requested
* @return uint16_t - the predictable address for the device 
*/
uint16_t Bootstrap::registerAddress(String name, uint16_t size) 
{
    // delay(10000);
    DeviceStruct device = getDeviceByName(name, size);
    String deviceDetails = String(device.version) + " " + String(device.size) + " " + String(device.name) + " " + String(device.address);
    Utils::log("REGISTERED_ADDRESS_DETAILS_FOR" + name, deviceDetails);
    if (!Utils::validConfigIdentity(device.version)) {
        uint16_t send = manualDeviceTracker + size + 1;
        manualDeviceTracker = send;
        return manualDeviceTracker;
    }
    return device.address;
}

/**
* @private getNextDeviceAddress
*
* gets the next address for device registration
* @return uint16_t 
*/
uint16_t Bootstrap::getNextDeviceAddress() {
    int mAddress = maxAddressIndex();

    if (mAddress != -1) {
        DeviceStruct lastDevice = devices[mAddress];
        if (Utils::validConfigIdentity(lastDevice.version)) {
            return lastDevice.size + lastDevice.address + 8;
        }
    }
    uint16_t start = deviceInitAddress();
    return start;
}

/**
* @private 
*
* processRegistration
*
* pulls the registered devices meta details into memory
* @return void 
*/
void Bootstrap::processRegistration()
{
    Utils::log("EPROM_REGISTRATION", String(this->deviceMeta.count ) + " " + String(this->deviceMeta.version ));
    if (this->deviceMeta.version != 1) {
        this->deviceMeta = {1, 0};
        EEPROM.put(this->deviceStartAddress, this->deviceMeta);
    } 
     
    if (this->deviceMeta.count > 0) {
        collectDevices();
    }
}
/**
* @private setMetaAddresses
*
* Puts all the potential addresses into memory for
* dynamic allocation
* @return void 
*/
void Bootstrap::setMetaAddresses()
{
    uint16_t size = sizeof(DeviceStruct);
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        deviceMetaAdresses[i] = deviceContainerAddressStart + (size * i) + i; 
    }
}
/**
* @private pullRegistration
*
* wrapper function to get pull the device meta details into memory
* @return void 
*/
void Bootstrap::pullRegistration() 
{
  EEPROM.get(this->deviceStartAddress, this->deviceMeta);
  processRegistration();
}

/**
* @private addNewDeviceToStructure
*
* adds an unregistered device to the meta structure
* @param DeviceStruct device - the device for registration
* @return void 
*/
void Bootstrap::addNewDeviceToStructure(DeviceStruct device) 
{
    if (device.address > this->epromSize()) {
        return;
    }
    uint16_t address = deviceMetaAdresses[deviceMeta.count];
    // now add based on to the next index
    devices[deviceMeta.count] = device;
    EEPROM.put(address, device);
    this->deviceMeta.count++;
    EEPROM.put(this->deviceStartAddress, this->deviceMeta);
}

/**
* @private machineName
*
* converts a string to an integerm for searachble
* EEPROM storage
* @param String name - the device name
* @return unsigned int 
*/
unsigned long Bootstrap::machineName(String name)
{
   unsigned int manchineName = 0;
   for (uint8_t i = 0; i < name.length(); i++) {
       char c = name.charAt(i);
       manchineName += (unsigned int)c * name.length();
   }
   return manchineName;
}

/**
* @public hasMaintenance
*
* if the system is in maintenance 
* @return bool - true if in this mode 
*/
bool Bootstrap::hasMaintenance()
{
    return this->maintenaceMode;
}
/**
* @public setMaintenance
*
* Sets the system in maintenance mode 
* @return void
*/
void Bootstrap::setMaintenance(bool maintain)
{
    this->maintenaceMode = maintain;
}
/**
* @public publishTimerFunc
*
* Sends back if a publish event is available 
* @return bool - if ready
*/
bool Bootstrap::publishTimerFunc()
{
    return publishReleased;
}
/**
* @public heatbeatTimerFunc
*
* Sends back if a heartbeat event is available 
* @return bool - if ready
*/
bool Bootstrap::heatbeatTimerFunc()
{
    return publishHeartbeat;
}
/**
* @public readTimerFun
*
* Sends back if a read event is available 
* @return bool - if ready
*/
bool Bootstrap::readTimerFun()
{
    return readReleased;
}
/**
* @public setPublishTimer
*
* Setter for the publish event 
* @param bool - sets a publish event
* @return void
*/
void Bootstrap::setPublishTimer(bool time)
{
    publishReleased = time;
}
/**
* @public setHeatbeatTimer
*
* Setter for the heartbeat event 
* @param bool - sets a publish event
* @return void
*/
void Bootstrap::setHeatbeatTimer(bool time)
{
    publishHeartbeat = time;
}
/**
* @public setReadTimer
*
* Setter for the read event 
* @param bool - sets a publish event
* @return void
*/
void Bootstrap::setReadTimer(bool time)
{
    readReleased = time;
}
/**
* @public isStrapped
*
* is the system fully bootstrapped
* @return bool 
*/
bool Bootstrap::isStrapped()
{
    return this->bootstrapped;
}

/**
* @public getMaxVal
*
* gets the maximum number or reads we store in memory
* @return size_t 
*/
size_t Bootstrap::getMaxVal()
{
    return this->MAX_VALUE_THRESHOLD;
}
/**
* @public getPublishTime
*
* How often in minutes do we publish
* @return unsigned int 
*/
unsigned int Bootstrap::getPublishTime()
{
    return this->PUBLISH_TIMER;
}
/**
* @public getReadTime
*
* How often are we goting read
* @return unsigned int 
*/
unsigned int Bootstrap::getReadTime()
{
    return this->READ_TIMER;
}
/**
* @public buildSendInterval
*
* Chages the interval that the system sends back data and adjusts the read
* interval accoringly.
* @param int interval - the int value for publishing
* @return void 
*/
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

/**
* @private getsavedConfig
*
* Returns the configuration stored in EEPROM
* @return EpromStruct 
*/
EpromStruct Bootstrap::getsavedConfig()
{
    EpromStruct values;
    EEPROM.get(EPROM_ADDRESS, values);
    if (values.version != 1)
    {
        EpromStruct defObject = {1, 1};
        values = defObject;
    }
    return values;
}
/**
* @private resetBeachCount
*
* Resets the beached count
* @return void 
*/
void Bootstrap::resetBeachCount()
{
    beachReset();
}
/**
* @private resetBeachCount
*
* Resets the beached count
* @return void 
*/
void Bootstrap::beachReset()
{
    Log.info("BEACH RESET > %u", Bootstrap::BEACH_ADDRESS);
    BeachStruct rebeach = {0, 0};
    EEPROM.put(Bootstrap::BEACH_ADDRESS, rebeach);
}
/**
* @private beachCount
*
* The number of times the system has been beached
* @return uint8_t 
*/
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
/**
* @private isBeached
*
* Is the system beached based on the count
* @return bool 
*/
bool Bootstrap::isBeached()
{
    uint8_t bCount = beachCount();
    uint8_t beachedIncrement = bCount + 1;
    BeachStruct beachBase = {0, beachedIncrement};

    EEPROM.put(Bootstrap::BEACH_ADDRESS, beachBase);
    Log.info("GOT THIS BEACH COUNT %u of %u and increment %u", bCount, BEACHED_THRSHOLD, beachBase.count);
    return bCount >= BEACHED_THRSHOLD;
}
/**
* @private beach
*
* Put the system in beach mode. It needs to recover from a SIM failure
* @return void
*/
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

    Log.info("Sending the beached as command...");
    Cellular.command("AT+COPS=0,2\r\n");
    resetBeachCount();
    delay(2000);
    // System.reset();
}


/*
* @private putSavedConfig: 
*
* Stores our config struct 
* @return void
*/
void Bootstrap::putSavedConfig(EpromStruct config)
{
    // EEPROM.clear();
    config.version = 1;
    Log.info("PUTTING version %d publish: %d", config.version, config.pub);
    EEPROM.put(EPROM_ADDRESS, config);
}
/*
* @private bootstrap: 
*
* Called when the system bootstraps. It pulls config, checks if beached
* @return void
*/
void Bootstrap::bootstrap()
{
    // delay(5000);

    if (isBeached())
    {
        beach();
    }

    EpromStruct values = getsavedConfig();
    Log.info("BOOTSTRAPPING version %d publish: %u", values.version, values.pub);
    // a default version is 2 or 0 when instantiated.
    buildSendInterval((int)values.pub);
    bootstrapped = true;
    staticBootstrapped = true;
}
/*
* restoreDefaults
* 
* cloud function that clears all config values
* @return void
*/
void Bootstrap::restoreDefaults()
{
    // EEPROM.clear();
    buildSendInterval(DEFAULT_PUB_INTERVAL);
    EpromStruct defObject = {1, publicationIntervalInMinutes};
    putSavedConfig(defObject);
}
/*
* batteryController
* 
* Batter controller turns off the particles charging abilities.
* comment this function call if you want the batter to charge 
* from particle. When using the boomo board, we do not require 
* this feature.
* @return void
*/
void Bootstrap::batteryController()
{
    PMIC pmic;
    pmic.begin();
    pmic.disableCharging();
}

/*
* @public timers
*
* just validate our times are constantly active in the main loop
* @return void
*/
void Bootstrap::timers()
{
   
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
    Log.info(serialReadContent);
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
    if (!SERIAL_COMMS_BAUD || !wantsSerial) {
       return;
    }
    
    if (serialBuilder() && checkHasId()) {
        // now store
        Utils::log("SERIAL_CONTENT_RECEIVED" , serialReadContent);
        storeSerialContent();
    }
}
/** 
 * @public startSerial
 * 
 * A device can tell boot strap it needs the serial
 * processor. If no devices request it, the serial reading
 * function is avoided. Please be mindful to ask for this 
 * in your device classes only if needed
 * @return void
 * 
*/
void Bootstrap::startSerial() 
{
    wantsSerial = true;
}


/** 
 * @private storeSerialContent
 * 
 *  Stores and verifies fully processed serial element
 * 
*/
void Bootstrap::storeSerialContent() 
{  
    if (serialStoreIndex < 0 || serialStoreIndex >= serial_buffer_length) {
        serialStoreIndex = 0;
    }
    pushSerial(serialReadContent);
    serialReadContent = "";
    serialStoreIndex++;
   
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
    String send = serial_buffer[index] ;
    serial_buffer[index] = "";
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
    String check = serial_buffer[index];
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
 * Fetches the last requested item from a device. Iterates the buffer 
 * until a matching device identity can be found.
 * @param String idenity of the device requesting at payload 
 * 
 * */
String Bootstrap::fetchSerial(String identity) 
{
    if (!wantsSerial) {
        wantsSerial = true;
    }
    // we want to start in the index prior to the store index.
    // it is most likely in the most there. 
    size_t startindex = serialStoreIndex - 1;
    size_t i = 0;
    size_t j = indexCounter(startindex);
    while (i < serial_buffer_length) {
        if (isCorrectIdentity(identity, j)) {
            return popSerial(j);
        }
        i++;
        j = indexCounter(startindex + i);
    }
    return "";
}