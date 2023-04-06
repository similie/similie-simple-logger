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
    static void reboot();
    static double parseCloudFunctionDouble(String value, String name);
    static int parseCloudFunctionInteger(String value, String name);
    static void setDebug(bool debug);
    bool hasSerialIdentity(int identity);
    bool inValidMessageString(String message, int identity);
    static bool validConfigIdentity(uint8_t identity);
    String requestDeviceId(int identity, String cmd);
    bool serialMesssageHasError(String message, int identity);
    String receiveDeviceId(int identity);
    void parseSerial(String ourReading, size_t paramLength, size_t max, float value_hold[][Bootstrap::OVERFLOW_VAL]);
    void parseSplitReadSerial(String ourReading, size_t paramLength, size_t max, String nameMap[], float value_hold[][Bootstrap::OVERFLOW_VAL]);
    bool invalidNumber(String value);
    bool containsChar(char c, String readFrom);
    const char *stringConvert(String value);
    void insertValue(int value, int arr[], size_t size);
    void insertValue(long value, long arr[], size_t size);
    void insertValue(float value, float arr[], size_t size);
    void shift(int value, size_t index, int arr[], size_t size);
    void shift(long value, size_t index, long arr[], size_t size);
    void shift(float value, size_t index, float arr[], size_t size);
    int getMedian(int readparam, int arr[]);
    float getMax(float values[], size_t MAX);
    float getSum(float values[], size_t MAX);
    float getMedian(float arr[], size_t max);
    long getMedian(long readparam, long arr[]);
    static bool connected();
    static void machineNameDirect(String name, byte *save);
    static String machineToReadableName(byte *restore);
    static unsigned long machineName(String name, bool unique);
    static int containsValue(String arr[], size_t size, String value);
    static int getIndexOf(String, String arr[], size_t);
    // template<typename T>
    static void log(String event, String message);
    static int log(String event, String message, int errorCode);
    size_t skipMultiple(unsigned int size, size_t maxVal, unsigned int threshold);
    static int simCallback(int type, const char *buf, int len, char *value);
};

#endif