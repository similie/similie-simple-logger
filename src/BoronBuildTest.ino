/*
 * Project BoronBuildTest
 * Description:
 * Author:
 * Date:
 */
#include "math.h"
#include <stdio.h>
#include <stdlib.h>

SYSTEM_THREAD(ENABLED);
//SYSTEM_MODE(MANUAL);
#define DIG_PIN D8
#define AN_PIN A3
//////// DEFAULTS //////////////////
const bool DIGITAL_DEFAULT = false;
const int DEFAULT_PUB_INTERVAL = 1;
const double DEF_DISTANCE_READ_DIG_CALIBRATION = 0.01724137931;
const double DEF_DISTANCE_READ_AN_CALIBRATION = 0.335;
///////////////////////////////////

String readParams[] = {"wl_cl", "hydrometric_level"};
const size_t length = sizeof(readParams) / sizeof(String);
const size_t MAX_SEND_TIME = 15;
const size_t MINUTE_IN_SECONDS = 60;
const unsigned int MILISECOND = 1000;

int publicationIntervalInMinutes = DEFAULT_PUB_INTERVAL;
// size_t currentSendTime =
// we double this buffer incase we get beached
const int MAX_VALUE_THRESHOLD = MAX_SEND_TIME; //readSeconds * MAX_SEND_TIME;

size_t readSeconds = 10;

unsigned int READ_TIMER = (MINUTE_IN_SECONDS * publicationIntervalInMinutes) / MAX_SEND_TIME * MILISECOND;
unsigned int PUBLISH_TIMER = publicationIntervalInMinutes * MINUTE_IN_SECONDS * MILISECOND;

// double DISTANCE_READ_DIG_CALIBRATION = DEF_DISTANCE_READ_DIG_CALIBRATION;
// double DISTANCE_READ_AN_CALIBRATION = DEF_DISTANCE_READ_AN_CALIBRATION;
double currentCalibration = 0;

String publishEvent = "AI_Results";
// adding a slight buffer to avoid index overflow
int VALUE_HOLD[length][MAX_VALUE_THRESHOLD + 5];

enum
{
  wl_cl,
  hydrometric_level
};

const int EPROM_ADDRESS = 20;

struct EpromStruct
{
  uint8_t version;
  int pub;
  double calibration;
  char digital;
};

const String PUBLISH_EVENT = "AI_Post";

SerialLogHandler logHandler;

const int NO_VALUE = -9999;

unsigned int read_count = 0;

u8_t attempt_count = 0;
const u8_t ATTEMPT_THRESHOLD = 2;
// more on this later
// void ai_result(const char *event, const char *data)
// {
//   Log.info("AI_Result: event=%s data=%s", event, (data ? data : "NULL"));
// }

char *stringConvert(String value)
{
  char *cstr = new char[value.length() + 1];
  strcpy(cstr, value.c_str());
  return cstr;
}

bool readReleased = false;
bool publishReleased = false;
bool readBusy = false;
bool publishBusy = false;
bool digital = DIGITAL_DEFAULT;
bool rebootEvent = false;
bool bootstrapped = false;

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

Timer readtimer(READ_TIMER, releaseRead);
Timer publishtimer(PUBLISH_TIMER, releasePublishRead);

void clearArray()
{
  for (size_t i = 0; i < length; i++)
  {
    for (size_t j = 0; j < MAX_VALUE_THRESHOLD; j++)
    {
      VALUE_HOLD[i][j] = NO_VALUE;
    }
  }
}
void printArray()
{
  for (size_t i = 0; i < length; i++)
  {
    for (size_t j = 0; j < MAX_VALUE_THRESHOLD; j++)
    {
      Log.info("PARAM VALUES FOR %s of iteration %d and value %d", stringConvert(readParams[i]), j, VALUE_HOLD[i][j]);
    }
  }
}

void reboot()
{
  Log.info("Reboot Event Requested");
  System.reset();
}

