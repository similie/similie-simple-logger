
#ifndef wire_comms_h
#define wire_comms_h
#include "resources/bootstrap/buffer-manager.h"
#include <Arduino.h>
#include "Particle.h"
#define ERROR_FLAG_VALUE "!!ERROR::"
#define ERROR_FLAG_VALUE_NO_DATA "!!ERROR::NO_DATA!!"

class WireComms
{
private:
    const char *resetCmd = "reset_wire";
    int slaveAddress = -1;
    int coprocessorAddress = -1;
    const uint8_t FAIL_SAFE_COUNT = 5;
    const static uint8_t maxSize = 32;
    const static uint8_t MAX_CYCLES = 10;
    static const uint16_t MAX_BUFFER_SIZE = maxSize * MAX_CYCLES + MAX_CYCLES;
    uint16_t receivedBufferSize = 0;
    const char *ERROR_FLAG = ERROR_FLAG_VALUE;
    const char *ERROR_FLAG_NO_DATA = ERROR_FLAG_VALUE_NO_DATA;
    // functions
    String appendForCoprocessor(String);
    void endAndStop(uint8_t);
    void requestFrom(uint8_t, unsigned long);
    bool inValidCharacter(char);
    int fillResponseBuffer(int);
    void printResponseBuffer();
    void getAllResponseData(uint8_t, unsigned long);
    bool receivedNoData();
    bool containsString(String, String);

public:
    WireComms();
    WireComms(int);
    ~WireComms();
    void begin();
    void begin(int);
    void end();
    void reset();
    void resetAll();
    static const unsigned long DEFAULT_WIRE_TIMEOUT = 1000;
    static const unsigned long DEFAULT_WIRE_WAIT = 500;
    void setCoprocessorAddress(int);
    String processWhatsAvailable();
    size_t writeToWire(uint8_t, String);
    String requiredFromWire(uint8_t);
    String requiredFromWire(uint8_t, unsigned long);
    String sendAndWaitForResponse(String);
    String sendAndWaitForResponse(uint8_t, String);
    String sendAndWaitForResponse(uint8_t address, String message, unsigned long);
    String sendAndWaitForResponse(uint8_t address, String message, unsigned long, unsigned long);
    String responseBufferToString();
    bool containsError(String);
};

#endif