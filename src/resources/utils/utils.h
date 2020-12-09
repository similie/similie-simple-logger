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
    const char *stringConvert(String value);
    void insertValue(int value, int arr[], size_t size);
    void shift(int value, size_t index, int arr[], size_t size);
    int getMedian(int readparam, int arr[]);
};

#endif