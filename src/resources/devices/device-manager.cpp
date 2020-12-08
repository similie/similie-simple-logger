#include "device-manager.h"

//WlDevice wl;
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
    // WlDevice wl;
    this->boots = boots;
    this->processor = processor;
    this->devices[0][0] = new WlDevice(boots);
    this->devices[0][1] = new Battery();
    const String DEVICE_ID = System.deviceID();
    this->blood = new HeartBeat(DEVICE_ID);
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
void DeviceManager::init()
{
    boots->init();
    Particle.function("reboot", DeviceManager::rebootRequest);
    clearArray();
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
    checkBootThreshold();
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
    if (waitFor(Particle.connected, 10000) && processor->connected())
    {
        publisher();
    }
    else
    {
        if (attempt_count < ATTEMPT_THRESHOLD)
        {
            // force this puppy to try and connect. May not be needed in automatic mode
            Particle.connect();
            attempt_count++;
        }
        else
        {
            // here we reboot because we have exceeded our attempts
            utils.reboot();
        }
    }
    read_count = 0;
    publishBusy = false;
}

void DeviceManager::publisher()
{

    char buf[BUFF_SIZE];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    writer.name("device").value(System.deviceID());
    writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
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
        }
        writer.endObject();
    }
    writer.endObject();

    attempt_count = 0;
    read_count = 0;
    String result = String(buf);
    clearArray();
    Log.info("SENDING EVENT:: %s", result.c_str());
    processor->publish(processor->getPublishTopic(), result);
}
/* IF IN MANUAL MODE */
void manageManualModel()
{
    if (waitFor(Particle.connected, 10000))
    {
        Particle.process();
    }
    else
    {
        Particle.connect();
    }
}

void DeviceManager::loop()
{
    process();
    boots->timers();
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
