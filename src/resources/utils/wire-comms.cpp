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

void WireComms::reset()
{
    Wire.reset();
}

void WireComms::resetAll()
{
    writeToWire(slaveAddress, String(resetCmd));
    delay(300);
    reset();
    delay(300);
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

void WireComms::setCoprocessorAddress(int address)
{
    this->coprocessorAddress = address;
}

void WireComms::endAndStop(uint8_t address)
{
    Wire.endTransmission();
    Wire.beginTransmission(address);
}

String WireComms::appendForCoprocessor(String message)
{
    if (message.endsWith("\n"))
    {
        return message;
    }
    return message + "\n";
}

size_t WireComms::writeToWire(uint8_t address, String send)
{
    if (!Wire.isEnabled())
    {
        begin();
        delay(200);
    }
    String message = appendForCoprocessor(send);
    size_t length = message.length();
    Wire.beginTransmission(address);
    for (size_t i = 0; i < length; i++)
    {
        char c = message.charAt(i);
        //  Serial.print(c);
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
        Serial.print(c);
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

int WireComms::fillResponseBuffer(int index)
{
    bool breakCycle = Wire.available() == 0;
    uint16_t breakCount = 0;
    uint16_t iterationCount = 0;
    while (Wire.available())
    {
        iterationCount++;     // slave may send less than requested
        char c = Wire.read(); // receive a byte as character
        if (c == 255)
        {
            breakCount++;
            continue;
        }

        if (inValidCharacter(c))
        {
            breakCycle = true;
            break;
        }
        responseBuffer[index] = c;
        index++;
    }
    responseBuffer[index] = '\0';
    receivedBufferSize = index;
    return iterationCount == breakCount || breakCycle ? -1 : index;
}

void WireComms::requestFrom(uint8_t address, unsigned long timeout)
{
    Wire.requestFrom(WireTransmission(address).quantity(maxSize).timeout(timeout));
}

void WireComms::getAllResponseData(uint8_t address, unsigned long timeout)
{
    receivedBufferSize = 0;
    int index = 0;
    uint16_t cycle = 0;
    while (cycle < MAX_CYCLES)
    {
        requestFrom(address, timeout);
        index = fillResponseBuffer(index);
        if (index == -1)
        {
            break;
        }
        cycle++;
    }

    //  printResponseBuffer();
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
    return requiredFromWire(address, DEFAULT_WIRE_WAIT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message)
{
    return sendAndWaitForResponse(address, message, DEFAULT_WIRE_WAIT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message, unsigned long timeout)
{
    return sendAndWaitForResponse(address, message, DEFAULT_WIRE_WAIT, DEFAULT_WIRE_TIMEOUT);
}

String WireComms::sendAndWaitForResponse(uint8_t address, String message, unsigned long timeout, unsigned long cmdTimeout)
{
    writeToWire(address, message);
    delay(cmdTimeout);
    return requiredFromWire(address, timeout);
}

/**
 * @brief checks to see if there is a match against string values
 *
 * @param value
 * @param match
 * @return true - if there is a match
 * @return false - if there is no match
 */
bool WireComms::containsString(String value, String match)
{
    return value.indexOf(match) >= 0;
}

bool WireComms::containsError(String response)
{
    return containsString(response, String(ERROR_FLAG)); //  errorType.length() < response.length();
}

String WireComms::sendAndWaitForResponse(String message)
{
    return sendAndWaitForResponse(coprocessorAddress, message);
}
