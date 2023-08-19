#include "battery.h"

SystemBattery::SystemBattery()
{
    pmic.begin();
}

SystemBattery::~SystemBattery()
{
}

void SystemBattery::loop()
{

    if (millis() - lastPowerCheck < POWER_CHECK_INTERVAL_MS)
    {
        return;
    }
    lastPowerCheck = millis();
    if (pmic.isPowerGood())
    {
        return;
    }
    pmic.disableBATFET();
}

void SystemBattery::setup()
{
    pmic.disableCharging();
    pmic.enableBATFET();
}
