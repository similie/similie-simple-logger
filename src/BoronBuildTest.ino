/*
 * Project similie-simple-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year of all hell breaking loose.
 */

SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(MANUAL);

#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"
#include "resources/devices/device-manager.h"

SerialLogHandler logHandler(LOG_LEVEL_INFO); //LOG_LEVEL_ALL LOG_LEVEL_INFO
const String DEVICE_ID = System.deviceID();

Bootstrap boots;
Processor processor;
//MqttProcessor processor(&boots);
Utils utils;
DeviceManager manager(&boots, &processor);

/*
* This is running the state based on our released values
*/

int setSendInverval(String read)
{
  int val = (int)atoi(read);
  // we dont let allows less than one or greater than 15
  if (val < 1 || val > 15)
  {
    return 0;
  }
  Log.info("setting payload delivery for every %d minutes", val);

  manager.setReadCount(0);
  manager.clearArray();
  boots.buildSendInterval(val);
  return val;
}

/*
* restoreDefaults: cloud function that clears all config values
*/
int restoreDefaults(String f)
{
  boots.restoreDefaults();
  manager.restoreDefaults();
  return 1;
}

int setMaintenanceMode(String read)
{
  int val = (int)atoi(read);
  if (val < 0 || val > 1)
  {
    return -1;
  }
  // digital = (bool)val;
  boots.setMaintenance((bool)val);
  return val;
}


// setup() runs once, when the device is first turned on.
void setup()
{
  Particle.function("setPublicationInterval", setSendInverval);
  Particle.function("restoreDefaults", restoreDefaults);
  Particle.function("setMaintenanceMode", setMaintenanceMode);
  // // setting variable
  processor.connect();
  manager.init();
  waitFor(boots.isStrapped, 10000);
 
}

// loop() runs over and over again, as quickly as it can execute
void loop()
{
  manager.loop();
  processor.loop();
}