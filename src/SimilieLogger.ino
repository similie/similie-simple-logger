/*
 * Project similie-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year of all hell breaking loose.
 */

SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(MANUAL);
#include "resources/devices/device-manager.h"

SerialLogHandler logHandler(LOG_LEVEL_INFO); //LOG_LEVEL_ALL LOG_LEVEL_INFO
Processor processor;
//MqttProcessor processor(&boots);
DeviceManager manager(&processor);

// setup() runs once, when the device is first turned on.
void setup()
{
  manager.init();
}

// loop() runs over and over again, as quickly as it can execute
void loop()
{
  manager.loop();
}