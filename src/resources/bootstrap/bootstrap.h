#include "Particle.h"
#include "string.h"

#ifndef bootstrap_h
#define bootstrap_h

#define DIGITAL_DEFAULT false
#define EPROM_ADDRESS 0
#define MINUTE_IN_SECONDS 60
#define MILISECOND 1000
#define NO_VALUE -9999
#define TIMEZONE 9
#define ATTEMPT_THRESHOLD 3

// #define DEFAULT_PUB_INTERVAL 1
#define DEFAULT_PUB_INTERVAL 10
#define DEF_DISTANCE_READ_DIG_CALIBRATION 0.01724137931
#define DEF_DISTANCE_READ_AN_CALIBRATION 0.335
#define MAX_SEND_TIME 15
//#define TIMER_STALL 60000
// const size_t MAX_SEND_TIME = 15;
// const size_t MINUTE_IN_SECONDS = 60;
// const unsigned int MILISECOND = 1000;
// const int NO_VALUE = -9999;
// const int TIMEZONE = 9;
// //////// DEFAULTS //////////////////
// // this is the number of times it failes before it reboots
// const u8_t ATTEMPT_THRESHOLD = 3;
// const unsigned int HEARTBEAT_TIMER = 60000;
// // this is the postevent

// // do we have a digital wl sensor
// // const bool DIGITAL_DEFAULT = false;
// // this is the default publish interval in minutes
// const int DEFAULT_PUB_INTERVAL = 1; // Min 1 - Max 15
// // eprom address for the config struct
// // const int EPROM_ADDRESS = 0;
// // default calibration for analgue and digtal sensors
// const double DEF_DISTANCE_READ_DIG_CALIBRATION = 0.01724137931;
// const double DEF_DISTANCE_READ_AN_CALIBRATION = 0.335;
// // this size of the array containing our data. We can have
// // read sends delayed for up to 15 minuites
// const size_t MAX_SEND_TIME = 15;///

struct EpromStruct
{
    uint8_t version;
    uint8_t pub;
    double calibration;
    char digital;
};

struct BeachStruct
{
    uint8_t version;
    uint8_t count;
};

class Bootstrap
{
private:
    bool bootstrapped = false;
    bool digital = DIGITAL_DEFAULT;

    char digitalChar(bool value);
    bool isDigital(char value);
    bool maintenaceMode = false;
    void batteryController();
    void bootstrap();
    EpromStruct getsavedConfig();
    void putSavedConfig(EpromStruct config);
    uint8_t publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
    int publishedInterval = DEFAULT_PUB_INTERVAL;
    double currentCalibration = (DIGITAL_DEFAULT) ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
    // reset beachcount after 5 Minutes

    const size_t MAX_VALUE_THRESHOLD = MAX_SEND_TIME;
    unsigned int READ_TIMER;
    unsigned int PUBLISH_TIMER;
    unsigned int BEAT_TIMER;
    const unsigned int BEACH_LISTEN_TIME = 240 * MILISECOND;
    uint8_t beachCount();
    const uint8_t BEACHED_THRSHOLD = 5;
    const static int BEACH_ADDRESS = sizeof(EpromStruct) + 8;
    bool strappingTimers = false;
    // Timer *publishtimer;
    // Timer *readtimer;

public:
    ~Bootstrap();
    //Bootstrap(String deviceId);
    void timers();
    bool isStrapped();
    void init();
    double getCalibration();
    void restoreDefaults();
    void setPublishTimer(bool time);
    void setHeatbeatTimer(bool time);
    void setReadTimer(bool time);
    void setCalibration(double val);
    void setDigital(bool digital);
    void buildSendInterval(int interval);
    // void buildSendInterval(int interval, bool test);
    void setMaintenance(bool maintain);
    bool hasMaintenance();
    bool publishTimerFunc();
    bool heatbeatTimerFunc();
    bool readTimerFun();
    bool isDigital();
    bool isBeached();
    void beach();
    void resetBeachCount();
    //static bool isStrapped();
    static void beachReset();
    size_t getMaxVal();
    const static unsigned int ONE_MINUTE = 1 * MILISECOND * MINUTE_IN_SECONDS;
    static const size_t OVERFLOW_VAL = MAX_SEND_TIME + 5;
    static const unsigned int HEARTBEAT_TIMER = MILISECOND * MINUTE_IN_SECONDS * 15;
    static const unsigned long BEACH_TIMEOUT_RESTORE = MINUTE_IN_SECONDS * MILISECOND * 10;
};

#endif