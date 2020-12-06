/*
 * Project similie-simple-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year of all hell breaking loose.
 */

SYSTEM_THREAD(ENABLED);
#include <stdio.h>
#include "resources/heartbeat/Heartbeat.h"
#include "resources/mqtt_processor/mqttprocessor.h"
#include "resources/bootstrap/bootstrap.h"

#include "math.h"

//SYSTEM_MODE(MANUAL);
#define DIG_PIN D8
#define AN_PIN A3
SerialLogHandler logHandler;
const String PUBLISH_EVENT = "AI_Post";
const String DEVICE_ID = System.deviceID();

HeartBeat blood(DEVICE_ID);
MqttProcessor processor(DEVICE_ID);
Bootstrap boots(DEVICE_ID);

///////////////////////////////////
// The params that we will be sending
String readParams[] = {"wl_cm", "hydrometric_level"};
// map and enum for loop iterations
enum
{
  wl_cm,
  hydrometric_level
};

// The following variables do not need to be directly altered
const size_t PARAM_LENGTH = sizeof(readParams) / sizeof(String);
const size_t MAX_VALUE_THRESHOLD = boots.getMaxVal();

// adding a slight buffer to avoid index overflow
const size_t OVERFLOW_VAL = boots.getMaxVal() + (size_t)5;
int VALUE_HOLD[PARAM_LENGTH][Bootstrap::OVERFLOW_VAL];

unsigned int connect_id = 0;
unsigned int read_count = 0;
u8_t attempt_count = 0;

// do not alter. These are state variables
// and are alter by the application
bool readBusy = false;
bool publishBusy = false;
bool rebootEvent = false;
bool mqttSubscribed = false;

/*
* stringConvert :: converts string to a char *
* @param: String value - to be converted 
*/
const char *stringConvert(String value)
{
  // char *cstr = new char[value.length() + 1];
  // strcpy(cstr, value.c_str());
  // return cstr;
  return value.c_str();
}

bool isNotPublishing()
{
  return !publishBusy;
}

bool isNotReading()
{
  return !readBusy;
}

void clearArray()
{
  for (size_t i = 0; i < PARAM_LENGTH; i++)
  {
    for (size_t j = 0; j < MAX_VALUE_THRESHOLD; j++)
    {
      VALUE_HOLD[i][j] = NO_VALUE;
    }
  }
}
// prints the array for debugging
void printArray()
{
  for (size_t i = 0; i < PARAM_LENGTH; i++)
  {
    for (size_t j = 0; j < MAX_VALUE_THRESHOLD; j++)
    {
      Log.info("PARAM VALUES FOR %s of iteration %d and value %d", stringConvert(readParams[i]), j, VALUE_HOLD[i][j]);
    }
  }
}
// reboots the system
void reboot()
{
  Log.info("Reboot Event Requested");
  System.reset();
}
/* 
 * readWL: this is our primary wl sensor function that takes the .
 */
