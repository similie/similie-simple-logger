#include "Particle.h"
#include "string.h"
#include "resources/utils/sleeper.h"
#include "resources/utils/wire-comms.h"
#include "resources/utils/battery.h"

#define DEBUG_WIRE_DISCONNECT 0
#define DEVICE_BYTE_BUFFER_SIZE 48
#ifndef bootstrap_h
#define bootstrap_h
#define NO_BEACH_MODE true
#define NO_SERIAL_START true
#define DEFAULT_PUB_INTERVAL 1

#define MAX_DEVICES 7
#define EPROM_ADDRESS 0
#define MINUTE_IN_SECONDS 60
#define MILLISECOND 1000
#define NO_VALUE -9999
#define TIMEZONE_SET 9
#define ATTEMPT_THRESHOLD 3

#define MAX_SEND_TIME 15

#ifdef NO_SERIAL_START
#define SERIAL_BUFFER_LENGTH 1
#else
#define SERIAL_BUFFER_LENGTH 5
#endif

#define SERIAL_COMMS_BAUD 76800 // 9600

#define MAX_U16 65535
#define MAX_EEPROM_ADDRESS 8197

static const int TIMEZONE = TIMEZONE_SET;

struct DeviceConfig
{
    uint8_t version;
    byte device[DEVICE_BYTE_BUFFER_SIZE];
};

struct DeviceMetaStruct
{
    uint8_t version;
    uint8_t count;
};

struct DeviceStruct
{
    uint8_t version;
    uint16_t size;
    unsigned long name;
    // const char * name;
    uint16_t address;
};

struct EpromStruct
{
    uint8_t version;
    uint8_t pub;
    int timezone;
    double sleep;
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
    enum
    {
        MO_SAMD_21G18,
        FW_32u4
    };
    int localTimezone = TIMEZONE;
    unsigned long debugWireDisconnectTime = millis();
    String processorNames[2] = {"MO_SAMD_21G18", "FW_32u4"};
    int getProcessorEnum(String);
    bool validTimezone(int);
    WireComms communicator;
    SystemBattery bat;
    bool hasSerialComms = false;
    bool hasWireComms = false;
    void applyTimeZone();
    void pullRegistration();
    void addNewDeviceToStructure(DeviceStruct);
    bool bootstrapped = false;
    uint8_t publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
    double batterySleepThresholdValue = 0;
    int publishedInterval = DEFAULT_PUB_INTERVAL;
    String processorName = "";
    // unsigned long machineName(String name);
    void collectDevices();
    void setFunctions();
    int setMaintenanceMode(String);
    int setBatterySleepThreshold(String);
    int setTimeZone(String);
    void sendBatteryValueToConfig(double);
    // we can use 255 to know the index is invalid as 0 is a valid index
    void pingSerialComms();
    void pingPong();
    void processPong(String);
    int maxAddressIndex();
    Sleeper sleep;
    DeviceMetaStruct deviceMeta;
    DeviceStruct devices[MAX_DEVICES];
    DeviceStruct getDeviceByName(String, uint16_t);
    void saveDeviceMetaDetails();
    uint16_t getNextDeviceAddress();
    uint16_t deviceInitAddress();
    void processRegistration();
    bool maintenaceMode = false;
    void batteryController();
    void bootstrap();
    EpromStruct getsavedConfig();
    void putSavedConfig(EpromStruct);
    void sendTimezoneValueToConfig(short);
    // reset beachcount after 5 Minutes
    const size_t MAX_VALUE_THRESHOLD = MAX_SEND_TIME;
    unsigned int READ_TIMER;
    unsigned int PUBLISH_TIMER;
    unsigned int BEAT_TIMER;
    const unsigned int BEACH_LISTEN_TIME = 240 * MILLISECOND;
    uint8_t beachCount();
    const uint8_t BEACHED_THRSHOLD = 5;

    bool strappingTimers = false;
    size_t serial_buffer_length = SERIAL_BUFFER_LENGTH;
    String serial_buffer[SERIAL_BUFFER_LENGTH];
    size_t serialStoreIndex = 0;
    String serialReadContent = "";
    bool checkHasId();
    void serialInit();
    size_t indexCounter(size_t);
    bool isCorrectIdentity(String, size_t);
    void storeSerialContent();
    int serialBuilder();
    void processSerial();
    void pushSerial(String);
    String popSerial(size_t);
    void debugNetworkDisconnect();
    // EEPROM Address Management
    void setMetaAddresses();
    uint16_t deviceMetaAdresses[MAX_DEVICES];
    uint16_t deviceConfigAdresses[MAX_DEVICES];
    // BEACH Storage
    static const uint16_t BEACH_ADDRESS = sizeof(EpromStruct) + 8;
    // Device Meta Storage
    const uint16_t DEVICE_META_ADDRESS = BEACH_ADDRESS + sizeof(BeachStruct) + 8;
    // Stores count details
    const uint16_t DEVICE_CONFIG_STORAGE_META_ADDRESS = DEVICE_META_ADDRESS + sizeof(DeviceMetaStruct) + 8;
    // Device Type Storage
    const uint16_t DEVICE_HOLD_ADDRESS = DEVICE_CONFIG_STORAGE_META_ADDRESS + (sizeof(DeviceStruct) * MAX_DEVICES) + (MAX_DEVICES + 2) + 8;
    // Now the storage for the specific devices
    const uint16_t DEVICE_SPECIFIC_CONFIG_ADDRESS = DEVICE_HOLD_ADDRESS + ((sizeof(DeviceConfig) + 4) * MAX_DEVICES) + (MAX_DEVICES + 2);
    // in case we have to manually issue an address on first come first serve.
    uint16_t manualDeviceTracker = DEVICE_SPECIFIC_CONFIG_ADDRESS;

public:
    ~Bootstrap();
    void timers();
    bool hasSerial();
    bool isStrapped();
    void init();
    bool hasCoprocessor();
    String getProcessorName();
    bool isCoProcessorMemoryConstrained();
    bool doesNotExceedsMaxAddressSize(uint16_t);
    void strapDevices(String *);
    void storeDevice(String, int);
    void clearDeviceConfigArray();
    void haultPublication();
    void resumePublication();
    double getCalibration();
    void restoreDefaults();
    void setPublishTimer(bool);
    void setHeatbeatTimer(bool);
    void setReadTimer(bool);
    void setCalibration(double);
    void setDigital(bool);
    void buildSendInterval(int);
    void buildSleepThreshold(double);
    void setMaintenance(bool);
    bool hasMaintenance();
    bool publishTimerFunc();
    bool heatbeatTimerFunc();
    bool readTimerFun();
    bool isBeached();
    void beach();
    void resetBeachCount();
    unsigned int getReadTime();
    unsigned int getPublishTime();
    void startSerial();
    WireComms getCommunicator();
    unsigned long defaultWireTimeout();
    unsigned long defaultWireWait();
    bool wireContainsError(String);
    String fetchSerial(String);
    uint16_t registerAddress(String, uint16_t);
    static size_t epromSize();
    static void beachReset();
    size_t getMaxVal();
    bool isWire();
    String removeNewLine(String value);
    static const uint8_t coProcessorAddress = 9;
    static const unsigned int ONE_MINUTE = 1 * MILLISECOND * MINUTE_IN_SECONDS;
    static const size_t OVERFLOW_VAL = MAX_SEND_TIME + 5;
    static const unsigned int HEARTBEAT_TIMER = MILLISECOND * MINUTE_IN_SECONDS * 15;
    static const unsigned long BEACH_TIMEOUT_RESTORE = MINUTE_IN_SECONDS * MILLISECOND * 10;
};

#endif