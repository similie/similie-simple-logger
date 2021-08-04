#include "utils.h"

Utils::~Utils()
{
}

Utils::Utils()
{
}

String Utils::requestDeviceId(int identity, String cmd) 
{
    return "request_" + String(identity) + " " + cmd;
}

bool Utils::serialMesssageHasError(String message, int identity) {
    bool error = false;
    if (message.startsWith("ERROR_" + String(identity))) {
        String errorMessage = " FOR " + String(identity) + " WITH MESSAGE:: " + message.replace("ERROR_" + String(identity), "");
        this->log("SERIAL_BUS_ERROR", errorMessage);
        error = true;
    }
    return error;
}

bool Utils::hasSerialIdentity(int identity)
{
    return identity > -1;
}

bool Utils::inValidMessageString(String message, int identity)
{
    return this->serialMesssageHasError(message, identity) || 
     (this->hasSerialIdentity(identity) && 
     !message.startsWith(receiveDeviceId(identity)));
}

String getTimePadding() {
    String time = String(millis());
    uint8_t pad = 10 - time.length();
    String padding = "";
    for (uint8_t i = 0; i < pad; i++) {
        padding += "0";
    }

    return padding + time;
}

bool Utils::validConfigIdentity(uint8_t identity) 
{
    return identity == 1;
}

void Utils::log(String event, String message)
{
    Serial.print(getTimePadding());Serial.print(" [SIMILIE] " + event + ": ");Serial.println(message);
}


String Utils::receiveDeviceId(int identity)
{
    return "result_" + String(identity);
}

void Utils::parseSerial(String ourReading, size_t paramLength, size_t max, float  value_hold[][Bootstrap::OVERFLOW_VAL])
{

    size_t j = 1;
    for (size_t i = 0; i < paramLength; i++)
    {
        char c = ourReading.charAt(j);
        j++;
        if (c == '+' || c == '-')
        {
            String buff = "";
            char d = ' ';
            while (d != '+' && d != '-' && d != '\n' && d != '\0')
            {
                d = ourReading.charAt(j);
                buff += String(d);
                j++;
                d = ourReading.charAt(j);
            }
            if (this->invalidNumber(buff))
            {
                continue;
            }

            if (this->containsChar('.', buff))
            {
                float value = buff.toFloat();
                if (c == '-')
                {
                    value = value * -1;
                }
                this->insertValue(value, value_hold[i], max);
            }
            else
            {
                float value = buff.toInt();
                if (c == '-')
                {
                    value = value * -1;
                }
                this->insertValue(value, value_hold[i], max);
            }
        }
    }
}

size_t Utils::skipMultiple(unsigned int size, size_t maxVal , unsigned int threshold) {
    size_t start = 1;
    for (size_t i = 1; i < maxVal; i++)
    {
        unsigned int test = size * i;
        float multiple = (float)test / threshold;
        if (multiple >= 1)
        {
            start = i;
            break;
        }
    }
    return start; 
}

int Utils::simCallback(int type, const char *buf, int len, char *value)
{
    if ((type == TYPE_PLUS) && value)
    {
        // if (sscanf(buf, "\r\n+CCID: %[^\r]\r\n", value) == 1)
        /*nothing*/;
        //   Log.info("HEH>>>>>");
    }
    // Log.info("GOT ME THIS TYPE %d", type);
    // Log.info("GOT ME THIS BUF %s", buf);
    // Log.info("GOT ME THIS LENGTH %d", len);
    // Log.info("GOT ME THIS VALUE %s", value);
    return WAIT;
}

bool Utils::connected()
{
    return Particle.connected() || Cellular.ready();
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
