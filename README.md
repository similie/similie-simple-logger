# Similie AiLogger

This project is the primary firmware for running our field-based data loggers using Particle's IoT architecture. The software has been testing using Particle's 3.0 and 3.1-based firmware. It may compile with older versions, but we have found problems running the firmware with 3rd-party SIM cards. Additionally the software is intended for Particle's 3rd-gen Boron/Argon Series and untested with other generatations. For devices requiring serial communications such as the Atmos41 or Terros 10/11, please see our repository [similie/sdi12-allweather-interface](https://github.com/similie/sdi12-allweather-interface)

## Basic Architecture Overview

The below image provides the basic visual representation for the architectural overview. The device-manager class works with the Device API and manages the correspending payloads. The processor classes send the payloads over the internet. Bootstrap manages read/publish timings and the associated cloud configuration.

![Similie Logger Architecture](/LoggerArchitecture.svg)

## Device Config

If needing just a single set of devices that compose a single dataset, all configuration can be done through the Particle cloud. Find the `addDevice` function and place your device's string value into input console. The format for the device is in the following form `device:[identity]:[pin]`. The identity and pin are only required for some devices and the pin is optional for others. We arbitrarily set a max device limit of 7, but this can be changed under `bootstrap.h` by altering the `MAX_DEVICES` macro. If new devices are added, there should be a corresponding representation under `configurator.h`. The follow provides a list of available devices:

```
all_weather:0 // pin 10 for the 32u4.
all_weather:1 // pin 11 for the 32u4.
soil_moisture:0 // pin 10 for the 32u4. Two or more should never have the same identity
soil_moisture:1 // pin 11 for the 32u4.
// if both a soil moisture and an all_weather are deployed,
// they should should have differnt identities
rain_gauge
gps
battery // this reads the Particle's fuel gauge
sonic_sensor:0:10 // unless deploying multiple sensors, both the pin and identity are optional

```

To remove a single device, you can select the `showDevices` function without any input parameters. A corresponding cloud event will send a list of all connected devices. Simply copy the full string name and populate the `removeDevice` function with this value. To delete all devices you can use the `clearAllDevices` function with the text `DELETE` in the input field.

In addition to the cloud configurator, you can configure the device suite in the device-manager constructor `/src/resources/devices/device-manager.cpp`. The advantage of using this method is that you can create multiple datasets with your devices. The following provides an example configuration with one AllWeather device, a SoilMoisture device, and the default Battery device.

```
DeviceManager::DeviceManager(Bootstrap *boots, Processor *processor)
{
    ...

    deviceAggregateCounts[ONE_I] = {THREE};// set the number of devices here
    // the numerical N_I values a indexs from 0, 1, 2 ... n
    // unless others needed. Most values will only needs the
    // ONE_I for the first dimension.
    this->devices[ONE_I][one_i] = new AllWeather(boots, ONE_I);
    this->devices[ONE_I][two_i] = new Battery();
    this->devices[ONE_I][three_i] = new SoilMoisture(boots, TWO_I);

    ...
}
```

#### `project.properties` file:

This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

#### Projects with external libraries

We use CellularHelper for our HeartBeat class. It is a simple library that analyzes the SIM/Cellular details for the Boron-class products. It should not be used if the solution runs with the Argon-line of devices. Additionally, we can use other libraries for our processors or devices if required. However, the default solution only requires CellularHelper to send periodic heartbeat details.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`

## License

GNU GENERAL PUBLIC LICENSE V3

Supported by Similie and made with LOVE in Timor-Leste
