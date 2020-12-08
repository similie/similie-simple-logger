/*
 * Project similie-simple-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year of all hell breaking loose.
 */

SYSTEM_THREAD(ENABLED);

#include "resources/bootstrap/bootstrap.h"
#include "resources/utils/utils.h"
#include "resources/devices/device-manager.h"

SerialLogHandler logHandler;
const String DEVICE_ID = System.deviceID();

//MqttProcessor processor;
Processor processor;
Bootstrap boots;
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
* setDigital: cloud function that sets a device as digital or analog
* takes a 1 or 0 input
*/
int setDigital(String read)
{
  int val = (int)atoi(read);
  if (val < 0 || val > 1)
  {
    return 0;
  }
  // digital = (bool)val;
  boots.setDigital((bool)val);
  return 1;
}
/*
* setCalibration: cloud function that calibration value
*/
int setCalibration(String read)
{
  const char *stringCal = read.c_str();
  double val = ::atof(stringCal);
  Log.info("setting calibration of %s", stringCal);

  if (val == 0)
  {
    return 0;
  }
  boots.setCalibration(val);
  return 1;
}

/*
* restoreDefaults: cloud function that clears all config values
*/
int restoreDefaults(String f)
{
  boots.restoreDefaults();
  return 1;
}
/*
* timers: just validate our times are constantly active in the main loop
*/

// setup() runs once, when the device is first turned on.
void setup()
{

  Particle.function("setSendInverval", setSendInverval);
  Particle.function("setDigital", setDigital);
  Particle.function("setCalibration", setCalibration);
  Particle.function("restoreDefaults", restoreDefaults);
  // setting variable
  manager.init();
  waitFor(boots.isStrapped, 10000);
}

// loop() runs over and over again, as quickly as it can execute
void loop()
{
  manager.loop();
  processor.loop();
}