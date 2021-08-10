#include "device-manager.h"
/**
 * ~DeviceManager
 * 
 * Default deconstructor
 */
DeviceManager::~DeviceManager()
{
}

/**
 * DeviceManager
 * 
 * Default constrictuor. Add your devices to the devices muti-dimentional 
 * array. Generally we will only need one device array established. But there
 * are options for more. The limit is 7 devices applied to a given box.
 *
 * @param Processor * processor - the process for sending data
 */
DeviceManager::DeviceManager(Processor *processor)
{
    // to clear EEPROM. Comment this line
    // once flashed and reflash
    // FRESH_START = true;
    if (FRESH_START) {
        SerialStorage::clearDeviceStorage();
    }
    /**
    * We need to update the deviceAggregateCounts array.
    * Normally you will leave the first dimension at ONE_I.
    * Between the curly braces {NUM}, Set the number of devices you need to initialize.
    * The max is by default set to 7.
    */
    deviceAggregateCounts[ONE_I] =  {FOUR}; //{FOUR}; // set the number of devices here
    // the numerical N_I values a indexs from 0, 1, 2 ... n
    // unless others needed. Most values will only needs the 
    // ONE_I for the first dimension.
    this->devices[ONE_I][ONE_I] = new AllWeather(&boots, ONE_I);
    this->devices[ONE_I][TWO_I] = new Battery();
    this->devices[ONE_I][THREE_I] = new SoilMoisture(&boots, TWO_I);
    // water level
    this->devices[ONE_I][FOUR_I] = new WlDevice(&boots, TWO_I);
    // rain gauge
    // this->devices[ONE_I][FIVE_I] = new RainGauge(boots);
    // another soil moisture will also work
    //this->devices[ONE_I][FIVE_I] = new SoilMoisture(boots, THREE_I);
    this->blood = new HeartBeat(System.deviceID());
    // end devices
    // set storage when we have a memory card reader
    storage = new SerialStorage(processor, &boots);
    // instantiate the processor
    this->processor = processor;
}

//////////////////////////////
/// Public Functions
////////////////////////////// 
/**
 * @public 
 * 
 * setSendInverval
 * 
 * Cloud function for setting the send interval
 * @return void
 */
int DeviceManager::setSendInverval(String read)
{
  int val = (int)atoi(read);
  // we dont let allows less than one or greater than 15
  if (val < 1 || val > 15)
  {
    return 0;
  }
  Utils::log("CLOUD_REQUESTED_INTERVAL_CHANGE", "setting payload delivery for every " + String(val) + " minutes");
  this->buildSendInverval(val);
  return val;
}

/**
 * @public 
 * 
 * init
 * 
 * Init function called at the device setup
 * @return void
 */
void DeviceManager::init()
{
    setParamsCount();
    processor->connect();
    boots.init();
    waitForTrue(&DeviceManager::isStrapped, this, 10000);
    setCloudFunction();
    clearArray();
    iterateDevices(&DeviceManager::initCallback, this);
}

/**
 * @public
 * 
 * setReadCount
 * 
 * Sets the read count 
 * @param unsigned int read_count 
 * @return void
 */
void DeviceManager::setReadCount(unsigned int read_count)
{
    this->read_count = read_count;
}

/**
 * @public 
 * 
 * recommendedMaintenace
 * 
 * Is maintenance mode recommeneded based on the timestamps
 * and the number of failed parameters? Device has neither connected
 * to the cloud nor has sensor connected.
 * @return bool - true if we should go to maintenance mode
 */
bool DeviceManager::recommendedMaintenace(u8_t damangeCount)
{
    long time = Time.now(); 
    const long THRESHOLD = 1600000000;
    if (time < THRESHOLD)
    {
        return true;
    }
    u8_t doubleConnt = damangeCount * 2;
    return doubleConnt >= this->paramsCount;
}

/**
 * @public 
 * 
 * isNotPublishing
 * 
 * It is not currently in publish mode
 * @return bool - true if a publish event can proceed
 */
bool DeviceManager::isNotPublishing()
{
    return !publishBusy;
}

/**
 * @public 
 * 
 * isNotReading
 * 
 * It is not currently in read mode
 * @return bool - true if a read event can proceed
 */
bool DeviceManager::isNotReading()
{
    return !readBusy;
}

/**
 * @public 
 * 
 * clearArray
 * 
 * Sends api request to clear devices storage arrays
 * @return void
 */
void DeviceManager::clearArray()
{
    iterateDevices(&DeviceManager::clearArrayCallback, this);
}

/**
 * @public
 * 
 * loop
 * 
 * runs off the main loop
 * @return void
 */
void DeviceManager::loop()
{
    process();  
    boots.timers();
    processor->loop();
    storage->loop();  
    processTimers();
    iterateDevices(&DeviceManager::loopCallback, this);
}