long readWL()
{
  long timeout = 1000;
  size_t count = 0;
  size_t doCount = 5;
  size_t selectedMedian = 2;
  long lastTime = millis();
  int reads[5];

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

    count++;
    // break off it it is taking too long
    if (currentTime - lastTime > timeout)
    {
      break;
    }
    delay(20);
    lastTime = millis();
  }

  int pw = NO_VALUE;
  while (pw == NO_VALUE && selectedMedian < doCount)
  {
    pw = reads[selectedMedian];
    selectedMedian++;
  }

  // double multiple = digital ? DISTANCE_READ_DIG_CALIBRATION : DISTANCE_READ_AN_CALIBRATION;
  return round(pw * currentCalibration);
}

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
  // here we have gone over. We assume due to
  // connnectivty issues;
  checkBootThreshold();
  // we do this, because we are mearly setting
  // all the values to the same
  /*
  * Every param should have it's own function, but since
  * we want our params the same, we instantiate them
  * at the top
  */

  int read = readWL();
  Log.info("WL Read:: %d", read);
  for (size_t i = 0; i < length; i++)
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

void read()
{
  waitFor(isNotPublishing, 10000);

  readBusy = true;

  setRead();
  // printArray();
  read_count++;
  readBusy = false;
  Log.info("READCOUNT=%d", read_count);
}

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

String packageJSON()
{
  Log.info("Prepping to package up %d params", length);

  char buf[300];
  memset(buf, 0, sizeof(buf));
  JSONBufferWriter writer(buf, sizeof(buf) - 1);
  writer.beginObject();

  for (size_t i = 0; i < length; i++)
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

void setEvent()
{
  String result = packageJSON();
  Particle.publish(PUBLISH_EVENT, stringConvert(result), PRIVATE);
  attempt_count = 0;
  read_count = 0;
}

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
      Particle.connect();
      attempt_count++;
    }
    else
    {
      reboot();
    }
  }
  publishBusy = false;
}

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
char digitalChar(bool value)
{
  return value ? 'y' : 'n';
}

EpromStruct getsavedConfig()
{
  EpromStruct values;
  EEPROM.get(EPROM_ADDRESS, values);
  if (values.version != 0)
  {
    // double calibration = DIGITAL_DEFAULT ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
    // EpromStruct defObject = {0, DEFAULT_PUB_INTERVAL, calibration, digitalChar(DIGITAL_DEFAULT)};
    EpromStruct defObject = {2, 0, 0.0, '!'};
    values = defObject;
  }
  return values;
}

void putSavedConfig(EpromStruct values)
{
  values.version = 0;
  EEPROM.clear();
  Log.info("PUTTING version %d publish: %d, calibrated: %d, digital: %s", values.version, values.pub, values.calibration, values.digital);
  EEPROM.put(EPROM_ADDRESS, values);
}

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

int setSendInverval(String read)
{
  int val = (int)atoi(read);
  if (val <= 0 || val > 15)
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
// 0 or 1
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

int setCalibration(String read)
{
  // Stream stream;
  double val = atof(read);
  Log.info("setting calibration of %s and %d", stringConvert(read), val);

  if (val == 0)
  {
    return 0;
  }
  currentCalibration = val;
  EpromStruct config = getsavedConfig();
  config.calibration = currentCalibration;
  putSavedConfig(config);

  return 1;
}

int rebootRequest(String f)
{
  Log.info("Reboot requested");
  rebootEvent = true;
  return 1;
}

void bootstrap()
{

  // EEPROM.clear();
  EpromStruct values = getsavedConfig();
  Log.info("BOOTSTRAPPING version %d publish: %d, calibrated: %d, digital: %s", values.version, values.pub, values.calibration, values.digital);

  if (values.version != 2 && values.pub > 0 && values.pub <= 15)
  {
    buildSendInterval(values.pub);
  }

  digital = isDigital(values.digital);
  Log.info("Setting it DIGITALLY version %d ", digital);
  if (values.version != 2 && values.calibration != 0)
  //if (false)
  {
    currentCalibration = values.calibration;
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
// setup() runs once, when the device is first turned on.
void setup()
{
  pinMode(AN_PIN, INPUT_PULLDOWN);

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

void timers()
{

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

// loop() runs over and over again, as quickly as it can execute.
void loop()
{

  timers();
  // if (waitFor(Particle.connected, 10000))
  // {
  //   Particle.process();
  // }
  // else
  // {
  //   Particle.connect();
  // }
  // The core of your code will likely live here.
  // delay(1000);
  process();
}