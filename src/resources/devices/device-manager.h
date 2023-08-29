#include "Particle.h"
#include "string.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"
#include "resources/bootstrap/buffer-manager.h"
#include "resources/utils/utils.h"
#include "resources/utils/serial_storage.h"
#include "resources/utils/configurator.h"
#include "resources/heartbeat/heartbeat.h"
#include "device.h"

// remove
#include "rika-airquality.h"

#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5
#define SIX 6
#define SEVEN 7

#define ONE_I 0
#define TWO_I 1
#define THREE_I 2
#define FOUR_I 3
#define FIVE_I 4
#define SIX_I 5
#define SEVEN_I 6

#ifndef device_manager_h
#define device_manager_h
#define BUFF_SIZE 300
// #define SEND_HOLD_BUFFER_SIZE 512

#define DEVICE_FAILED_TO_INSTANTIATE 0
#define DEVICES_ALREADY_INSTANTIATED -1
#define DEVICES_AT_MAX -2
#define DEVICES_VIOLATES_RULES -3

const size_t DEVICE_COUNT = 5;
const size_t DEVICE_AGGR_COUNT = SEVEN;

class DeviceManager
{
private:
    SerialStorage *storage;
    bool publishBusy = false;
    bool readBusy = false;
    bool rebootEvent = false;
    void process();
    const char *AI_DEVICE_LIST_EVENT = "Ai/Get/Devices";
    unsigned int read_count = 0;
    uint8_t attempt_count = 0;
    Configurator config;
    void strapDevices();
    int applyDevice(Device *device, String deviceString, bool startup);
    void resetBuffer();
    // this value is the payload values size. We capture the other
    // values at initialization from the selected devices
    const size_t DEFAULT_BUFFER_SIZE = 120;
    const size_t DEFAULT_BUFFER_SIZE_MAX = (size_t)BUFF_SIZE;
    Bootstrap boots;
    Processor *processor;
    bool FRESH_START = false;
    const size_t deviceCount = ONE;
    size_t deviceAggregateCounts[ONE] = {1};
    Utils utils;
    HeartBeat *blood;
    unsigned int ROTATION = 0;
    uint8_t paramsCount = 0;
    const uint8_t POP_COUNT_VALUE = 5;
    void resetDeviceIndex(size_t index);
    void storePayload(String payload, String topic);
    void nullifyPayload(const char *key);
    void shuffleLoad(String payloadString);
    void placePayload(String payload);
    void popOfflineCollection();
    void confirmationExpiredCheck();
    void read();
    void publish();
    void publisher();
    void manageManualModel();
    void heartbeat();
    void processTimers();
    size_t getBufferSize();
    void packagePayload(JSONBufferWriter *writer);
    String devicesString[MAX_DEVICES];
    Device *devices[DEVICE_COUNT][DEVICE_AGGR_COUNT];
    String getTopic(bool maintenance);
    String payloadWriter(uint8_t &maintenanceCount);
    bool checkMaintenance(uint8_t maintenanceCount);
    void copyDevicesFromIndex(int index);
    void loopCallback(Device *device);
    void setParamsCountCallback(Device *device);
    void restoreDefaultsCallback(Device *device);
    void initCallback(Device *device);
    void clearArrayCallback(Device *device);
    void setReadCallback(Device *device);
    void buildSendInterval(int interval);
    int restoreDefaults(String read);
    void processRestoreDefaults();
    int rebootRequest(String f);
    bool isNotPublishing();
    bool isNotReading();
    bool isStrapped();
    void setCloudFunctions();
    // for device configuration
    void clearDeviceString();
    bool publishDeviceList();
    size_t countDeviceType(String deviceName);
    bool violatesDeviceRules(String value);
    int addDevice(String value);
    int removeDevice(String value);
    int showDevices(String value);
    int clearAllDevices(String value);

    void setParamsCount();
    bool waitForTrue(bool (DeviceManager::*func)(), DeviceManager *binding, unsigned long time);
    void iterateDevices(void (DeviceManager::*iter)(Device *d), DeviceManager *binding);

public:
    ~DeviceManager();
    DeviceManager(Processor *processor, bool debug);
    void init();
    int setSendInterval(String read);
    void clearArray();
    void setReadCount(unsigned int read_count);
    void loop();
    bool recommendedMaintenace(uint8_t damangeCount);
};

#endif