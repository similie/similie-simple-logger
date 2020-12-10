#include "Utils.h"

Utils::~Utils()
{
}

Utils::Utils()
{
}

float Utils::getSum(float values[], size_t MAX)
{
    int sum = 0;
    const int THRSHOLD_VAL = 100;
    const int DIVISOR = 100;
    const int THRESHOLD = DIVISOR * THRSHOLD_VAL;
    bool hasValue = false;
    size_t MIN = 0;
    size_t last = 0;
    for (size_t i = MIN; i < MAX; i++)
    {
        if (values[i] == NO_VALUE)
        {
            continue;
        }
        int val = (int)((float)values[i] * DIVISOR);
        // this might offer a buffer
        int delta = val - last;
        if (abs(delta) > THRESHOLD)
        {
            continue;
        }
        last = val;
        hasValue = true;
        sum += val;
    }

    if (!hasValue)
    {
        return NO_VALUE;
    }

    return (float)sum / DIVISOR;
}

float Utils::getMax(float values[], size_t MAX)
{
    float value = 0;
    size_t MIN = 0;
    size_t i = MAX;
    while (i > MIN)
    {

        value = values[i - 1];
        if (value != NO_VALUE)
        {
            return value;
        }
        i--;
    }
    return value;
}

/*
* Get the middle value or the value toward the zero index
* this is not a NO_VALUE
*/
float Utils::getMedian(float arr[], size_t max)
{
    float center = (float)max / 2.0;
    int index = ceil(center);
    float value = NO_VALUE;
    while (value == NO_VALUE && index >= 0)
    {
        value = arr[index];
        index--;
    }
    return value;
}

bool Utils::containsChar(char c, String readFrom)
{
    bool contains = false;
    for (u_int i = 0; i < readFrom.length(); i++)
    {
        char check = readFrom.charAt(i);
        if (check == c)
        {
            contains = true;
            break;
        }
    }
    return contains;
}

bool Utils::invalidNumber(String value)
{
    //value.charAt(j)
    bool invalid = false;
    for (uint16_t i = 0; i < value.length(); i++)
    {
        char c = value.charAt(i);
        if ((c > '9' || c < '0') && (c != '.' && c != '-'))
        {
            invalid = true;
            break;
        }
    }

    return invalid;
}

/*
* shift :  Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param int value - the value to be inserted 
* @param int index - the index where to insert. 
*     The other values will be pushed to the next index
* @param int param - the enum value that represents the param
* @return void;
*/
void Utils::shift(float value, size_t index, float arr[], size_t size)
{

    float last = arr[index];
    arr[index] = value;
    index++;

    while (index < size)
    {
        float temp = arr[index];
        arr[index] = last;
        index++;
        if (index < size)
        {
            last = arr[index];
            arr[index] = temp;
            index++;
        }
    }
}
/*
* insertValue : insert a value into a multidimentional array in sorted order
* @param int value - the value to be inserted 
* @param int param - the enum value that represents the param
* @return void;
*/
void Utils::insertValue(float value, float arr[], size_t size)
{
    size_t index = 0;

    float aggr = arr[index];

    while (value >= aggr && aggr != NO_VALUE && index < size)
    {
        index++;
        aggr = arr[index];
    }
    shift(value, index, arr, size);
}

/*
* shift :  Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param int value - the value to be inserted 
* @param int index - the index where to insert. 
*     The other values will be pushed to the next index
* @param int param - the enum value that represents the param
* @return void;
*/
void Utils::shift(int value, size_t index, int arr[], size_t size)
{

    int last = arr[index];
    arr[index] = value;
    index++;

    while (index < size)
    {
        int temp = arr[index];
        arr[index] = last;
        index++;
        if (index < size)
        {
            last = arr[index];
            arr[index] = temp;
            index++;
        }
    }
}
/*
* insertValue : insert a value into a multidimentional array in sorted order
* @param int value - the value to be inserted 
* @param int param - the enum value that represents the param
* @return void;
*/
void Utils::insertValue(int value, int arr[], size_t size)
{
    size_t index = 0;

    int aggr = arr[index];

    while (value >= aggr && aggr != NO_VALUE && index < size)
    {
        index++;
        aggr = arr[index];
    }
    shift(value, index, arr, size);
}

/*
* Get the middle value or the value toward the zero index
* this is not a NO_VALUE
*/
int Utils::getMedian(int readparam, int arr[])
{
    float center = (float)readparam / 2.0;
    int index = ceil(center);
    int value = NO_VALUE;
    while (value == NO_VALUE && index >= 0)
    {
        value = arr[index];
        index--;
    }
    return value;
}

/*
* stringConvert :: converts string to a char *
* @param: String value - to be converted 
*/
const char *Utils::stringConvert(String value)
{
    return value.c_str();
}

void Utils::reboot()
{
    Log.info("Reboot Event Requested");
    System.reset();
}
