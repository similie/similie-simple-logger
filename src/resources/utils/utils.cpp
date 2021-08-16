#include "utils.h"

static bool debugValue = false; 

/**
* @deconstructor
*/
Utils::~Utils()
{
}

/**
 * @constructor
 */
Utils::Utils()
{
}

/**
 * @public 
 * 
 * parseCloudFunctionDouble
 * 
 * Parses a string into a double. Used for parse cloud functions
 * that are a string into the double value
 * 
 * @param String value - the cloud value
 * @param String name - the name of the device requesting
 * 
 * @return double
 * 
 */
double Utils::parseCloudFunctionDouble(String value, String name)
{
    const char *stringCal = value.c_str();
    double val = ::atof(stringCal);
    Utils::log("SETTING_CALIBRATION_" + name, "value: " + String(stringCal));
    return val;
}

/**
 * @public 
 * 
 * requestDeviceId
 * 
 * Returns a concatenated String that contains the request identity, and
 * command intended over the serial bus
 * 
 * @param int identity - devices specific identity
 * @param String name - the command being sent over the serial bus
 * 
 * @return String
 * 
 */
String Utils::requestDeviceId(int identity, String cmd)
{
    return "request_" + String(identity) + " " + cmd;
}

/**
 * @public 
 * 
 * receiveDeviceId
 * 
 * Applies the request identity for pulled data off a serialbus
 * for devices with assinged an identity.
 * 
 * @param int identity - devices specific identity
 * 
 * @return String 
 * 
 */
String Utils::receiveDeviceId(int identity)
{
    return "result_" + String(identity);
}

/**
 * @public 
 * 
 * serialMesssageHasError
 * 
 * Checks to see if there is an error on the serial bus for a specific device
 * 
 * @param String message - the message off the serial bus
 * @param int identity - devices specific identity
 * 
 * @return String
 * 
 */
bool Utils::serialMesssageHasError(String message, int identity)
{
    bool error = false;
    if (message.startsWith("ERROR_" + String(identity)))
    {
        String errorMessage = " FOR " + String(identity) + " WITH MESSAGE:: " + message.replace("ERROR_" + String(identity), "");
        this->log("SERIAL_BUS_ERROR", errorMessage);
        error = true;
    }
    return error;
}

/**
 * @public 
 * 
 * hasSerialIdentity
 * 
 * Checks an ID to see if has been registered with a value greater than -1 (default)
 *
 * @param int identity - devices specific identity
 * 
 * @return bool
 * 
 */
bool Utils::hasSerialIdentity(int identity)
{
    return identity > -1;
}

/**
 * @public 
 * 
 * inValidMessageString
 * 
 * Checks to see if a serial payload is valid for storage
 *
 * @param String message - the message off the serial bus
 * @param int identity - devices specific identity
 * 
 * @return bool
 * 
 */
bool Utils::inValidMessageString(String message, int identity)
{
    return this->serialMesssageHasError(message, identity) ||
           (this->hasSerialIdentity(identity) &&
            !message.startsWith(receiveDeviceId(identity)));
}

/**
 * @public 
 * 
 * getTimePadding
 * 
 * Prefixes milliseconds with zero values for 10 bytes. Used in logging
 * 
 * @return String - the milliseconds since boot padded with zeros
 * 
 */
String getTimePadding()
{
    String time = String(millis());
    uint8_t pad = 10 - time.length();
    String padding = "";
    for (uint8_t i = 0; i < pad; i++)
    {
        padding += "0";
    }

    return padding + time;
}

/**
 * @public 
 * 
 * validConfigIdentity
 * 
 * All stored config should have a param version set to 1
 * 
 * @return uint8_t identity - the version value of the config 
 * 
 */
bool Utils::validConfigIdentity(uint8_t identity)
{
    return identity == 1;
}

/**
 * @public 
 * 
 * log
 * 
 * Logs messages to the console
 * 
 * @todo Use a template variant to accept parameters other
 * than a String value
 * 
 * @param String event - the context for the logging
 * @param String message - the actual message
 * 
 * @return void
 * 
 */
// template<typename T>
void Utils::log(String event, String message)
{
    /**
     * For devices we deploy, we can shutdown this logging attribute
     * Value is found under bootstrap.h
     */
    if (!debugValue)
    {
        return;
    }

    Serial.print(getTimePadding());
    Serial.print(" [SIMILIE] " + event + ": ");
    Serial.println(message);
}

