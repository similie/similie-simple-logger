#include "adafruit-ina219.h"
#include "Arduino.h"
//#include <arduino.h>
#include <stdlib.h>
#include <ArduinoJson.h>

#define JBUF_JSON_SIZE 450
#define PW_PIN D8

String config_hook = "Ai_Config_" + System.deviceID();
String log_hook = "Ai_Post_" + System.deviceID();
// String log_hook = "Ai_Post_AWS";
String post_hook = "Ai_Post_AWS";
String debug_hook = "Ai_Debug_" + System.deviceID();
String logging_hook = "Ai_Log_" + System.deviceID();

Adafruit_INA219 ina219(0x40);
const int BATTERY_MAX = 420;
const int BATTERY_MIN = 320;
const int BATTERY_DIVISOR = 100;

const int debugThreshold = 5;
const int MINUTE_INCREMENTS = 6;
const int READ_BUF_SIZE = 116;

int SEND_READ = 90;
bool readyRead = false;
bool rebootFlag = false;

int READ_MULTIPLE = 1;
int URBAN_THRESHOLD = 0;

const float ZERO_VALUE = -9999.99;
const double DISTANCE_READ_CALIBRATION = 0.01724137931;

long TICK = 0;
int TOCK = 1;

const size_t CAPACITY = JSON_OBJECT_SIZE(1);

StaticJsonDocument<JBUF_JSON_SIZE> doc;
JsonObject readData = doc.to<JsonObject>();

String ourReading = "";
int counter = 0;
enum
{
    solar,
    precipitation,
    strikes,
    strike_distance,
    wind_speed,
    wind_direction,
    gust_wind_speed,
    air_temperature,
    vapor_pressure,
    atmospheric_pressure,
    relative_humidity,
    humidity_sensor_temperature,
    x_orientation,
    y_orientation,
    null_val,
    wind_speed_north,
    wind_speed_east,
    hydrometric_level
};

enum
{
    water_level,
    read_interval,
    bat_volts,
    bat_percent
};

String valueMapExtras[] =
    {
        "wl",
        "r_int",
        "bat_v",
        "bat_p"
        //"curr"
};

const int V_MAP_EX_SIZE = 4;
const int SEND_READ_MAX = 90;
const int PARAM_SIZE = 17;
//hydrometric_level
String valueMap[] =
    {
        "sol",
        "precip",
        "strks",
        "str_d",
        "ws",
        "wd",
        "g_ws",
        "temp",
        "pres",
        "ap",
        "hum",
        "s_s_t",
        "x_or",
        "y_or",
        "null",
        "ws_n",
        "ws_e"};

float aggregateValues[PARAM_SIZE][SEND_READ_MAX];
Timer timeBoom(10000, boomoThatSerial);
void setup()
{

    while (!Serial1)
        ;
    timeBoom.start();
    Serial.begin(115200);
    Serial1.begin(9600);
    pinMode(PW_PIN, INPUT);
    Serial.println("COMING TO LIFE");
    clearValues();

    Particle.variable("SEND_READ", READ_MULTIPLE);
    Particle.variable("URBAN_THRESHOLD", URBAN_THRESHOLD);
    Particle.variable("TOCK", TOCK);

    Particle.function("setUrbanThreshold", setUrbanThreshold);
    Particle.function("setSendRead", setSendRead);
    Particle.function("setTock", setTock);
    Particle.function("reboot", reboot);
    SEND_READ = READ_MULTIPLE * MINUTE_INCREMENTS;
    ina219.begin();
}

void boomoThatSerial()
{
    Serial.println("SENDING SOME BOOMO");
    Serial1.println("0R0!");
}

void log(String message)
{
    Particle.publish(logging_hook, message, PRIVATE);
}

float getVolts()
{
    float v = ina219.getBusVoltage_V();
    if (v > 32)
    {
        return -1;
    }
    return v;
}
float getCurrent()
{
    return ina219.getCurrent_mA();
}

int getBatt(float volts)
{
    if (volts < 0)
    {
        return volts;
    }
    volts *= BATTERY_DIVISOR;
    float percentVoltate = (volts > BATTERY_MAX || volts < BATTERY_MIN) ? (volts > BATTERY_MAX ? BATTERY_MAX : BATTERY_MIN) : volts;
    int percent = percentVoltate - BATTERY_MIN;
    return percent;
}

