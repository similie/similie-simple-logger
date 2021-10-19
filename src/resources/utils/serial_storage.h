#include "Particle.h"
#include "resources/processors/Processor.h"

#ifndef serial_storage_h
#define serial_storage_h

#define OFFLINE_COUNTER 5
#define WAIT_SECONDS 3

struct popContent
{
    bool valid;
    String key;
    String content;
};

class SerialStorage
{
private:
    const unsigned long sendWait = WAIT_SECONDS * 1000;
    unsigned long lastSend = 0;
    uint8_t sendIndex = 0;
    popContent popStore[OFFLINE_COUNTER];
    bool hasPopContent = false;
    const uint8_t POP_COUNT_VALUE = OFFLINE_COUNTER;
    const short int INVALID = -1;
    short int storePayloadToSend(popContent content);
    void checkPopSend();
    void invalidatePopArray();
    short int findValidPopIndex();
    bool sendPop(popContent content);
    void resetPopElement(short int index);
    popContent payloadRestorator(String payload);
    void processPop(String value);
    String getPopStartIndex(String read);
    // bool sendPopRead();
    short int firstSpaceIndex(String value, uint8_t index);
    Processor *holdProcessor;
    Bootstrap *boots;
    String popString = "";

public:
    ~SerialStorage();
    SerialStorage(Processor *holdProcessor, Bootstrap *boots);
    static void clearDeviceStorage();
    static bool notSendingOfflineData();
    void loop();
    void storePayload(String payload, String topic);
    void popOfflineCollection();
};

#endif