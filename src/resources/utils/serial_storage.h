#include "Particle.h"
#include "resources/processors/Processor.h"

#ifndef serial_storage_h
#define serial_storage_h


class SerialStorage
{
private:
    void payloadRestorator(String payload);
    void processPop(String value);
    String getPopStartIndex(String read);
    bool sendPopRead();
    size_t firstSpaceIndex(String value, u8_t index);
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
    void popOfflineCollection(u8_t count);
};

#endif