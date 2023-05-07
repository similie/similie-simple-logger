/*
 * Project similie-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Similie
 * Date: Started 2 of December, 2020.
 */
SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC);
// SYSTEM_MODE(MANUAL);
// SYSTEM_MODE(SEMI_AUTOMATIC);
#include "resources/devices/device-manager.h"
#define DEBUGGER true                        // set to false for field devices
SerialLogHandler logHandler(LOG_LEVEL_INFO); // LOG_LEVEL_ALL LOG_LEVEL_INFO
Processor processor;
// MqttProcessor processor(&boots);
DeviceManager manager(&processor, DEBUGGER);

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