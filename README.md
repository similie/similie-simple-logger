# SimilieLogger

This project is the primary firmware for running our field-based data loggers using Particle's IoT architecture. The software has been testing using Particle's 3.0 and 3.1-based firmware. It may compile with older versions, but we have found problems running the firmware with 3rd-party SIM cards. Additionally the software is intended for Particle's 3rd-gen Boron/Argon Series and untested with other generatations. For devices requiring serial communications such as the Atmos41 or Terros 10/11, please see our repository [similie/sdi12-allweather-interface](https://github.com/similie/sdi12-allweather-interface)

## Basic Architecture Overview

The below image provides the basic visual representation for the architectural overview. The device-manager class works with the Device API and manages the correspending payloads. The processor classes send the payloads over the internet. Bootstrap manages read/publish timings and the associated cloud configuration.

![Similie Logger Architecture](/LoggerArchitecture.svg)

## Device Config

You can configure the software in the device-manager constructor `/src/resources/devices/device-manager.cpp`. The following provides an example configuration with one AllWeather device, a SoilMoisture device, and the default Battery device.

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

#### `/src` folder:

This is the source folder that contains the firmware files for your project. It should _not_ be renamed.
Anything that is in this folder when you compile your project will be sent to the Particle compile service and compiled into a firmware binary for the device that you have targeted.

If your application contains multiple files, they should all be included in the `src` folder. If your firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### `.ino` file:

This file is the firmware that will run as the primary application on your Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### `project.properties` file:

This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

#### Projects with external libraries

We use CellularHelper for our HeartBeat class. It is a simple library that analyzes the SIM/Cellular details for the Boron-class products. It should not be used if the solution runs with the Argon-line of devices. Additionally, we can use other libraries for our processors or devices if required. However, the default solution only requires CellularHelper to send periodic heartbeat details.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`
