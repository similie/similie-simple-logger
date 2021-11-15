#include "Particle.h"
#include "resources/processors/Processor.h"

#ifndef serial_storage_h
#define serial_storage_h

class SerialStorage
{
private:
    const short int INVALID = -1;
    void payloadRestorator(String payload);
    void processPop(String value);
    String getPopStartIndex(String read);
    bool sendPopRead();
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
    void popOfflineCollection(uint8_t count);
};

#endif