long readWL()
{
  long timeout = 1000;
  size_t count = 0;
  size_t doCount = 5;
  size_t selectedMedian = 2;
  long lastTime = millis();
  int reads[5];
  long sum = 0;

  for (size_t i = 0; i < doCount; i++)
  {
    reads[i] = NO_VALUE;
  }

  for (size_t i = 0; i < doCount; i++)
  {
    long currentTime = millis();
    long read = 0;
    if (boots.isDigital())
    {
      read = pulseIn(DIG_PIN, HIGH);
      reads[i] = read;
    }
    else
    {
      read = analogRead(AN_PIN);
      reads[i] = read;
    }
    sum += read;
    count++;
    // break off it it is taking too long
    if (currentTime - lastTime > timeout)
    {
      break;
    }
    // we pop a quick delay to let the sensor, breath a bit
    delay(20);
    lastTime = millis();
  }

  int pw = NO_VALUE;
  while (pw == NO_VALUE && selectedMedian < doCount)
  {
    pw = reads[selectedMedian];
    selectedMedian++;
  }
  // we can either take the average
  long avg = sum / count;
  // or we can take the middle value
  return round(pw * boots.getCalibration());
}
/*
* shift :  Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param int value - the value to be inserted 
* @param int index - the index where to insert. 
*     The other values will be pushed to the next index
* @param int param - the enum value that represents the param
* @return void;
*/
void shift(int value, size_t index, int param)
{
  int last = VALUE_HOLD[param][index];
  VALUE_HOLD[param][index] = value;
  index++;
  while (index < MAX_VALUE_THRESHOLD)
  {
    int temp = VALUE_HOLD[param][index];
    VALUE_HOLD[param][index] = last;
    index++;
    if (index < MAX_VALUE_THRESHOLD)
    {
      last = VALUE_HOLD[param][index];
      VALUE_HOLD[param][index] = temp;
      index++;
    }
  }
}
/*
* insertValue : insert a value into a multidimentional array in sorted order
* @param int value - the value to be inserted 
* @param int param - the enum value that represents the param
* @return void;
*/
void insertValue(int value, int param)
{
  size_t index = 0;
  int aggr = VALUE_HOLD[param][index];
  while (value >= aggr && aggr != NO_VALUE && index < MAX_VALUE_THRESHOLD)
  {
    index++;
    aggr = VALUE_HOLD[param][index];
  }
  shift(value, index, param);
}
/*
* We can start enforcing a reboot event when our send attempts hit our
* configured threshold
*/
void checkBootThreshold()
{
  if (read_count >= MAX_VALUE_THRESHOLD)
  {
    if (attempt_count >= ATTEMPT_THRESHOLD)
    {
      reboot();
    }
    else
    {
      read_count = 0;
      clearArray();
    }
  }
}

