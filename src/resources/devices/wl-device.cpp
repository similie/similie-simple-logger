#include "wl-device.h"

WlDevice::~WlDevice()
{
}

WlDevice::WlDevice(Bootstrap *boots)
{
    this->boots = boots;
}

WlDevice::WlDevice(Bootstrap *boots, int sendIdentity)
{
    this->boots = boots;
    this->sendIdentity = sendIdentity;
}

void WlDevice::saveEEPROM(WLStruct storage) 
{ 
    if (saveAddressForWL == 0) {
        return;
    }
    storage.version = 1;
    EEPROM.put(saveAddressForWL, storage);
}

int WlDevice::setDigitalCloud(String read)
{
  int val = (int)atoi(read);
  if (val < 0 || val > 1)
  {
    return 0;
  }
  digital = (bool)val;
  char saved = WlDevice::isDigital(digital);
  WLStruct storage = getProm();
  storage.digital = saved;
  WlDevice::setPin(digital);
  if (!Utils::validConfigIdentity(storage.version)) {
    storage.calibration = currentCalibration;
  }
  saveEEPROM(storage);
  return 1;
}
/*
* setCalibration: cloud function that calibration value
*/
int WlDevice::setCalibration(String read)
{
  const char *stringCal = read.c_str();
  double val = ::atof(stringCal);
  Log.info("setting calibration of %s", stringCal);

  if (val == 0)
  {
    return 0;
  }

  currentCalibration = val;

  WLStruct storage = getProm();
  storage.calibration = currentCalibration;
  if (!Utils::validConfigIdentity(storage.version)) {
    storage.digital = WlDevice::isDigital(digital);
  }
  saveEEPROM(storage);
  return 1;
}

WLStruct WlDevice::getProm() {
  WLStruct prom;
  EEPROM.put(saveAddressForWL, prom);
  return prom;
}

String WlDevice::name() {
    return this->deviceName;
}

/*
* We represent the digital bool as y or n so as to 
* have a value for the default
*/
char WlDevice::setDigital(bool value) 
{
    return value ? 'y' : 'n';
}
/*
* @private isDigital : Tells us if a config value is digital
*/
bool WlDevice::isDigital(char value)
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

void WlDevice::setPin(bool digital) 
{
    if (digital)
    {

        pinMode(DIG_PIN, INPUT);
    }
    else
    {
        pinMode(AN_PIN, INPUT_PULLDOWN);
    }
}

void WlDevice::configSetup() 
{
    digital = WlDevice::isDigital(this->config.digital);
    const double calibration = this->config.calibration;
    currentCalibration = calibration;
    WlDevice::setPin(digital);
   
}

void WlDevice::restoreDefaults() 
{
    digital = DIGITAL_DEFAULT;
    WlDevice::setPin(digital);
    currentCalibration = digital ? DEF_DISTANCE_READ_DIG_CALIBRATION : DEF_DISTANCE_READ_AN_CALIBRATION;
    this->config.calibration = currentCalibration;
    this->config.digital = digital;
    saveEEPROM(this->config);
}

bool WlDevice::hasSerialIdentity() 
{
    return utils.hasSerialIdentity(this->sendIdentity);
}

void WlDevice::setCloudFunctions()
{
  String appendage = appendIdentity();
  Particle.function("setDigital" + appendage, &WlDevice::setDigitalCloud, this);
  Particle.function("setCalibration" + appendage, &WlDevice::setCalibration, this);
  Particle.variable("digital" + appendage, digital);
  Particle.variable("currentCalibration" + appendage, currentCalibration);
}

String WlDevice::appendIdentity() 
{
    return this->hasSerialIdentity() ? String(this->sendIdentity) : "";
}

String WlDevice::uniqueName() 
{
    if (this->hasSerialIdentity())
    {
        return this->name() + String(this->sendIdentity);
    }
    return this->name();
}

void WlDevice::init()
{
   // setup
   setCloudFunctions();
   Utils::log("WL_BOOT_ADDRESS REGISTRATION", this->uniqueName());
   saveAddressForWL = boots->registerAddress(this->uniqueName(), sizeof(WLStruct));
   Utils::log("WL_BOOT_ADDRESS " + this->uniqueName(), String(saveAddressForWL));
   this->config = getProm();
   if (!Utils::validConfigIdentity(this->config.version)) {
       restoreDefaults();
   }
   configSetup();
}

int WlDevice::readWL()
{
    long timeout = 1000;
    size_t doCount = 5;
    long lastTime = millis();
    int reads[doCount];

    for (size_t i = 0; i < doCount; i++)
    {
        reads[i] = NO_VALUE;
    }

    for (size_t i = 0; i < doCount; i++)
    {
        long currentTime = millis();
        long read = 0;
       
        utils.insertValue(read, reads, doCount);
        // break off it it is taking too long
        if (currentTime - lastTime > timeout)
        {
            break;
        }
        // we pop a quick delay to let the sensor, breath a bit
        delay(50);
        lastTime = millis();
    }

    int pw = utils.getMedian(doCount, reads);
    // or we can take the middle value
    return round(pw * currentCalibration);
}

void WlDevice::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        String param = readParams[i];
        int median = utils.getMedian(attempt_count, VALUE_HOLD[i]);
        if (median == 0)
        {
            maintenanceTick++;
        }

        writer.name(utils.stringConvert(param)).value(median);
        Log.info("Param=%s has median %d", utils.stringConvert(param), median);
    }
}

void WlDevice::read()
{
    int read = readWL();
    Log.info("WL %d", read);
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        utils.insertValue(read, VALUE_HOLD[i], boots->getMaxVal());
    }
}

void WlDevice::loop()
{
}

void WlDevice::clear()
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            VALUE_HOLD[i][j] = NO_VALUE;
        }
    }
}

void WlDevice::print()
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            Log.info("PARAM VALUES FOR %s of iteration %d and value %d", utils.stringConvert(readParams[i]), j, VALUE_HOLD[i][j]);
        }
    }
}

size_t WlDevice::buffSize()
{
    return 150;
}

u8_t WlDevice::paramCount()
{
    return PARAM_LENGTH;
}

u8_t WlDevice::matenanceCount()
{
    u8_t maintenance = this->maintenanceTick;
    maintenanceTick = 0;
    return maintenance;
}