//////////////////////////////
/// Private Functions
//////////////////////////////
/**
 * @public 
 * 
 * setParamsCount
 * 
 * Counts the number of params that the system is collecting from
 * all of the initialized devices
 * @return void
 */
void DeviceManager::setParamsCount()
{
   iterateDevices(&DeviceManager::setParamsCountCallback, this);
}

/**
 * @private
 * 
 * process
 * 
 * This waits until there is a request 
 * to reboot the system
 * 
 * @return void
 */
void DeviceManager::process()
{
    if (rebootEvent)
    {
        delay(1000);
        System.reset();
    }
}

/**
 * @private
 * 
 * processTimers
 * 
 * Checks the timers to see if they are ready for the various events
 * 
 * @return void
 */
void DeviceManager::processTimers()
{
    if (boots.readTimerFun())
    {
        boots.setReadTimer(false);
        read();
    }

    if (boots.publishTimerFunc())
    {
        boots.setPublishTimer(false);
        publish();
    }

    if (processor->hasHeartbeat() && boots.heatbeatTimerFunc())
    {
        boots.setHeatbeatTimer(false);
        heartbeat();
    }
}

/**
* @private 
*
* rebootRequest
*
* Cloud function that calls a reboot request
* @param String read 
*/
int DeviceManager::rebootRequest(String read)
{
    Log.info("Reboot requested");
    rebootEvent = true;
    return 1;
}

/**
 * @private 
 * 
 * restoreDefaults
 * 
 * Cloud function for resetting defaults
 * 
 * @param String read 
 * 
 * @return void 
 */
int DeviceManager::restoreDefaults(String read)
{
  this->processRestoreDefaults();
  return 1;
}


/**
 * @private 
 * 
 * restoreDefaults
 * 
 * Calls the devices to restore their default values
 * should there be a request
 * @return void 
 */
void DeviceManager::processRestoreDefaults()
{
    boots.restoreDefaults();
    iterateDevices(&DeviceManager::restoreDefaultsCallback, this);
}

/**
 * @private 
 * 
 * setCloudFunction
 * 
 * Sets the cloud functions for the device
 * @return void
 */
void DeviceManager::setCloudFunction()
{
    Particle.function("setPublicationInterval", &DeviceManager::setSendInverval, this);
    Particle.function("restoreDefaults", &DeviceManager::restoreDefaults, this);
    Particle.function("reboot", &DeviceManager::rebootRequest, this);
}

/**
 * @private 
 * 
 * storePayload
 * 
 * A payload need to be stored to a given memory card
 * @return void
 */
void DeviceManager::storePayload(String payload, String topic)
{
    this->storage->storePayload(payload, topic);
}

/**
 * @private 
 * 
 * heartbeat
 * 
 * Publishes a heartbeach payload
 * @return void
 */
void DeviceManager::heartbeat()
{
    if (processor->connected())
    {
        String artery = blood->pump();
        Log.info("pumping blood %s", utils.stringConvert(artery));
        processor->publish(processor->getHeartbeatTopic(), artery);
    }
}

/**
 * @private
 * 
 * read
 * 
 * API call to the devices to ask for a reading
 * @return void
 */
void DeviceManager::read()
{
    waitForTrue(&DeviceManager::isNotPublishing, this, 10000);
    readBusy = true;
    iterateDevices(&DeviceManager::setReadCallback, this);
    read_count++;
    if (read_count >= MAX_SEND_TIME)
    {
        setReadCount(0);
    }

    readBusy = false;
    Log.info("READCOUNT=%d", read_count);
}

/**
 * @private
 * 
 * read
 * 
 * Wrapper to setup devices for publishing
 * @return void
 */
void DeviceManager::publish()
{
    Log.info("READY TO PUBLISH: event=%d", DeviceManager::isNotReading());
    // checkBootThreshold();
    waitForTrue(&DeviceManager::isNotReading, this, 10000);
    //waitFor(DeviceManager::isNotReading, 10000);
    publishBusy = true;

    publisher();

    if (attempt_count < ATTEMPT_THRESHOLD)
    {
        // force this puppy to try and connect. May not be needed in automatic mode
        // Particle.connect();
        // Cellular.connect();
        attempt_count++;
    }
    else
    {
        Log.info("I NEED HELP!. MY OFFLINE COUNT IS BEACHED AS %d", attempt_count);
        attempt_count = 0;
    }
    read_count = 0;
    publishBusy = false;
}

/**
 * @private
 * 
 * getBufferSize
 * 
 * Returns the buffer size for all connected devices
 * @return size_t
 */
