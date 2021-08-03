#include "Particle.h"
#include "string.h"
#include "math.h"
#include "resources/bootstrap/bootstrap.h"

#ifndef Utils_h
#define Utils_h


class Utils
{
private:

public:
    ~Utils();
    Utils();
    void reboot();

    static bool validConfigIdentity(uint8_t identity);
    String requestDeviceId(int identity, String cmd);
    String receiveDeviceId(int identity);
    void parseSerial(String ourReading, size_t paramLength, size_t max, float  value_hold[][Bootstrap::OVERFLOW_VAL]);
    bool invalidNumber(String value);
    bool containsChar(char c, String readFrom);
    const char *stringConvert(String value);
    void insertValue(int value, int arr[], size_t size);
    void insertValue(float value, float arr[], size_t size);
    void shift(int value, size_t index, int arr[], size_t size);
    void shift(float value, size_t index, float arr[], size_t size);
    int getMedian(int readparam, int arr[]);
    float getMax(float values[], size_t MAX);
    float getSum(float values[], size_t MAX);
    float getMedian(float arr[], size_t max);
    static bool connected();
    static void log(String event, String message);
    size_t skipMultiple(unsigned int size, size_t maxVal , unsigned int threshold);
    static int simCallback(int type, const char *buf, int len, char *value);
};

#endif