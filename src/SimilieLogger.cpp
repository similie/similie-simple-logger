/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/guernica0131/Sites/similie-simple-logger/src/SimilieLogger.ino"
/*
 * Project similie-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year all hell broke loose.
 */

void setup();
void loop();
#line 8 "/home/guernica0131/Sites/similie-simple-logger/src/SimilieLogger.ino"
SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(MANUAL);
#include "resources/devices/device-manager.h"

// SerialLogHandler logHandler(LOG_LEVEL_INFO); //LOG_LEVEL_ALL LOG_LEVEL_INFO
Processor processor;
//MqttProcessor processor(&boots);
DeviceManager manager(&processor);
 
// setup() runs once, when the device is first turned on.
void setup()
{
  // EEPROM.clear();
  manager.init();
}

// loop() runs over and over again, as quickly as it can execute
void loop()
{
  manager.loop();
}