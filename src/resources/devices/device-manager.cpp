#include "device-manager.h"

bool publishBusy = false;
bool readBusy = false;
bool rebootEvent = false;

void process()
{

    if (rebootEvent)
    {
        delay(1000);
        System.reset();
    }
}

DeviceManager::~DeviceManager()
{
}

DeviceManager::DeviceManager()
{
}

DeviceManager::DeviceManager(Bootstrap *boots, Processor *processor)
{
    this->boots = boots;
    this->processor = processor;
    this->devices[0][0] = new AllWeather(boots, 0);
    //this->devices[0][0] = new WlDevice(boots);
    this->devices[0][1] = new Battery();
    this->devices[0][2] = new SoilMoisture(boots, 1);
    const String DEVICE_ID = System.deviceID();
    this->blood = new HeartBeat(DEVICE_ID);
    setParamsCount();
    storage = new SerialStorage(processor, boots);
    //  this->initPayloadController();
}

void DeviceManager::initPayloadController()
{
    //  unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
}

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

void DeviceManager::shuffleLoad(String payloadString)
{

    unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;

    ControlledPayload *load[size];
    ControlledPayload newLoad(this->ROTATION, payloadString);
    load[0] = &newLoad;

    for (size_t i = 1; i < size; i++)
    {
        ControlledPayload *payload = this->payloadController[i];
        load[i] = payload;
    }

    for (size_t i = 0; i < size; i++)
    {
        this->payloadController[i] = load[i];
    }
}

//unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
void DeviceManager::placePayload(String payloadString)
{
    unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
    for (size_t i = 0; i < size; i++)
    {

        ControlledPayload *payload = this->payloadController[i];
        Log.info("CAN I PLACE THIS PAYLOAD %d %u", ControlledPayload::isExpired(payload), payload->getTarget());
        if (ControlledPayload::isExpired(payload))
        {
            // here we replace it with a new load;
            Log.info("WHAT IS THE BEACH STREET %u", this->ROTATION);
            ControlledPayload load(this->ROTATION, payloadString);
            this->payloadController[i] = &load;
            break;
        }

        if (i == size - 1)
        {
            this->shuffleLoad(payloadString);
            // we are at the end
        }
    }
}
/*
* rebootRequest: cloud function that calls a reboot request
*/
int DeviceManager::rebootRequest(String f)
{
    Log.info("Reboot requested");
    rebootEvent = true;
    return 1;
}

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

bool DeviceManager::isNotPublishing()
{
    return !publishBusy;
}

bool DeviceManager::isNotReading()
{
    return !readBusy;
}

bool DeviceManager::isStrapped(bool boots)
{
    return boots;
}

void DeviceManager::confirmationExpiredCheck()
{
    unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
    for (u8_t i = 0; i < size; i++)
    {
        // ControlledPayload *payload = this->payloadController[i];
        // if (ControlledPayload::isExpired(payload) && payload->getHoldings() != NULL)
        // {
        //     this->storePayload(payload->getHoldings());
        // }
    }
}

void DeviceManager::storePayload(String payload, String topic)
{
    this->storage->storePayload(payload, topic);
}

void DeviceManager::nullifyPayload(const char *key)
{
    // char *ptr;
    int target = atoi(key);
    if (target == 0)
    {
        return;
    }

    Log.info("Nullifying %d", target);

    unsigned long size = (ControlledPayload::EXPIRATION_TIME / 60) / 1000;
    for (u8_t i = 0; i < size; i++)
    {
        ControlledPayload *payload = this->payloadController[i];
        Log.info("IS IT ON TARGET %d %u", payload->onTarget(target), payload->getTarget());
        if (payload->onTarget(target))
        {
            Log.info(" GETTING BEACHED AS " + payload->getHoldings());
        }
    }
}

void DeviceManager::subscriptionConfirmation(const char *eventName, const char *data)
{
    Log.info("eventName=%s data=%s", eventName, data);
    // this->nullifyPayload(data);
}

void DeviceManager::subscriptionTermination(const char *eventName, const char *data)
{
    Log.info("eventName=%s data=%s", eventName, data);
    this->nullifyPayload(data);
}

void DeviceManager::setSubscriber()
{
    Particle.subscribe(this->SUSCRIPTION_CONFIRMATION + System.deviceID(), &DeviceManager::subscriptionConfirmation, this);
    Particle.subscribe(this->SUSCRIPTION_TERMINATION + System.deviceID(), &DeviceManager::subscriptionTermination, this);
}

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

    this->setSubscriber();
}

void DeviceManager::heartbeat()
{
    if (processor->connected())
    {
        String artery = blood->pump();
        Log.info("pumping blood %s", utils.stringConvert(artery));
        processor->publish(processor->getHeartbeatTopic(), artery);
    }
}

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

void DeviceManager::setReadCount(unsigned int read_count)
{
    this->read_count = read_count;
}

void DeviceManager::checkBootThreshold()
{
    if (read_count >= boots->getMaxVal())
    {
        if (attempt_count >= ATTEMPT_THRESHOLD)
        {
            boots->resetBeachCount();
            utils.reboot();
        }
        else
        {
            read_count = 0;
            clearArray();
        }
    }
}

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

/*
* Called to publish our payload
*/
void DeviceManager::publish()
{
    Log.info("READY TO PUBLISH: event=%d", DeviceManager::isNotReading());

    // checkBootThreshold();
    waitFor(DeviceManager::isNotReading, 10000);
    publishBusy = true;

    publisher();

    // if (waitFor(Utils::connected, 10000) && processor->connected())
    // {
    //  //  publisher();
    // }
    // else
    // {
    if (attempt_count < ATTEMPT_THRESHOLD)
    {
        // force this puppy to try and connect. May not be needed in automatic mode
        // Particle.connect();
        // Cellular.connect();
        attempt_count++;
    }
    else
    {
        // here we reboot because we have exceeded our attempts
        // utils.reboot();
        Log.info("I NEED HELP!. MY OFFLINE COUNT IS BEACHED AS %d", attempt_count);
        attempt_count = 0;
    }
    // }
    read_count = 0;
    publishBusy = false;
}

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

void DeviceManager::popOfflineCollection()
{
    this->storage->popOfflineCollection(this->POP_COUNT_VALUE);
}

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
