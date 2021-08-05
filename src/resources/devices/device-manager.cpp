#include "device-manager.h"

bool publishBusy = false;
bool readBusy = false;
bool rebootEvent = false;

/**
 * process
 * 
 * This waits until there is a request 
 * to reboot the system
 * @return void
 */
void process()
{
    if (rebootEvent)
    {
        delay(1000);
        System.reset();
    }
}
/**
* @public rebootRequest
*  cloud function that calls a reboot request
*/
int DeviceManager::rebootRequest(String f)
{
    Log.info("Reboot requested");
    rebootEvent = true;
    return 1;
}

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
 * @param BootStrap * boots - the bootstrap object
 * @param Processor * processor - the process for sending data
 */
DeviceManager::DeviceManager(Bootstrap *boots, Processor *processor)
{
    this->boots = boots;
    this->processor = processor;
    // to clear EEPROM. Comment this line
    // once flashed and reflash
    // FRESH_START = true;
    if (FRESH_START) {
        storage->clearDeviceStorage();
    }
    /**
    * We need to update the deviceAggregateCounts array.
    * Normally you will leave the first dimension at ONE_I.
    * Between the curly braces {NUM}, Set the number of devices you need to initialize.
    * The max is by default set to 7.
    */
    deviceAggregateCounts[ONE_I] =  {THREE}; //{FOUR}; // set the number of devices here
    // the numerical N_I values a indexs from 0, 1, 2 ... n
    // unless others needed. Most values will only needs the 
    // ONE_I for the first dimension.
    this->devices[ONE_I][ONE_I] = new AllWeather(boots, ONE_I);
    this->devices[ONE_I][TWO_I] = new Battery();
    this->devices[ONE_I][THREE_I] = new SoilMoisture(boots, TWO_I);
    // water level
    // this->devices[ONE_I][FOUR_I] = new WlDevice(boots, THREE_I);
    // rain gauge
    //this->devices[ONE_I][FOUR_I] = new RainGauge();
    // another soil moisture will work
    //this->devices[ONE_I][FIVE_I] = new SoilMoisture(boots, THREE_I);
    this->blood = new HeartBeat(System.deviceID());
    // end devices
    setParamsCount();
    // set storage when we have a memory card reader
    storage = new SerialStorage(processor, boots);
}

//////////////////////////////
/// Publics
////////////////////////////// 

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
    boots->init();
    Particle.function("reboot", DeviceManager::rebootRequest);
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

    long time = Time.now(); // - (millis() / 1000);
    //Sunday, September 13, 2020 12:26:40 PM
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
 * restoreDefaults
 * 
 * Calls the devices to restore their default values
 * should there be a request
 * @return void 
 */
void DeviceManager::restoreDefaults()
{
    iterateDevices(&DeviceManager::restoreDefaultsCallback, this);
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
    boots->timers();
    storage->loop();
    
    if (boots->readTimerFun())
    {
        boots->setReadTimer(false);
        read();
    }

    if (boots->publishTimerFunc())
    {
        boots->setPublishTimer(false);
        publish();
    }

    if (processor->hasHeartbeat() && boots->heatbeatTimerFunc())
    {
        boots->setHeatbeatTimer(false);
        heartbeat();
    }

    iterateDevices(&DeviceManager::loopCallback, this);
}

//////////////////////////////
/// Privates
////////////////////////////// 

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

    waitFor(DeviceManager::isNotPublishing, 10000);
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
    waitFor(DeviceManager::isNotReading, 10000);
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
    size_t buff_size = 0;
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
    bool inMaintenance = boots->hasMaintenance();
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

    Log.info("Send %d %d", success, maintenance);

    if (!maintenance && !success)
    {
        Log.info("SENDING PAYLOAD FAILED. Storing");
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