int reboot(String __)
{

    rebootFlag = true;
    return 1;
}
int setTock(String threshold)
{
    int val = (int)atoi(threshold);
    if (val < 0)
    {
        return 0;
    }
    TOCK = val;
    return val;
}

int setUrbanThreshold(String threshold)
{
    int val = (int)atoi(threshold);
    if (val < 0 || val > floor((READ_MULTIPLE * MINUTE_INCREMENTS - 1) / 2))
    {
        return 0;
    }
    URBAN_THRESHOLD = val;
    return val;
}

int setSendRead(String read)
{
    int val = (int)atoi(read);
    if (val <= 0 || val >= SEND_READ)
    {
        return 0;
    }
    READ_MULTIPLE = val;
    SEND_READ = (int)READ_MULTIPLE * MINUTE_INCREMENTS;
    return val;
}

char buffer[READ_BUF_SIZE];
void loop()
{
    readSerial(buffer);
    print();

    if (rebootFlag)
    {
        delay(2000);
        System.reset();
    }
}
void shift(float value, int index, int param)
{
    float last = aggregateValues[param][index];
    aggregateValues[param][index] = value;
    index++;
    while (index < SEND_READ)
    {
        float temp = aggregateValues[param][index];
        aggregateValues[param][index] = last;
        index++;
        if (index < SEND_READ)
        {
            last = aggregateValues[param][index];
            aggregateValues[param][index] = temp;
            index++;
        }
    }
}
void insertValue(float value, int param)
{
    int index = 0;
    float aggr = aggregateValues[param][index];
    while (value >= aggr && aggr != ZERO_VALUE && index < SEND_READ)
    {
        index++;
        aggr = aggregateValues[param][index];
    }
    shift(value, index, param);
}

