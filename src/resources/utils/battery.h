#include "Particle.h"

/*
Sleep class monitors voltage and can place the device to sleep if
a threshold is crossed.
*/
#include "Particle.h"

#ifndef system_battery_h
#define system_battery_h

class SystemBattery
{
private:
    PMIC pmic;
    void disableCharging();
    const unsigned long POWER_CHECK_INTERVAL_MS = 1000;
    unsigned long lastPowerCheck = 0;

public:
    ~SystemBattery();
    SystemBattery();
    void setup();
    void loop();
};

#endif