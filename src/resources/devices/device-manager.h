#include "Particle.h"
#include "string.h"
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/processor.h"
#include "resources/processors/mqtt_processor/mqttprocessor.h"
#include "resources/utils/utils.h"
#include "device.h"
#include "wl-device.h"
#include "battery.h"
#include "resources/heartbeat/Heartbeat.h"

#ifndef device_manager_h
#define device_manager_h
#define BUFF_SIZE 300

const size_t DEVICE_COUNT = 5;
const size_t DEVICE_AGGR_COUNT = 5;

class DeviceManager
{
private:
    // bool readBusy = false;
    // bool publishBusy = false;
    unsigned int read_count = 0;
    u8_t attempt_count = 0;
    Bootstrap *boots;
    Processor *processor;
    const size_t deviceCount = 1;
    size_t deviceAggregateCounts[1] = {2};
    Utils utils;
    HeartBeat *blood;
    void read();
    void checkBootThreshold();
    void publish();
    void publisher();
    void manageManualModel();
    void heartbeat();
    Device *devices[DEVICE_COUNT][DEVICE_AGGR_COUNT];

public:
    ~DeviceManager();
    DeviceManager();
    DeviceManager(Bootstrap *boots, Processor *processor);
    void init();
    static bool isNotPublishing();
    static bool isNotReading();
    void clearArray();
    void setReadCount(unsigned int read_count);
    static int rebootRequest(String f);
    void loop();
    bool isStrapped(bool boots);
};

#endif