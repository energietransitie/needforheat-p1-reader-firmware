# Twomes P1 Reader
This [Twomes P1 Reader](https://github.com/energietransitie/twomes-p1-reader-firmware) firmware used on the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware) reads data from the P1 port of smart meters adhering to the [DSMR standards](https://www.netbeheernederland.nl/dossiers/slimme-meter-15/documenten) and occupancy (i.e. the number of smartphones of participating subjects) via Bluetooth name requests.

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
This measurement device firmware uses the [Generic Firmware for Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware) with presence detection enabled. Contains an improved and updated way to read data from a p1 port compared to the  [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware) repository. 


## Measurements

The Twomes P1 Reader, in addition to [generic data sent by any Twomes measurement device](https://github.com/energietransitie/twomes-generic-esp-firmware#readme), sends data about the following property via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) to a Twomes server:

| Sensor | Property           | Unit | [Printf format](https://en.wikipedia.org/wiki/Printf_format_string) | Default measurement interval \[h:mm:ss\] | Description                            |
|--------|--------------------|------|--------|-------------------|----------------------------------------|
| P1 port   | `e_use_lo_cum__kWh`         | kWh   | %4.3f   | 0:05:00           | electricity meter reading                        |
| P1 port   | `e_use_hi_cum__kWh`         | kWh   | %4.3f   | 0:05:00           | electricity meter reading                        |
| P1 port   | `e_ret_lo_cum__kWh`         | kWh   | %4.3f   | 0:05:00           | electricity meter reading                        |
| P1 port   | `e_ret_hi_cum__kWh`         | kWh   | %4.3f   | 0:05:00           | electricity meter reading                        |
| P1 port   | `g_use_cum__m3`         |  m<sup>3</sup>  | %7.3f   | 0:05:00           | gas meter reading                        |
| Bluetooth  | `occupancy__p`         | [-]   | %u   | 0:10:00           | number of smartphones responding to Bluetooth name request                        |

## Deploying
To deploy this software, see the [deploying section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/deploying/prerequisites/). The firmware needed can be found as a [release from this repository](https://github.com/energietransitie/twomes-p1-reader-firmware). 

## Developing
To develop software for, or based on this software, see the [developing section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/starting/prerequisites/)

## Supported devices
This example was tested on:
- [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware)

## Features
List of features ready and TODOs for future development (other than the [features of the generic Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#features)). 

Ready:
* Read data from the P1 port of devices adhering to DSMRv4 and DSMRv5 (UART settings 115200/8N1).
* Read data from the P1 port of devices adhering to DSMRv2 and DSMRv3 (UART settings 9600/7E1).
* Indicate status and error via LEDs.
* Reset Wi-Fi provisioning by a long press (>10s) of the button `K` (this button is labeled `GPIO12 (SW3)` on the PCB). 

To-do:
* Research and implement an improved way for presence detection.

## Status
Project is: _in progress_

## License
This software is available under the [Apache 2.0 license](./LICENSE), Copyright 2022 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software was created by:
* Joël van de Weg · [@JoelvdWeg](https://github.com/JoelvdWeg)

... with help from the following persons for laying the ground work (see legacy branch for their contributions):
* Sjors Smit ·  [@Shorts1999](https://github.com/Shorts1999)
* Fredrik-Otto Lautenbag ·  [@Fredrik1997](https://github.com/Fredrik1997)
* Gerwin Buma ·  [@GerwinBuma](https://github.com/GerwinBuma) 
* Werner Heetebrij ·  [@Werner-Heetebrij](https://github.com/Werner-Heetebrij)
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

... and with help from the following persons for bugfixes:
* Nick van Ravenzwaaij · [@n-vr](https://github.com/n-vr)

Product owners:
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

We use and gratefully acknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache License 2.0](https://github.com/espressif/esp-idf/blob/9d34a1cd42f6f63b3c699c3fe8ec7216dd56f36a/LICENSE)
* [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware), by [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie), licensed under [Apache License 2.0](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/LICENSE.md)
