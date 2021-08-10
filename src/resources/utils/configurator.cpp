#include "Particle.h"
#include "resources/devices/device-manager.h"
#include "resources/devices/device.h"
#include "resources/devices/all-weather.h"
#include "resources/devices/soil-moisture.h"
#include "resources/devices/wl-device.h"
#include "resources/devices/rain-gauge.h"
#include "resources/devices/gps-device.h"
#include "resources/devices/battery.h"

#ifndef configurator_h
#define configurator_h


class Configurator
{
private:
    Device pullDeviceType();
    bool applyDevice(Device * d);

public:
    ~Configurator();
    Configurator(DeviceManager *device);
    bool addDevice(String payload);
    bool removeDevice(String payload);
    String currentDevices();
};

#endif