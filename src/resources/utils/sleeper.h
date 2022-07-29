#include "Particle.h"

/*
Sleep class monitors voltage and can place the device to sleep if
a threshold is crossed.
*/

#ifndef sleeper_h
#define sleeper_h

class Sleeper
{
private:
    bool DEBUG = false;
    double threshold = 0;
    const String HELP_MESSAGE = "Ai/Post/Alert/Battery";
    const unsigned long CYCLE_CHECK = 100000;
    unsigned long checkCycles = 0;
    const unsigned long SECOND_BASE = 1000;
    const unsigned long MINUTE_BASE = SECOND_BASE * 60;
    const unsigned long HOUR_BASE = MINUTE_BASE * 60;
    u_int8_t getShiftDelta(int currentHour, u_int8_t shift);
    u_int8_t hourShift(u_int8_t deltaShift, u_int8_t span);
    const u_int8_t MIDNIGHT = 0;      // 12am
    const u_int8_t EARLY_SUNRISE = 6; // 6am
    const u_int8_t NOON = 12;         // 12pm
    const u_int8_t EVENING = 18;      // 6pm
    const u_int8_t NIGHT_SHIFT = 18;
    const u_int8_t MORNING_SHIFT = 12;
    const u_int8_t AFTERNOON_BUMP = 6;
    String getHelpMessage();
    bool waitForTrue(bool (Sleeper::*func)(), Sleeper *binding, unsigned long time);
    bool testHourInterval();
    float getVCell();
    bool checkSleepValues();
    bool needsSleep();
    bool goodNight();
    bool cryForHelp();
    unsigned long timeCalToSunRise(int currentHour);
    unsigned long timeToSunRise();
    unsigned long getMinutes(u_int8_t minutes);
    unsigned long getHours(u_int8_t hours);
    FuelGauge fuel;
    SystemSleepConfiguration sleepConfig();
    unsigned long getDuration();

public:
    ~Sleeper();
    Sleeper();
    bool verify();
    void set(double value);
    void clear();
};

#endif