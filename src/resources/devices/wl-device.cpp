#include "wl-device.h"

WlDevice::~WlDevice()
{
}

WlDevice::WlDevice()
{
}

WlDevice::WlDevice(Bootstrap *boots)
{
    this->boots = boots;
}
void WlDevice::init()
{
    if (boots->isDigital())
    {

        pinMode(DIG_PIN, INPUT);
    }
    else
    {
        pinMode(AN_PIN, INPUT_PULLDOWN);
    }
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
        if (boots->isDigital())
        {
            read = pulseIn(DIG_PIN, HIGH);
        }
        else
        {
            read = analogRead(AN_PIN);
        }

        utils.insertValue(read, reads, doCount);
        // break off it it is taking too long
        if (currentTime - lastTime > timeout)
        {
            break;
        }
        // we pop a quick delay to let the sensor, breath a bit
        delay(20);
        lastTime = millis();
    }

    int pw = utils.getMedian(doCount, reads);
    // or we can take the middle value
    return round(pw * boots->getCalibration());
}

void WlDevice::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
    for (size_t i = 0; i < PARAM_LENGTH; i++)
    {
        String param = readParams[i];
        int median = utils.getMedian(attempt_count, VALUE_HOLD[i]);
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