/**
 * @public 
 * 
 * log
 * 
 * Wrapper to log with error code
 * 
 * @todo Use a template variant to accept parameters other
 * than a String value
 * 
 * @param String event - the context for the logging
 * @param String message - the actual message
 * @param int errorCode - any code you want to slip in
 * 
 * @return int
 * 
 */
int Utils::log(String event, String message, int errorCode)
{
    log(event, message);
    return errorCode;
}

/**
* @private public
*
* machineName
*
* converts a string to an long for searachble
* EEPROM storage
* @param String name - the device name
* @return unsigned long 
*/
unsigned long Utils::machineName(String name, bool unique)
{
    unsigned long manchineName = 0;
    uint16_t multiple = unique ? name.length() : 1;
    for (uint8_t i = 0; i < name.length(); i++)
    {
        char c = name.charAt(i);
        manchineName += (unsigned long)c * multiple;
    }
    return manchineName;
}

/**
* @private public
*
* machineNameDirect
*
* converts a string to an long for searachble
* EEPROM storage
* @param String name - the device name
* @param byte * restore
*
* @return unsigned long 
*/
void Utils::machineNameDirect(String name, byte * save)
{
    name.getBytes(save, name.length() + 1);
}

/**
* @private public
*
* machineToReadableName
* 
* Reverts manchine name values back to a string 
* 
* @param byte * restore
* @return String 
*/
String Utils::machineToReadableName(byte * restore)
{
    String send = "";

    byte b = 255;
    uint8_t i = 0;
    while (b != 0 && i < DEVICE_BYTE_BUFFER_SIZE)
    {
        b = restore[i];
        
        char c = (char)b;
        send += String(c);
        i++;
    }

    return send;
}

/**
 * @public 
 * 
 * containsValue
 * 
 * Does the array contain a specifig string value
 * 
 * @param String arr[] - the array
 * @param size_t size - array size
 * @param String value - the value to search
 * 
 * @return bool
 * 
 */
int Utils::containsValue(String arr[], size_t size, String value)
{
    int has = -1;
    for (size_t i = 0; i < size; i++)
    {
        String check = arr[i];
        if (check.equals(value))
        {
            has = i;
            break;
        }
    }
    return has;
}

/**
 * @public 
 * 
 * parseSerial
 * 
 * Parses serial strings sent over the serial bus for Meter SDI-12 devices
 * 
 * @param String ourReading - the serial details
 * @param size_t paramLength - the number of parameters to parse
 * @param size_t max - the max value for the valuehold array
 * @param float value_hold[][Bootstrap::OVERFLOW_VAL] - a multi-dimentional array holding the values
 * 
 * @return void
 * 
 */
void Utils::parseSerial(String ourReading, size_t paramLength, size_t max, float value_hold[][Bootstrap::OVERFLOW_VAL])
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

/**
 * @public 
 * 
 * skipMultiple
 * 
 * Some devices don't want or cant process a read on every interval. This sets values
 * where a read is acceptable
 * 
 * @param unsigned int size - the size requested from a 15 interval
 * @param size_t maxVal - the max interval, normally 15
 * @param unsigned int threshold - max to do in a given interval
 * 
 * @return size_t 
 * 
 */
