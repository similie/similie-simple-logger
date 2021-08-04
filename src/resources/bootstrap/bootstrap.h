#include "Particle.h"
#include "string.h"

#ifndef bootstrap_h
#define bootstrap_h

#define DEFAULT_PUB_INTERVAL 1

#define MAX_DEVICES 7
#define EPROM_ADDRESS 0
#define MINUTE_IN_SECONDS 60
#define MILISECOND 1000
#define NO_VALUE -9999
#define TIMEZONE 9
#define ATTEMPT_THRESHOLD 3

#define MAX_SEND_TIME 15
#define SERIAL_BUFFER_LENGTH 5
#define SERIAL_COMMS_BAUD 9600

#define MAX_U16 65535
#define MAX_EEPROM_ADDRESS 8197


struct DeviceMetaStruct {
    uint8_t version;
    uint8_t count;
    // uint16_t startAddress;
    // uint16_t endAddress;
    // uint16_t address_0;
    // uint16_t address_1;
    // uint16_t address_2;
    // uint16_t address_3;
    // uint16_t address_4;
    // uint16_t address_5;
    // uint16_t address_6;
};

struct DeviceStruct {
    uint8_t version;
    uint16_t size;
    unsigned long name;
    uint16_t address;
};

struct EpromStruct
{
    uint8_t version;
    uint8_t pub;
};

struct BeachStruct
{
    uint8_t version;
    uint8_t count;
};

class Bootstrap
{
private:
    bool wantsSerial = false;
    void pullRegistration();
    void addNewDeviceToStructure(DeviceStruct device);
    bool bootstrapped = false;
    uint8_t publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
    int publishedInterval = DEFAULT_PUB_INTERVAL;
    unsigned long machineName(String name);
    void collectDevices();
    void setFunctions();
    int setMaintenanceMode(String read);
     // we can use 255 to know the index is invalid as 0 is a valid index
    // uint8_t maxAddressIndex = 255;
    int maxAddressIndex();
    uint16_t registeredAddresses[MAX_DEVICES];
   
    DeviceMetaStruct deviceMeta;
    DeviceStruct devices[MAX_DEVICES];
    DeviceStruct getDeviceByName(String name,  uint16_t size);
    uint16_t getNextDeviceAddress();
    uint16_t deviceInitAddress();
    void processRegistration();
    // bool isDigital(char value);
    bool maintenaceMode = false;
    void batteryController();
    void bootstrap();
    EpromStruct getsavedConfig();
    void putSavedConfig(EpromStruct config);
    // reset beachcount after 5 Minutes
    const size_t MAX_VALUE_THRESHOLD = MAX_SEND_TIME;
    unsigned int READ_TIMER;
    unsigned int PUBLISH_TIMER;
    unsigned int BEAT_TIMER;
    const unsigned int BEACH_LISTEN_TIME = 240 * MILISECOND;
    uint8_t beachCount();
    const uint8_t BEACHED_THRSHOLD = 5;
    const static int BEACH_ADDRESS = sizeof(EpromStruct) + 8;
    int nextAddress = BEACH_ADDRESS + 8;
    bool strappingTimers = false;
    size_t serial_buffer_length = SERIAL_BUFFER_LENGTH;
    String serial_buffer[SERIAL_BUFFER_LENGTH];
    size_t serialStoreIndex = 0;
    String serialReadContent = "";
    bool checkHasId();
    void serialInit();
    size_t indexCounter(size_t startIndex);
    bool isCorrectIdentity(String identity, size_t index) ;
    void storeSerialContent();
    int serialBuilder();
    void processSerial();
    void pushSerial(String serial);
    String popSerial(size_t index);
    bool exceedsMaxAddressSize(uint16_t address);
    void setMetaAddresses();
    uint16_t deviceMetaAdresses[MAX_DEVICES];
    uint16_t deviceContainerAddressStart = BEACH_ADDRESS + sizeof(BeachStruct) + 8;
    uint16_t deviceStartAddress = deviceContainerAddressStart  + (sizeof(DeviceStruct) * MAX_DEVICES)  +  (MAX_DEVICES + 1);
    uint16_t manualDeviceTracker = deviceStartAddress + 8;

public:
    ~Bootstrap();
    void timers();
    bool isStrapped();
    void init();
    int getStorageAddress(size_t size);
    double getCalibration();
    void restoreDefaults();
    void setPublishTimer(bool time);
    void setHeatbeatTimer(bool time);
    void setReadTimer(bool time);
    void setCalibration(double val);
    void setDigital(bool digital);
    void buildSendInterval(int interval);
    void setMaintenance(bool maintain);
    bool hasMaintenance();
    bool publishTimerFunc();
    bool heatbeatTimerFunc();
    bool readTimerFun();
    // bool isDigital();
    bool isBeached();
    void beach();
    void resetBeachCount();
    unsigned int getReadTime();
    unsigned int getPublishTime();
    void startSerial();

    String fetchSerial(String identity);
    uint16_t registerAddress(String name, uint16_t size);
    static size_t epromSize();
    static void beachReset();
    size_t getMaxVal();
    const static unsigned int ONE_MINUTE = 1 * MILISECOND * MINUTE_IN_SECONDS;
    static const size_t OVERFLOW_VAL = MAX_SEND_TIME + 5;
    static const unsigned int HEARTBEAT_TIMER = MILISECOND * MINUTE_IN_SECONDS * 15;
    static const unsigned long BEACH_TIMEOUT_RESTORE = MINUTE_IN_SECONDS * MILISECOND * 10;
};

#endif