size_t DeviceManager::getBufferSize()
{
    size_t buff_size = 120;
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            size_t buff = this->devices[i][j]->buffSize();
            if (buff)
            {
                buff_size += buff;
            }
        }
    }
    if (!buff_size)
    {
        return BUFF_SIZE;
    }

    return buff_size;
}

/**
 * @private
 * 
 * getBupopOfflineCollectionfferSize
 * 
 * Asks the storage to pop off data stored while offline
 * @return void
 */
void DeviceManager::popOfflineCollection()
{
    this->storage->popOfflineCollection(this->POP_COUNT_VALUE);
}

/**
 * @private
 * 
 * publisher
 * 
 * Gathers all data and sends to the processor the returned content
 * @return void
 */
void DeviceManager::publisher()
{

    char buf[getBufferSize()];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    writer.name("device").value(System.deviceID());
    writer.name("target").value(this->ROTATION);
    writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
    u8_t maintenanceCount = 0;
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        if (i != 0)
        {
            String name = "payload-" + String(i);
            writer.name(name.c_str()).beginObject();
        }
        else
        {
            writer.name("payload").beginObject();
        }

        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            // this->devices[i][j]->print();
            this->devices[i][j]->publish(writer, attempt_count);
            maintenanceCount += this->devices[i][j]->matenanceCount();
        }
        writer.endObject();
    }
    writer.endObject();
    // we do this so if things get beached, it automatically hits a maintenance mode
    bool recommenededMainteanc = recommendedMaintenace(maintenanceCount);
    bool inMaintenance = boots.hasMaintenance();
    bool maintenance = recommenededMainteanc || inMaintenance;
    attempt_count = 0;
    read_count = 0;
    String result = String(buf);
    clearArray();
    String topic = processor->getPublishTopic(maintenance);
    bool success = false;

    if (waitFor(Utils::connected, 2000) && processor->connected())
    {
        success = processor->publish(topic, result);
    }

    Utils::log("SENDING_EVENT_READY", String(success) + " " + String(maintenance));

    if (!maintenance && !success)
    {
        Utils::log("SENDING PAYLOAD FAILED. Storing", result);
        this->storePayload(result, topic);
    }
    else if (success)
    {
        this->popOfflineCollection();
    }

    this->ROTATION++;
}

/**
 * @private 
 * 
 * buildSendInverval
 * 
 * Called from a cloud function for setting up the timing mechanism
 * @return void
 */
void DeviceManager::buildSendInverval(int interval)
{
  this->setReadCount(0);
  this->clearArray();
  boots.buildSendInterval(interval);
}

/**
 * @private 
 * 
 * loopCallback
 * 
 * Calls the device loop function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::loopCallback(Device * device)
{
    device->loop();
}

/**
 * @private 
 * 
 * setParamsCountCallback
 * 
 * Calls the device paramCount function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::setParamsCountCallback(Device * device)
{
    u8_t count = device->paramCount();
    this->paramsCount += count;
}

/**
 * @private 
 * 
 * restoreDefaultsCallback
 * 
 * Calls the device restoreDefaults function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::restoreDefaultsCallback(Device * device)
{
    device->restoreDefaults();
}

/**
 * @private 
 * 
 * initCallback
 * 
 * Calls the device init function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::initCallback(Device * device)
{
  device->init();
}

/**
 * @private 
 * 
 * clearArrayCallback
 * 
 * Calls the device clear function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::clearArrayCallback(Device * device) 
{
  device->clear();
}

/**
 * @private 
 * 
 * setReadCallback
 * 
 * Calls the device read function during the iteration loop
 * 
 * @return void
 * 
*/
void DeviceManager::setReadCallback(Device * device) {
  device->read();
} 

/**
 * @private 
 * 
 * isStrapped
 * 
 * Checkts to see if bootstrap is finished bootstrapping 
 * 
 * @return void
 * 
*/
bool DeviceManager::isStrapped()
{
    return this->boots.isStrapped();
}

/**
 * @private 
 * 
 * iterateDevices
 * 
 * Iterates through all the devices and calls the supplied callback
 * 
 * @return void
 * 
*/
void DeviceManager::iterateDevices(void (DeviceManager::*iter) (Device * d) , DeviceManager *binding)
{
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {   
            (binding->*iter)(this->devices[i][j]);
        }
    }
}

/**
 * @private 
 * 
 * waitForTrue
 * 
 * Similie to particle's waitFor function but want it working with 
 * member functions.
 * 
 * @param bool() function - the function that needs to been called
 * @param unsigned long time - to wait for
 * 
 * @return bool
 * 
*/
bool DeviceManager::waitForTrue(bool (DeviceManager::*func)(), DeviceManager *binding, unsigned long time)
{
    bool valid = false;
    unsigned long then = millis();
    while(!valid && (millis() - time) < then) {
     valid = (binding->*func)();  
    }
    return valid;
}