long readPW()
{
    long timeout = 1000;

    long pw = 0;
    int count = 0;
    int doCount = 5;
    long lastTime = millis();
    for (int i = 0; i < doCount; i++)
    {
        long currentTime = millis();
        long read = pulseIn(PW_PIN, HIGH);
        count++;
        pw += read;
        // break off it it is taking too long
        if (currentTime - lastTime > timeout)
        {
            break;
        }
        lastTime = millis();
    }

    pw /= count;

    return pw * DISTANCE_READ_CALIBRATION;
}
bool containsChar(char c, String readFrom)
{
    boolean contains = false;
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
void printValues(int index)
{
    Serial.print(valueMap[index]);
    Serial.print(" ");
    for (int i = 0; i < SEND_READ; i++)
    {
        float value = aggregateValues[index][i];
        Serial.print(value);
        Serial.print(" ");
    }
    Serial.println("");
}
void printValues()
{
    for (int i = 0; i < PARAM_SIZE; i++)
    {
        Serial.print(valueMap[i]);
        Serial.print(" ");
        for (int j = 0; j < SEND_READ; j++)
        {
            float value = aggregateValues[i][j];
            Serial.print(value);
            Serial.print(" ");
        }
        Serial.println(" / ");
    }
}

void clearValues()
{
    for (int i = 0; i < PARAM_SIZE; i++)
    {
        for (int j = 0; j < SEND_READ_MAX; j++)
        {
            aggregateValues[i][j] = ZERO_VALUE;
        }
    }
}

float getSum(float values[], int MAX)
{
    int sum = 0;
    const int THRSHOLD_VAL = 100;
    const int DIVISOR = 100;
    const int THRESHOLD = DIVISOR * THRSHOLD_VAL;
    bool hasValue = false;
    int MIN = 0 + URBAN_THRESHOLD;
    MAX = MAX > SEND_READ ? SEND_READ : MAX;
    int last = 0;
    for (int i = MIN; i < MAX; i++)
    {
        if (values[i] == ZERO_VALUE)
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
        return 0;
    }

    return (float)sum / DIVISOR;
}

float getMax(float values[], int MAX)
{
    float value = 0;
    int MIN = 0 + URBAN_THRESHOLD;
    int i = MAX;
    while (i > MIN)
    {

        value = values[i - 1];
        if (value != ZERO_VALUE)
        {
            return value;
        }
        i--;
    }
    return value;
}

void extractValue(float values[], int key)
{
    const int SPLIT = ceil(SEND_READ / 2);
    int MAX = SEND_READ - URBAN_THRESHOLD;
    String param = valueMap[key];

    if (param == NULL || param == " " || param == "")
    {
        param = "_FAILURE_";
    }

    switch (key)
    {
    case gust_wind_speed:
        readData[param] = getMax(values, MAX);
        break;
    case precipitation:
        readData[param] = getSum(values, MAX);
    case strikes:
        readData[param] = getSum(values, MAX);
        break;
    default:
        readData[param] = getMax(values, SPLIT);
    }
}

void packagePayload()
{
    for (int i = 0; i < PARAM_SIZE; i++)
    {
        extractValue(aggregateValues[i], i);
    }
}

bool validateAvailable()
{
    int goodCount = 0;
    for (int i = 0; i < (int)PARAM_SIZE; i++)
    {
        if ((int)readData[valueMap[i]] >= (int)ZERO_VALUE)
        {
            goodCount++;
        }
    }
    return goodCount < debugThreshold;
}

void clearRead()
{
    for (int i = 0; i < PARAM_SIZE; i++)
    {
        readData[valueMap[i]] = NULL;
    }

    for (int i = 0; i < V_MAP_EX_SIZE; i++)
    {
        readData[valueMapExtras[i]] = NULL;
    }
    //readData = doc.to<JsonObject>();
}

void postData()
{
    Serial.println("PRINTVALS");
    printValues();
    Serial.println("PACKAGE");
    packagePayload();
    Serial.println("CLEAR");
    clearValues();
    Serial.println("WL ");
    long wl = readPW();
    if (wl >= 0)
    {
        readData[valueMapExtras[water_level]] = wl;
    }
    Serial.print("TICK ");
    Serial.println(TICK);
    readData[valueMapExtras[read_interval]] = TICK;
    Serial.println("BEFORE VOLTS ");
    float volts = getVolts();
    if (volts >= 0)
    {
        //float current = getCurrent();
        int percent = getBatt(volts);
        readData[valueMapExtras[bat_volts]] = volts;
        readData[valueMapExtras[bat_percent]] = percent;
        //readData["curr"] = current;
    }
    Serial.print("VOLTS ");
    Serial.println(volts);
    bool debug = validateAvailable();
    Serial.print("DEBUG ");
    Serial.println(debug);

    char jsonPrintBuf[JBUF_JSON_SIZE];

    serializeJsonPretty(doc, jsonPrintBuf);

    Serial.print("Current Read ");
    Serial.println(jsonPrintBuf);

    if ((TOCK && (TICK < (long)TOCK)) || debug)
    {
        Debug(jsonPrintBuf);
    }
    else
    {
        POST(jsonPrintBuf);
    }
    TICK++;
    clearRead();
}

void POST(char jsonPrintBuf[])
{

    Particle.publish(post_hook, jsonPrintBuf, PRIVATE);
}

void Debug(char jsonPrintBuf[])
{

    Particle.publish(debug_hook, jsonPrintBuf, PRIVATE);
}

bool invalidNumber(String value)
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

void print()
{
    if (readyRead)
    {
        Serial.println(ourReading);
        readyRead = false;
        //float values[17];
        int j = 1;
        // we had paramsize +1
        for (int i = 0; i < PARAM_SIZE; i++)
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
                if (invalidNumber(buff))
                {
                    continue;
                }

                if (containsChar('.', buff))
                {
                    float value = buff.toFloat();
                    if (c == '-')
                    {
                        value = value * -1;
                    }
                    insertValue(value, i);
                }
                else
                {
                    float value = buff.toInt();
                    if (c == '-')
                    {
                        value = value * -1;
                    }
                    insertValue(value, i);
                }
            }
        }
        counter++;
        Serial.print("COUNT ");
        Serial.println(counter);

        if (counter == SEND_READ)
        {
            counter = 0;
            postData();
        }
        else if (counter > SEND_READ)
        {
            counter = 0;
        }
        //postData();

        ourReading = "";
    }
}

int readSerial(char result[])

{
    int avail = Serial1.available();
    for (int i = 0; i < avail; i++)
    {
        char inChar = Serial1.read();
        if (inChar == '\n' || inChar == '\0')
        {
            readyRead = true;
            return 1;
        }
        if (inChar != '\r')
        {
            ourReading += String(inChar);
        }
    }

    return 0;
}