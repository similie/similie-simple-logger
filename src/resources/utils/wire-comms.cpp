#include "wire-comms.h"

WireComms::WireComms()
{
}

WireComms::WireComms(int slaveAddress)
{
    this->slaveAddress = slaveAddress;
}

WireComms::~WireComms()
{
    end();
}

void WireComms::begin()
{

    if (this->slaveAddress != -1)
    {
        return begin(this->slaveAddress);
    }
    Wire.begin();
}

void WireComms::begin(int slaveAddress)
{
    Wire.begin(slaveAddress);
    this->slaveAddress = slaveAddress;
}

void WireComms::end()
{
    Wire.end();
}

void WireComms::endAndStop(uint8_t address)
{
    Wire.endTransmission();
    Wire.beginTransmission(address);
}

size_t WireComms::writeToWire(uint8_t address, String message)
{
    if (!Wire.isEnabled())
    {
        begin();
        delay(200);
    }
    size_t length = message.length();
    Wire.beginTransmission(address);
    for (size_t i = 0; i < length; i++)
    {
        char c = message.charAt(i);
        Wire.write(c);
        if (i % maxSize == 0)
        {
            endAndStop(address);
        }
    }
    Wire.endTransmission();
    return length;
}

String WireComms::processWhatsAvailable()
{
    String send = "";
    while (Wire.available()) // if serial monitor is outputting something…
    {
        int read = Wire.read();
        if ((read == 0xFE || read == 0xFF))
        {
            continue;
        }
        send += String((char)read);
    }
    return send;
}

bool WireComms::inValidCharacter(char c)
{
    return (c < 32 || c > 126) && !(c == '\n' || c == '\r');
}

void WireComms::printResponseBuffer()
{
    uint16_t index = 0;
    char c = responseBuffer[index];
    while (c != '\0' && index < MAX_BUFFER_SIZE)
    {
        index++;
        c = responseBuffer[index];
    }
}

String WireComms::responseBufferToString()
{
    uint16_t index = 0;
    char c = responseBuffer[index];
    String send = String(c);
    while (c != '\0' && index < MAX_BUFFER_SIZE)
    {
        index++;
        c = responseBuffer[index];
        send += String(c);
    }
    return send;
}

bool WireComms::fillResponseBuffer(uint16_t index)
{
    bool breakCycle = Wire.available() == 0;
    while (Wire.available())
    {                         // slave may send less than requested
        char c = Wire.read(); // receive a byte as character
        if (inValidCharacter(c))
        {
            breakCycle = true;
            break;
        }
        responseBuffer[index] = c;
        index++;
        receivedBufferSize = index;
    }
    responseBuffer[index] = '\0';
    return breakCycle;
}

void WireComms::requestFrom(uint8_t address, unsigned long timeout)
{
    Wire.requestFrom(WireTransmission(address).quantity(maxSize).timeout(timeout));
}

void WireComms::getAllResponseData(uint8_t address, unsigned long timeout)
{
    receivedBufferSize = 0;
    uint16_t index = 0;
    uint16_t cycle = 0;
    while (cycle < MAX_CYCLES)
    {
        requestFrom(address, timeout);
        if (fillResponseBuffer(index))
        {
            break;
        }
        cycle++;
        index = cycle * maxSize;
    }
}

bool WireComms::receivedNoData()
{
    return receivedBufferSize == 0;
}

String WireComms::requiredFromWire(uint8_t address, unsigned long timeout)
{
    getAllResponseData(address, timeout);
    if (receivedNoData())
    {
        return ERROR_FLAG_NO_DATA;
    }
    return responseBufferToString();
}

String WireComms::requiredFromWire(uint8_t address)
{
    return requiredFromWire(address, DEFAULT_WIRE_TIMEOUT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message)
{
    return sendAndWaitForResponse(address, message, DEFAULT_WIRE_TIMEOUT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message, unsigned long timeout)
{
    return sendAndWaitForResponse(address, message, DEFAULT_WIRE_TIMEOUT, DEFAULT_WIRE_TIMEOUT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message, unsigned long timeout, unsigned long cmdTimeout)
{
    writeToWire(address, message);
    delay(cmdTimeout);
    return requiredFromWire(address, timeout);
}

bool WireComms::containsError(String response)
{
    String errorType = response.replace(ERROR_FLAG, "");
    return errorType.length() < response.length();
}
