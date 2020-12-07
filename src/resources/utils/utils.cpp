#include "Utils.h"

Utils::~Utils()
{
}

Utils::Utils()
{
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
    // char *cstr = new char[value.length() + 1];
    // strcpy(cstr, value.c_str());
    // return cstr;
    return value.c_str();
}

void Utils::reboot()
{
    Log.info("Reboot Event Requested");
    System.reset();
}
