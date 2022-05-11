#include "relay.h"
/**
 *
 * This is the primary api for the Relay classes. All public attributes must be inherited by the
 * Relay object, even of they provide no operational function to the working of the object. Just
 * leave the function blank as below.
 *
 */

/**
 * @deconstructor
 */
Relay::~Relay()
{
    init();
}

/**
 * @constructor
 */
Relay::Relay(Bootstrap *boots)
{
    init();
}

/**
 * @constructor
 */
Relay::Relay(int pin)
{
    this->pin = pin;
    init();
}

bool Relay::on()
{
    digitalWrite(pin, HIGH);
    return true;
}

bool Relay::off()
{
    digitalWrite(pin, LOW);
    return false;
}

/**
 * @public
 *
 * publish
 *
 * Called during a publish event
 *
 * @return void
 */
void Relay::publish(JSONBufferWriter &writer, uint8_t attempt_count)
{
}

/**
 * @public
 *
 * restoreDefaults
 *
 * Called when defaults should be restored
 *
 * @return void
 */
void Relay::restoreDefaults()
{
}

/**
 * @public
 *
 * name
 *
 * Returns the Relay name
 * @return String
 */
String Relay::name()
{
    return "";
}

/**
 * @public
 *
 * paramCount
 *
 * Returns the number of params returned
 *
 * @return uint8_t
 */
uint8_t Relay::paramCount()
{
    return 0;
}

/**
 * @public
 *
 * matenanceCount
 *
 * Is the Relay functional
 *
 * @return uint8_t
 */
uint8_t Relay::matenanceCount()
{
    return 0;
}

/**
 * @public
 *
 * read
 *
 * Called during a read event
 *
 * @return void
 */
void Relay::read()
{
}

/**
 * @public
 *
 * loop
 *
 * Called during a loop event
 *
 * @return void
 */
void Relay::loop()
{
}

/**
 * @public
 *
 * clear
 *
 * Called during a clear event
 *
 * @return void
 */
void Relay::clear()
{
}

/**
 * @public
 *
 * print
 *
 * Called during a print event
 *
 * @return void
 */
void Relay::print()
{
}

/**
 * @public
 *
 * init
 *
 * Called at setup
 *
 * @return void
 */
void Relay::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}
/**
 * @public
 *
 * buffSize
 *
 * Returns the payload size the Relay requires for sending data
 *
 * @return size_t
 */
size_t Relay::buffSize()
{
    return 0;
}
