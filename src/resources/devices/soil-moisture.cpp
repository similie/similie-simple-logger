#include "soil-moisture.h"

double multiple = SOIL_MOISTURE_DEFAULT;
uint16_t saveAddressForMoisture = -1;

int setMoistureCalibration(String read)
{
  const char *stringCal = read.c_str();
  double val = ::atof(stringCal);
  Log.info("setting calibration of %s", stringCal);

  if (val == 0)
  {
    return 0;
  }

  if (saveAddressForMoisture != -1) {
    VWCStruct store = {1, val};
    EEPROM.put(saveAddressForMoisture, store);
  }
  
  multiple  = val;
  return 1;
}

void SoilMoisture::restoreDefaults() 
{
    multiple = SOIL_MOISTURE_DEFAULT;
    VWCStruct store = {1, multiple};
    EEPROM.put(saveAddressForMoisture, store);
}

SoilMoisture::~SoilMoisture()
{
}
SoilMoisture::SoilMoisture(Bootstrap *boots)
{
    this->boots = boots;
}

SoilMoisture::SoilMoisture(Bootstrap *boots, int identity)
{
    this->boots = boots;
    this->sendIdentity = identity;
}

SoilMoisture::SoilMoisture()
{
}

String SoilMoisture::name() {
    return this->deviceName;
}

void SoilMoisture::nullifyPayload(const char *key)
{
}

float SoilMoisture::extractValue(float values[], size_t key, size_t max)
{
    switch (key)
    {
    case vwc:
        return utils.getMedian(values, max) * multiple;
    default:
        return utils.getMedian(values, max);
    }
}

float SoilMoisture::extractValue(float values[], size_t key)
{
    size_t MAX = readSize();
    return extractValue(values, key, MAX);
}


String SoilMoisture::paramName(size_t index) {
    String param = valueMap[index];
    if (param.equals(NULL) || param.equals(" ") || param.equals(""))
    {
      param = "_FAILURE_";
    } else if (this->hasSerialIdentity()) {
      param += "_" + String(sendIdentity);
    } 
    return param;   
}

void SoilMoisture::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
    print();
    size_t MAX = readSize();
    for (size_t i = 0; i < SoilMoisture::PARAM_LENGTH; i++)
    {
        String param = paramName(i);
        float paramValue = extractValue(VALUE_HOLD[i], i, MAX);
        if (paramValue == NO_VALUE)
        {
            maintenanceTick++;
        }
        writer.name(param.c_str()).value(paramValue);
    }
}

bool SoilMoisture::readReady()
{
    if (!waitFor(SerialStorage::notSendingOfflineData, 1000))
    {
        return SerialStorage::notSendingOfflineData();
    }
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal() , READ_THRESHOLD);
    return readAttempt >= skip;
}
size_t SoilMoisture::readSize()
{
    unsigned int size = boots->getReadTime() / 1000;
    size_t skip = utils.skipMultiple(size, boots->getMaxVal() , READ_THRESHOLD);
    size_t expand = floor(boots->getMaxVal() / skip);
    return expand;
}

String SoilMoisture::serialResponseIdentity()
{
    return utils.receiveDeviceId(this->sendIdentity);
}

String SoilMoisture::constrictSerialIdentity()
{
    return utils.requestDeviceId(this->sendIdentity, serialMsgStr);
}

String SoilMoisture::getReadContent()
{
    if (this->hasSerialIdentity())
    {
        return constrictSerialIdentity();
    }

    return serialMsgStr;
}

String SoilMoisture::fetchReading()
{
    if (!readCompile)
    {
        return boots->fetchSerial(this->serialResponseIdentity());
    }
    return "";
}

void SoilMoisture::read()
{
    readAttempt++;
    if (!readReady())
    {
        return;
    }
    readAttempt = 0;
    String content = getReadContent();
    Serial1.println(content);
    Serial1.flush();
}

void SoilMoisture::loop()
{
    String completedSerialItem = boots->fetchSerial(this->serialResponseIdentity());
    if (!completedSerialItem.equals(""))
    {
        parseSerial(completedSerialItem);
    }
}

void SoilMoisture::clear()
{
    for (size_t i = 0; i < SoilMoisture::PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < boots->getMaxVal(); j++)
        {
            VALUE_HOLD[i][j] = NO_VALUE;
        }
    }
}

bool SoilMoisture::hasSerialIdentity()
{
    return this->sendIdentity > -1;
}

bool SoilMoisture::inValidMessageString(String message)
{
    return this->hasSerialIdentity() && !message.startsWith(this->serialResponseIdentity());
}

String SoilMoisture::replaceSerialResponceItem(String message)
{
    if (!this->hasSerialIdentity())
    {
        return message;
    }
    String replaced = message.replace(this->serialResponseIdentity() + " ", "");
    return replaced;
}

void SoilMoisture::parseSerial(String ourReading)
{

    if (inValidMessageString(ourReading))
    {
        Utils::log("SOIL_MOSTURE", "Invalid Message String");
        return;
    }

    ourReading = replaceSerialResponceItem(ourReading);
    
    readCompile = true;
    utils.parseSerial(ourReading, PARAM_LENGTH, boots->getMaxVal(), VALUE_HOLD);
    readCompile = false;
}

void SoilMoisture::print()
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        for (size_t j = 0; j < readSize(); j++)
        {
            //  Log.info("PARAM VALUES FOR %s of iteration %d and value %0.2f", utils.stringConvert(valueMap[i]), j, VALUE_HOLD[i][j]);
        }
    }
}



String SoilMoisture::uniqueName() 
{
 if (this->hasSerialIdentity())
  {
    return this->name() + String(this->sendIdentity);
  }
  return this->name();
}

void SoilMoisture::pullEpromData() 
{
  VWCStruct pulled;
  EEPROM.get(saveAddressForMoisture, pulled);
  if (pulled.version == 1 && !isnan(pulled.multiple)) {
      multiple = pulled.multiple;
  }
  delay(5000);
  Utils::log("SOIL_MOISTURE_BOOTSTRAP_MULTIPLIER", String(multiple));
}

void SoilMoisture::setDeviceAddress() {
    delay(6000);
    saveAddressForMoisture = boots->registerAddress(this->uniqueName(), sizeof(VWCStruct));
    Utils::log("SOIL_MOISTURE_BOOTSTRAP_ADDRESS", String(saveAddressForMoisture));
}

void SoilMoisture::setFunctions()
{
    Particle.function("set" + this->uniqueName(), setMoistureCalibration);
    Particle.variable(this->uniqueName(), multiple); 
}

void SoilMoisture::init()
{
    boots->startSerial();
    setDeviceAddress();
    pullEpromData();
    setFunctions();
   
}

size_t SoilMoisture::buffSize()
{
    return 75;
}

u8_t SoilMoisture::paramCount()
{
    return PARAM_LENGTH;
}

u8_t SoilMoisture::matenanceCount()
{
    u8_t maintenance = this->maintenanceTick;
    maintenanceTick = 0;
    return maintenance;
}
