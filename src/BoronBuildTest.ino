/*
 * Project similie-simple-logger
 * Description: A never fail basic runtime for our particle borons
 * Author: Some BadMotherFuckers
 * Date: 2 of December, the year of all hell breaking loose.
 */
#include "math.h"
#include <stdio.h>

SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(MANUAL);
#define DIG_PIN D8
#define AN_PIN A3

SerialLogHandler logHandler;
struct EpromStruct
{
  uint8_t version;
  uint8_t pub;
  double calibration;
  char digital;
};

const size_t MINUTE_IN_SECONDS = 60;
const unsigned int MILISECOND = 1000;
const int NO_VALUE = -9999;
//////// DEFAULTS //////////////////
// this is the number of times it failes before it reboots
const u8_t ATTEMPT_THRESHOLD = 3;
// this is the postevent
const String PUBLISH_EVENT = "AI_Post";
// do we have a digital wl sensor
const bool DIGITAL_DEFAULT = false;
// this is the default publish interval in minutes
const int DEFAULT_PUB_INTERVAL = 1; // Min 1 - Max 15
// eprom address for the config struct
const int EPROM_ADDRESS = 0;
// default calibration for analgue and digtal sensors
const double DEF_DISTANCE_READ_DIG_CALIBRATION = 0.01724137931;
const double DEF_DISTANCE_READ_AN_CALIBRATION = 0.335;
// this size of the array containing our data. We can have
// read sends delayed for up to 15 minuites
const size_t MAX_SEND_TIME = 15;
const int MAX_VALUE_THRESHOLD = MAX_SEND_TIME;
// We save our device ID
const String DEVICE_ID = System.deviceID();
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
int publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
unsigned int READ_TIMER = (MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
unsigned int PUBLISH_TIMER = publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND;
double currentCalibration = DIGITAL_DEFAULT ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;

// adding a slight buffer to avoid index overflow
int VALUE_HOLD[PARAM_LENGTH][MAX_VALUE_THRESHOLD + 5];

unsigned int read_count = 0;
u8_t attempt_count = 0;

// do not alter. These are state variables
// and are alter by the application
bool readReleased = false;
bool publishReleased = false;
bool readBusy = false;
bool publishBusy = false;
bool digital = DIGITAL_DEFAULT;
bool rebootEvent = false;
bool bootstrapped = false;

// more on this later
// void ai_result(const char *event, const char *data)
// {
//   Log.info("AI_Result: event=%s data=%s", event, (data ? data : "NULL"));
// }

/*
* stringConvert :: converts string to a char *
* @param: String value - to be converted 
*/
char *stringConvert(String value)
{
  char *cstr = new char[value.length() + 1];
  strcpy(cstr, value.c_str());
  return cstr;
}
/*
* Functions for mitigating state
*/
bool isStrapped()
{
  return bootstrapped;
}

bool isNotPublishing()
{
  return !publishBusy;
}

bool isNotReading()
{
  return !readBusy;
}

void releaseRead()
{
  readReleased = true;
}

void releasePublishRead()
{
  publishReleased = true;
}
/////////////////////////////////////////
// default times. There is a time that dictates
// a read event, and a publish event
Timer readtimer(READ_TIMER, releaseRead);
Timer publishtimer(PUBLISH_TIMER, releasePublishRead);
// sets the array to default values
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
    if (digital)
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
  return round(pw * currentCalibration);
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
void shift(int value, int index, int param)
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
  int index = 0;
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

  for (size_t i = 0; i < PARAM_LENGTH; i++)
  {
    String param = readParams[i];
    int median = getMedian(i, attempt_count);
    writer.name(stringConvert(param)).value(median);
    Log.info("Param=%s has median %d", stringConvert(param), median);
  }

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
  Particle.publish(PUBLISH_EVENT, stringConvert(result), PRIVATE);
  attempt_count = 0;
  read_count = 0;
}
/*
* Called to publish our payload
*/
void publish()
{
  Log.info("READY TO PUBLISH: event=%d", isNotReading());
  waitFor(isNotReading, 10000);
  publishBusy = true;
  if (waitFor(Particle.connected, 10000))
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

  if (readReleased)
  {
    read();
    readReleased = false;
  }

  if (publishReleased)
  {
    publish();
    publishReleased = false;
  }
}
/*
* We represent the digital bool as y or n so as to 
* have a value for the default
*/
char digitalChar(bool value)
{
  return value ? 'y' : 'n';
}

/*
* isDigital : Tells us if a config value is digital
*/
bool isDigital(char value)
{
  if (value == 'y')
  {
    return true;
  }
  else if (value == 'n')
  {
    return false;
  }
  else
  {
    return DIGITAL_DEFAULT;
  }
}

