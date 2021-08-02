#include "Particle.h"
#include "string.h"
#include <stdint.h>
#include "resources/bootstrap/bootstrap.h"
#include "resources/processors/Processor.h"

#include "resources/utils/utils.h"
#include "device.h"
#include "wl-device.h"
#include "resources/utils/controlled_payload.h"
#include "resources/utils/serial_storage.h"

#include "battery.h"
#include "all-weather.h"
#include "soil-moisture.h"
#include "resources/heartbeat/heartbeat.h"

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
    SerialStorage *storage;
    String SUSCRIPTION_CONFIRMATION = "Ai/Post/Confirm/";
    String SUSCRIPTION_TERMINATION = "Ai/Post/Terminate/";
    ControlledPayload *payloadController[(ControlledPayload::EXPIRATION_TIME / 60) / 1000]; //{ControlledPayload(), ControlledPayload(), ControlledPayload()}
    unsigned int read_count = 0;
    u8_t attempt_count = 0;
    Bootstrap *boots;
    Processor *processor;
    const size_t deviceCount = 1;
    size_t deviceAggregateCounts[1] = {3};
    Utils utils;
    HeartBeat *blood;
    unsigned int ROTATION = 0;
    u8_t paramsCount = 0;
    const u8_t POP_COUNT_VALUE = 5;
    void storePayload(String payload, String topic);
    void nullifyPayload(const char *key);
    void shuffleLoad(String payloadString);
    void placePayload(String payload);
    void popOfflineCollection();
    void confirmationExpiredCheck();
    void initPayloadController();
    void read();
    void checkBootThreshold();
    void publish();
    void publisher();
    void manageManualModel();
    void heartbeat();
    size_t getBufferSize();
    Device *devices[DEVICE_COUNT][DEVICE_AGGR_COUNT];
    void setSubscriber();

public:
    ~DeviceManager();
    DeviceManager();
    DeviceManager(Bootstrap *boots, Processor *processor);
    void init();
    static bool isNotPublishing();
    static bool isNotReading();
    void clearArray();
    void setParamsCount();
    void setReadCount(unsigned int read_count);
    static int rebootRequest(String f);
    void loop();
    bool recommendedMaintenace(u8_t damangeCount);
    bool isStrapped(bool boots);
    void subscriptionConfirmation(const char *eventName, const char *data);
    void subscriptionTermination(const char *eventName, const char *data);
};

#endif