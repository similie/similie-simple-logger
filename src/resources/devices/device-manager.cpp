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
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            u8_t count = this->devices[i][j]->paramCount();
            this->paramsCount += count;
        }
    }
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
     for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            this->devices[i][j]->restoreDefaults();
        }
    }
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
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            this->devices[i][j]->init();
        }
    }

    // this->setSubscriber();
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
 * clearArray
 * 
 * Sends api request to clear devices storage arrays
 * @return void
 */
void DeviceManager::clearArray()
{
    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            this->devices[i][j]->clear();
        }
    }
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
    // checkBootThreshold();
    readBusy = true;

    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            this->devices[i][j]->read();
        }
    }
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
    // this->confirmationExpiredCheck();

    if (!maintenance && !success)
    {
        //this->placePayload(result);
        Log.info("SENDING PAYLOAD FAILED. Storing");
        this->storePayload(result, topic);
    }
    else if (success)
    {
        this->popOfflineCollection();
    }

    this->ROTATION++;
    // Log.info("SENDING EVENT %d, %s :: %s", success, topic.c_str(), result.c_str());
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

    for (size_t i = 0; i < this->deviceCount; i++)
    {
        size_t size = this->deviceAggregateCounts[i];
        for (size_t j = 0; j < size; j++)
        {
            this->devices[i][j]->loop();
        }
    }
}


// /**
//  * @private
//  * 
//  * checkBootThreshold
//  * @todo consider value - no longer used
//  * @return void
//  */
// void DeviceManager::checkBootThreshold()
// {
//     if (read_count >= boots->getMaxVal())
//     {
//         if (attempt_count >= ATTEMPT_THRESHOLD)
//         {
//             boots->resetBeachCount();
//             utils.reboot();
//         }
//         else
//         {
//             read_count = 0;
//             clearArray();
//         }
//     }
// }

/**
 * @private 
 * 
 * shuffleLoad
 * 
 * Counts the number of params that the system is collecting from
 * all of the initialized devices
 * @return void
 */
// void DeviceManager::shuffleLoad(String payloadString)
// {

//     unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;

//     ControlledPayload *load[size];
//     ControlledPayload newLoad(this->ROTATION, payloadString);
//     load[0] = &newLoad;

//     for (size_t i = 1; i < size; i++)
//     {
//         ControlledPayload *payload = this->payloadController[i];
//         load[i] = payload;
//     }

//     for (size_t i = 0; i < size; i++)
//     {
//         this->payloadController[i] = load[i];
//     }
// }

// //unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
// void DeviceManager::placePayload(String payloadString)
// {
//     unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
//     for (size_t i = 0; i < size; i++)
//     {

//         ControlledPayload *payload = this->payloadController[i];
//         Log.info("CAN I PLACE THIS PAYLOAD %d %u", ControlledPayload::isExpired(payload), payload->getTarget());
//         if (ControlledPayload::isExpired(payload))
//         {
//             // here we replace it with a new load;
//             Log.info("WHAT IS THE BEACH STREET %u", this->ROTATION);
//             ControlledPayload load(this->ROTATION, payloadString);
//             this->payloadController[i] = &load;
//             break;
//         }

//         if (i == size - 1)
//         {
//             this->shuffleLoad(payloadString);
//             // we are at the end
//         }
//     }
// }
// void DeviceManager::confirmationExpiredCheck()
// {
//     unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
//     for (u8_t i = 0; i < size; i++)
//     {
//         // ControlledPayload *payload = this->payloadController[i];
//         // if (ControlledPayload::isExpired(payload) && payload->getHoldings() != NULL)
//         // {
//         //     this->storePayload(payload->getHoldings());
//         // }
//     }
// }

// void DeviceManager::nullifyPayload(const char *key)
// {
//     // char *ptr;
//     int target = atoi(key);
//     if (target == 0)
//     {
//         return;
//     }

//     Log.info("Nullifying %d", target);

//     unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
//     for (u8_t i = 0; i < size; i++)
//     {
//         ControlledPayload *payload = this->payloadController[i];
//         Log.info("IS IT ON TARGET %d %u", payload->onTarget(target), payload->getTarget());
//         if (payload->onTarget(target))
//         {
//             Log.info(" GETTING BEACHED AS " + payload->getHoldings());
//         }
//     }
// }

// void DeviceManager::subscriptionConfirmation(const char *eventName, const char *data)
// {
//     Log.info("eventName=%s data=%s", eventName, data);
//     // this->nullifyPayload(data);
// }

// void DeviceManager::subscriptionTermination(const char *eventName, const char *data)
// {
//     Log.info("eventName=%s data=%s", eventName, data);
//     this->nullifyPayload(data);
// }

// void DeviceManager::setSubscriber()
// {
//     Particle.subscribe(this->SUSCRIPTION_CONFIRMATION + System.deviceID(), &DeviceManager::subscriptionConfirmation, this);
//     Particle.subscribe(this->SUSCRIPTION_TERMINATION + System.deviceID(), &DeviceManager::subscriptionTermination, this);
// }