void setRead()
{
  // check of we are online
  checkBootThreshold();
  // our read, normally we would pull this value
  // in the loop below, but since we are using this value
  // for the same element, we pull it above
  int read = readWL();
  Log.info("WL Read:: %d", read);
  // for future builds and other sensors, we can use
  // the loop below to run their selection functions
  for (size_t i = 0; i < PARAM_LENGTH; i++)
  {
    // int read = NO_VALUE;
    // switch (i)
    // {
    // // case wl_cl:
    // //   break;
    // // case hydrometric_level:
    // //   break;
    // default:
    //   read = readWL();
    // }
    // Log.info("local param Read:: %d", read);
    insertValue(read, i);
  }
}
/*
* A generic read function. If we set this up as a class, this is our
* public function
*/
void read()
{
  waitFor(isNotPublishing, 10000);

  readBusy = true;
  setRead();
  // uncomment for debugging the order values array
  // printArray();
  read_count++;
  readBusy = false;
  Log.info("READCOUNT=%d", read_count);
}
/*
* Get the middle value or the value toward the zero index
* this is not a NO_VALUE
*/
int getMedian(int param, int readparam)
{
  float center = (float)readparam / 2.0;
  int index = ceil(center);
  int value = NO_VALUE;
  while (value == NO_VALUE && index >= 0)
  {
    value = VALUE_HOLD[param][index];
    index--;
  }
  return value;
}
/*
* Put our params into a json string. Native package for particle
*/
String packageJSON()
{
  Log.info("Prepping to package up %d params", PARAM_LENGTH);

  char buf[300];
  memset(buf, 0, sizeof(buf));
  JSONBufferWriter writer(buf, sizeof(buf) - 1);
  writer.beginObject();
  writer.name("device").value(DEVICE_ID);
  // const time_t time = Time.local();
  writer.name("date").value(Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
  writer.name("payload").beginObject();
  for (size_t i = 0; i < PARAM_LENGTH; i++)
  {
    String param = readParams[i];
    int median = getMedian(i, attempt_count);
    writer.name(stringConvert(param)).value(median);
    Log.info("Param=%s has median %d", stringConvert(param), median);
  }
  writer.endObject();
  writer.endObject();
  clearArray();
  return String(buf);
}
/*
* Here we send our data to particle. Future iteration will use an mqtt server+
*/
void setEvent()
{

  String result = packageJSON();
  Log.info("SENDING PAYLOAD:: %s", stringConvert(result));
  attempt_count = 0;
  read_count = 0;
  // Particle.publish(PUBLISH_EVENT, stringConvert(result), PRIVATE);
  processor.publish(processor.getPublishTopic(), result);
}

void heartbeat()
{
  if (processor.connected())
  {
    String artery = blood.pump();
    Log.info("pumping blood %s", stringConvert(artery));
    processor.publish(processor.getHeartbeatTopic(), artery);
  }
}
/*
* Called to publish our payload
*/
void publish()
{
  Log.info("READY TO PUBLISH: event=%d", isNotReading());
  waitFor(isNotReading, 10000);
  publishBusy = true;
  if (waitFor(Particle.connected, 10000) && waitFor(processor.connected, 10000))
  {
    setEvent();
  }
  else
  {
    if (attempt_count < ATTEMPT_THRESHOLD)
    {
      // force this puppy to try and connect. May not be needed in automatic mode
      Particle.connect();
      attempt_count++;
    }
    else
    {
      // here we reboot because we have exceeded our attempts
      reboot();
    }
  }
  publishBusy = false;
}
/*
* This is running the state based on our released values
*/
void process()
{

  if (rebootEvent)
  {
    delay(1000);
    reboot();
  }

  if (boots.readTimerFun())
  {
    boots.setReadTimer(false);
    read();
  }

  if (boots.publishTimerFunc())
  {
    boots.setPublishTimer(false);
    publish();
  }

  if (boots.heatbeatTimerFunc())
  {
    boots.setHeatbeatTimer(false);
    heartbeat();
  }
}

int setSendInverval(String read)
{
  int val = (int)atoi(read);
  // we dont let allows less than one or greater than 15
  if (val < 1 || val > 15)
  {
    return 0;
  }
  Log.info("setting payload delivery for every %d minutes", val);
  read_count = 0;
  clearArray();
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
  // currentCalibration = val;
  // EpromStruct config = getsavedConfig();
  // config.calibration = val;
  // putSavedConfig(config);
  boots.setCalibration(val);

  return 1;
}
/*
* rebootRequest: cloud function that calls a reboot request
*/
int rebootRequest(String f)
{
  Log.info("Reboot requested");
  rebootEvent = true;
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

void manageManualModel()
{
  if (waitFor(Particle.connected, 10000))
  {
    Particle.process();
  }
  else
  {
    Particle.connect();
  }
}

// setup() runs once, when the device is first turned on.
void setup()
{
  // EEPROM.clear();
  // batteryController();

  pinMode(AN_PIN, INPUT_PULLDOWN);
  // more on this later
  // Particle.subscribe(publishEvent, ai_result);
  Particle.function("setSendInverval", setSendInverval);
  Particle.function("setDigital", setDigital);
  Particle.function("setCalibration", setCalibration);
  Particle.function("reboot", rebootRequest);
  Particle.function("restoreDefaults", restoreDefaults);
  // setting variable
  boots.init();
  waitFor(boots.isStrapped, 10000);
  // boots.setPublishTimer(releasePublishRead);
  // boots.setHeatbeatTimer(releaseHeartbeat);
  // boots.setReadTimer(releaseRead);
  // connectMQTT();
  clearArray();
}

// loop() runs over and over again, as quickly as it can execute.

void loop()
{
  // if we want to go into manual mode,
  // we can un comment this code to connect to the cloud
  //  manageManualModel()
  // The core of your code will likely live here.
  boots.timers();
  process();
  processor.loop();
  //mqttLoop();
}