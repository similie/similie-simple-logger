#include "all-weather.h"
/**
 * @description
 *
 * Works the Atmos 41 all-in-one weather sensor from Meter.
 * https://www.metergroup.com/environment/products/atmos-41-weather-station/
 *
 * Since particle does not support SDI-12, we use a 32u4/SAMD co-processor.
 * https://www.adafruit.com/product/2796
 * The source code we use can be found: https://github.com/similie/sdi12-allweather-interface
 *
 */

/**
 * default constructor
 * @param Bootstrap boots - bootstrap object
 */

AllWeather::AllWeather(Bootstrap *boots) : SDI12Device(boots)
{
    setElements(&elements);
}

/**
 * constructor
 * @param Bootstrap boots - bootstrap object
 * @param int identity - numerical value used to idenify the device
 */
AllWeather::AllWeather(Bootstrap *boots, int identity) : SDI12Device(boots, identity)
{
    setElements(&elements);
}
