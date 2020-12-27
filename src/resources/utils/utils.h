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
    static int simCallback(int type, const char *buf, int len, char *value);
};

#endif