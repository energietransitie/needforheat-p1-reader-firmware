# Twomes Presence Detector example

This is a [template project](https://github.com/energietransitie/twomes-presence-detector/generate) that can be used to create firmware for measurement devices that use the [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library.

## Table of contents
* [General info](#general-info)
* [Measurements](#measurements)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Supported devices](#supported-devices)
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
The Twomes Presence Detector primarily serves as an example and template repository how to create firmware that uses the [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library. Nevertheless, it can be used in  measurement campaigns that require occupancy detection using Bluetooth name requests.

## Measurements

A Twomes Presence Detector measurement device, in addition to [generic data sent by any Twomes measurement device](https://github.com/energietransitie/twomes-generic-esp-firmware#readme), sends data about the following property via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) to a Twomes server:

| Sensor | Property           | Unit | [Printf format](https://en.wikipedia.org/wiki/Printf_format_string) | Default measurement interval \[h:mm:ss\] | Description                            |
|--------|--------------------|------|--------|-------------------|----------------------------------------|
| Bluetooth  | `countPresence`         | [-]   | %u   | 0:10:00           | number of smartphones responding to Bluetooth name request                        |

## Deploying
To deploy this software, see the [deploying section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/deploying/prerequisites/). The firmware needed can be found as a [release from this repository](https://github.com/energietransitie/twomes-presence-detector/releases).

## Developing
To develop software for, or based on this software, see the [developing section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/starting/prerequisites/)

## Supported devices
This example was tested on:
- [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk)

This example still needs to be tested n:
- [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3)

## Features
The example features all generic firmware tasks.

Ready:
* Generic firmware tasks using the [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library, including presence detection based on Bluetooth name requests.

## Status
Project is: _in progress_

## License
This software is available under the [Apache 2.0 license](./LICENSE), Copyright 2022 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software was created by:
* Nick van Ravenzwaaij · [@n-vr](https://github.com/n-vr)

Product owners:
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

We use and gratefully acknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache License 2.0](https://github.com/espressif/esp-idf/blob/9d34a1cd42f6f63b3c699c3fe8ec7216dd56f36a/LICENSE)
* [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware), by [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie), licensed under [Apache License 2.0](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/LICENSE.md)