EpromStruct getsavedConfig()
{
  EpromStruct values;
  EEPROM.get(EPROM_ADDRESS, values);
  if (values.version != 0)
  {
    EpromStruct defObject = {2, 0, 0.0, '!'};
    values = defObject;
  }
  return values;
}
/*
* putSavedConfig: Stores our config struct 
*/
void putSavedConfig(EpromStruct config)
{
  config.version = 0;
  EEPROM.clear();
  Log.info("PUTTING version %d publish: %d, digital: %s", config.version, config.pub, config.digital);
  EEPROM.put(EPROM_ADDRESS, config);
}
/*
* buildSendInterval: using the interval count. we can calculate the timer values and set them
* to their non-default values 
*/
void buildSendInterval(int interval)
{
  publicationIntervalInMinutes = interval;
  READ_TIMER = (MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
  PUBLISH_TIMER = publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND;
  Log.info("MY READ TIMER IS SET FOR %d and the publish time is %d", READ_TIMER, PUBLISH_TIMER);
  read_count = 0;
  clearArray();
  if (readtimer.isActive())
  {
    readtimer.stop();
  }
  if (publishtimer.isActive())
  {
    publishtimer.stop();
  }
  readtimer.changePeriod(READ_TIMER);
  publishtimer.changePeriod(PUBLISH_TIMER);
  readtimer.start();
  publishtimer.start();
}
/*
* setSendInverval: cloud function that is called to set the send interval
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
  buildSendInterval(val);
  EpromStruct config = getsavedConfig();
  config.pub = publicationIntervalInMinutes;
  putSavedConfig(config);

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
  digital = (bool)val;
  EpromStruct config = getsavedConfig();
  config.digital = digitalChar(digital);
  putSavedConfig(config);
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
  currentCalibration = val;
  EpromStruct config = getsavedConfig();
  config.calibration = val;
  putSavedConfig(config);

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
* bootstrap: bootstraps the system with the values sent over post-configured cloud functions
*/
void bootstrap()
{
  delay(5000);
  // uncomment if you need to clear eeprom on load
  // EEPROM.clear();
  EpromStruct values = getsavedConfig();
  Log.info("BOOTSTRAPPING version %d publish: %d, digital: %s", values.version, values.pub, values.digital);
  // a default version is 2 or 0 when instantiated.
  if (values.version != 2 && values.pub > 0 && values.pub <= 15)
  {
    buildSendInterval(values.pub);
  }
  digital = isDigital(values.digital);

  const double calibration = values.calibration;
  // Log.info("LOADING CALIBRATOR version %s", values.calibration);
  if (values.version != 2 && calibration != 0)
  {
    currentCalibration = calibration;
  }
  else
  {
    if (digital)
    {
      currentCalibration = DEF_DISTANCE_READ_DIG_CALIBRATION;
    }
    else
    {
      currentCalibration = DEF_DISTANCE_READ_AN_CALIBRATION;
    }
  }
  bootstrapped = true;
  clearArray();
}
/*
* restoreDefaults: cloud function that clears all config values
*/
int restoreDefaults(String f)
{
  EEPROM.clear();
  buildSendInterval(DEFAULT_PUB_INTERVAL);
  digital = DIGITAL_DEFAULT;
  if (digital)
  {
    currentCalibration = DEF_DISTANCE_READ_AN_CALIBRATION;
  }
  else
  {
    currentCalibration = DEF_DISTANCE_READ_AN_CALIBRATION;
  }
  return 1;
}
/*
* timers: just validate our times are constantly active in the main loop
*/
void timers()
{
  // if we have a reboot call from a cloud function
  if (rebootEvent)
  {
    delay(1000);
    reboot();
  }

  if (!readtimer.isActive())
  {
    readtimer.start();
  }
  if (!publishtimer.isActive())
  {
    publishtimer.start();
  }
}

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
  pinMode(AN_PIN, INPUT_PULLDOWN);
  // EEPROM.clear();
  // more on this later
  // Particle.subscribe(publishEvent, ai_result);
  Particle.function("setSendInverval", setSendInverval);
  Particle.function("setDigital", setDigital);
  Particle.function("setCalibration", setCalibration);
  Particle.function("reboot", rebootRequest);
  Particle.function("restoreDefaults", restoreDefaults);
  // setting variables
  Particle.variable("digital", digital);
  Particle.variable("publicationInterval", publicationIntervalInMinutes);
  Particle.variable("currentCalibration", currentCalibration);
  Serial.println("ABOUT TO BOOTSTRAP");
  bootstrap();
  waitFor(isStrapped, 10000);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // if we want to go into manual mode,
  // we can un comment this code to connect to the cloud
  //  manageManualModel()
  // The core of your code will likely live here.
  timers();
  process();
}