size_t Utils::skipMultiple(unsigned int size, size_t maxVal, unsigned int threshold)
{
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

/**
 * @public 
 * 
 * simCallback
 * 
 * Callback function used during beach mode to talk to the sym. 
 * 
 * @todo add functionality
 * 
 * @param int type
 * @param const char *buf
 * @param int len
 * @param char *value
 * 
 * @return size_t 
 * 
 */
int Utils::simCallback(int type, const char *buf, int len, char *value)
{
    if ((type == TYPE_PLUS) && value)
    {
        // if (sscanf(buf, "\r\n+CCID: %[^\r]\r\n", value) == 1)
        /*nothing*/;
    }
    // Log.info("GOT THIS TYPE %d", type);
    // Log.info("GOT THIS BUF %s", buf);
    // Log.info("GOT THIS LENGTH %d", len);
    // Log.info("GOT THIS VALUE %s", value);
    return WAIT;
}

/**
 * @public 
 * 
 * connected
 * 
 * Wraps system functions to check of the system is online
 * 
 *
 * @return bool 
 * 
 */
bool Utils::connected()
{
    return Particle.connected() || Cellular.ready();
}

/**
 * @public 
 * 
 * getSum
 * 
 * Sums all values in a float array
 * 
 * @param float values[] - the values to sum
 * @param size_t MAX - the size of the array to scan
 *
 * @return float 
 * 
 */
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

/**
 * @public 
 * 
 * getMax
 * 
 * Gets the max value of an array
 * 
 * @param float values[] - the values to sum
 * @param size_t MAX - the size of the array to scan
 *
 * @return float 
 * 
 */
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

/**
 * @public 
 * 
 * getMedian
 * 
 * Get the middle value or the value toward the zero index
 * this is not a NO_VALUE
 * 
 * @param float values[] - the values to sum
 * @param size_t MAX - the size of the array to scan
 *
 * @return float 
 * 
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

/**
 * @public 
 * 
 * containsChar
 * 
 * Does the String contain a specific character
 * 
 * @param char c - the character
 * @param String readFrom - the read string
 *
 * @return bool 
 * 
 */
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

/**
 * @public 
 * 
 * invalidNumber
 * 
 * Is the string an invalid number representation
 * 
 * @param String value - the read string
 *
 * @return bool 
 * 
 */
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

/**
* @public
* 
* shift
* 
* Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param float value - the value to be inserted 
* @param size_t index - the index where to insert;
* @param float arr[] - the array to insert into     
* @param size_t size - size of the array
*
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

/**
* @public
*
* shift 
*
* Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param int value - the value to be inserted 
* @param size_t index - the index where to insert;
* @param int arr[] - the array to insert into     
* @param size_t size - size of the array
*
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

/**
* @public
*
* shift 
*
* Helper fuction that inserts an element in asscending order into
* the sorted array where we can our median value
* @param long value - the value to be inserted 
* @param size_t index - the index where to insert;
* @param long arr[] - the array to insert into     
* @param size_t size - size of the array
*
* @return void;
*/
void Utils::shift(long value, size_t index, long arr[], size_t size)
{

    long last = arr[index];
    arr[index] = value;
    index++;

    while (index < size)
    {
        long temp = arr[index];
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

/**
* @public
*
* insertValue 
*
* Insert a value into a multidimentional array in sorted order
*
* @param float value - the value to be inserted 
* @param int param - the enum value that represents the param
* @param size_t size - the size of the array
*
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

/**
* @public
* 
* insertValue  
*
* Insert a value into a multidimentional array in sorted order
*
*
* @param int value - the value to be inserted 
* @param int param - the enum value that represents the param
* @param size_t size - the size of the array
*
* @return void;
*/
void Utils::insertValue(long value, long arr[], size_t size)
{
    size_t index = 0;

    long aggr = arr[index];

    while (value >= aggr && aggr != NO_VALUE && index < size)
    {
        index++;
        aggr = arr[index];
    }
    shift(value, index, arr, size);
}

/**
* @public
* 
* insertValue  
*
* Insert a value into a multidimentional array in sorted order
*
*
* @param int value - the value to be inserted 
* @param int param - the enum value that represents the param
* @param size_t size - the size of the array
*
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


/**
 * @public 
 * 
 * getMedian
 * 
 * Get the middle value or the value toward the zero index
 * this is not a NO_VALUE
 * 
 * @param long readparam - the expected max value
 * @param long arr[] - the array of values
 * 
 * @return long
 * 
 */
long Utils::getMedian(long readparam, long arr[])
{
    double center = (double)readparam / 2.0;
    int index = ceil(center);
    long value = NO_VALUE;
    while (value == NO_VALUE && index >= 0)
    {
        value = arr[index];
        index--;
    }
    return value;
}


/**
 * @public 
 * 
 * getMedian
 * 
 * Get the middle value or the value toward the zero index
 * this is not a NO_VALUE
 * 
 * @param int readparam - the expected max value
 * @param int arr[] - the array of values
 * 
 * @return int
 * 
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

/**
 * @public
 * 
 * stringConvert 
 * 
 * converts string to a char *
 *
 * @param: String value - to be converted 
 * 
 * @return const * char 
 * 
 */
const char *Utils::stringConvert(String value)
{
    return value.c_str();
}

/**
 * @public
 * 
 * reboot 
 * 
 * Wrapper to reboot the system
 * 
 * @return void
 * 
 */
void Utils::reboot()
{
    Log.info("Reboot Event Requested");
    System.reset();
}


/**
 * @public
 * 
 * setDebug
 * 
 * turns on and off system logging
 * 
 * @return void
 * 
 */
void Utils::setDebug(bool debug)
{   
    debugValue = debug;
}