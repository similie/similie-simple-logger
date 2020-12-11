#include "serial-gps.h"

Adafruit_GPS GPS(&GPSSerial);

SerialGps::~SerialGps()
{
}
SerialGps::SerialGps(Bootstrap *boots)
{
    this->boots = boots;
}

SerialGps::SerialGps()
{
}

void SerialGps::publish(JSONBufferWriter &writer, u8_t attempt_count)
{
}

u8_t SerialGps::paramCount()
{
    return 0;
}

u8_t SerialGps::matenanceCount()
{
    return 0;
}

void SerialGps::read()
{
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4);
    Serial.print(GPS.lat);
    Serial.print(", ");
    Serial.print(GPS.longitude, 4);
    Serial.println(GPS.lon);
    Serial.print("Speed (knots): ");
    Serial.println(GPS.speed);
    Serial.print("Angle: ");
    Serial.println(GPS.angle);
    Serial.print("Altitude: ");
    Serial.println(GPS.altitude);
    Serial.print("Satellites: ");
    Serial.println((int)GPS.satellites);
}

void SerialGps::loop()
{
    // read data from the GPS in the 'main loop'
    char c = GPS.read();

    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
        if (c)
            Serial.print(c);
    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived())
    {
        // a tricky thing here is if we print the NMEA sentence, or data
        // we end up not listening and catching other sentences!
        // so be very wary if using OUTPUT_ALLDATA and trying to print out data
        Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
        if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
            return;                     // we can fail to parse a sentence in which case we should just wait for another
    }
}

void SerialGps::clear()
{
}

void SerialGps::print()
{
}

void SerialGps::init()
{

    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    // uncomment this line to turn on only the "minimum recommended" data
    //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
    // the parser doesn't care about other sentences at this time

    // Set the update rate
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
    // For the parsing code to work nicely and have time to sort thru the data, and
    // print it out we don't suggest using anything higher than 1 Hz

    // Request updates on antenna status, comment out to keep quiet
    GPS.sendCommand(PGCMD_ANTENNA);

    delay(1000);

    // Ask for firmware version
    GPSSerial.println(PMTK_Q_RELEASE);
}

size_t SerialGps::buffSize()
{
    return 300;
}
