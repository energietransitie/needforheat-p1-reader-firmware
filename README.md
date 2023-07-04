# Twomes P1 Reader
This repository contains firmware and binary releases that can be installed on the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware). Connected to the P1 port of smart meters adhering to the [DSMR](https://www.netbeheernederland.nl/dossiers/slimme-meter-15/documenten) standards, this device can read smart meter data. It can also register occupancy (i.e. the number of smartphones of participating subjects) via [Bluetooth name requests](https://github.com/energietransitie/twomes-generic-esp-firmware/tree/main/src/presence_detection). All data that is registered will be sent regularly to Twomes server via the [Twomes v2 API](https://github.com/energietransitie/twomes-backoffice-api).

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
This measurement device firmware is built on top of the [Generic Firmware for Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware) with presence detection enabled. It contains an improved and updated way to read data from a P1 port, compared to the earlier [twomes-p1-gateway-firmware](https://github.com/energietransitie/twomes-p1-gateway-firmware). 


## Measurements

The Twomes P1 Reader, in addition to [generic data sent by any Twomes measurement device](https://github.com/energietransitie/twomes-generic-esp-firmware#readme), sends data about the following properties via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) to a Twomes server:

| Sensor | Property           | Unit | [Printf format](https://en.wikipedia.org/wiki/Printf_format_string) | Default measurement interval \[h:mm:ss\] | Description                            |
|--------|--------------------|------|--------|-------------------|----------------------------------------|
| P1 port   | `e_use_lo_cum__kWh`         | kWh   | %.3f   | 0:10:00           | electricity meter reading                        |
| P1 port   | `e_use_hi_cum__kWh`         | kWh   | %.3f   | 0:10:00           | electricity meter reading                        |
| P1 port   | `e_ret_lo_cum__kWh`         | kWh   | %.3f   | 0:10:00           | electricity meter reading                        |
| P1 port   | `e_ret_hi_cum__kWh`         | kWh   | %.3f   | 0:10:00           | electricity meter reading                        |
| P1 port   | `g_use_cum__m3`         |  m<sup>3</sup>  | %.3f   | 0:10:00           | gas meter reading                        |
| Bluetooth  | `occupancy__p`         | [-]   | %u   | 0:10:00           | number of smartphones responding to Bluetooth name request                        |

## Deploying 

### Deploying on [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware)
This section provides instructions on deploying firmware releases on the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware). This deployment method allows you to update the firmware without modifying the source code, without requiring a development environment, and without the need to compile the source code.

#### Prerequisites
To deploy the firmware, in addition to the [generic prerequisites for deploying Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#prerequisites), you need:
* a 3.3V TTL-USB Serial Port Adapter (e.g. [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter), CP210x, etc..), including the cable to connect ths adapter to a free USB port on your computer (a USB to miniUSB cable in the case of a [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter));
* (optional: more stable) Supply 5V DC power to the device via the micro-USB jack of the device.
* Find a row of 6 holes holes (next to the ESP32 on the PCB of the  P1 Gateway), find the `GND` pin (see  bottom of the PCB), align the 6 pins of the serial port adapter such that `GND` and other pins match; then connect the serial port adapter to your computer and connect the 6 pins of the serial port adapter to the 6 holes on the PCB.

#### Device preparation step 1: Uploading firmware

* Download the [binary release for your device](https://github.com/energietransitie/twomes-p1-gateway-firmware/releases) and extract it to a directory of your choice.
* If you used the device before, you shoud first [erase all persistenly stored data](https://github.com/energietransitie/twomes-generic-esp-firmware#erasing-all-persistenly-stored-data).
* Follow the [generic Twomes firmware upload instructions ](https://github.com/energietransitie/twomes-generic-esp-firmware#device-preparation-step-1a-uploading-firmware-to-esp32), with the exceptions mentioned below:
	* When you see the beginning of the sequence `Connecting ......_____......`, press and hold the button labeled `GPIO0 (SW2)` on the PCB, then briefly press the button labeled `RESET`, then release the button labeled `GPIO0 (SW2) `;
	* You should see an indication that the firmware is being written to the device.
	* When the upload is finished, view the serial output with a serial monitor tool like [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) or the utility of your IDE (115200 baud). Press `RESET (SW1)` shortly to  make sure the firmware boots. 


#### Device Preparation step 2 and further 
Please follow the [generic firmware instructions for these steps](https://github.com/energietransitie/twomes-generic-esp-firmware#device-preparation-step-2-establishing-a-device-name-and-device-activation_token). 

### Deploying on a [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk)
To deploy this software on [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk), see the [deploying section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/deploying/prerequisites/). The firmware needed can be found as a [release from this repository](https://github.com/energietransitie/twomes-p1-reader-firmware). 

## Developing
To develop software, or based on this software, see the [developing section in the twomes-generic-esp-firmware library documentation](https://www.energietransitiewindesheim.nl/twomes-generic-esp-firmware/starting/prerequisites/). Remember to press buttons to upload the firmware on the [twomes-p1-gateway](https://github.com/energietransitie/twomes-p1-gateway-hardware): 
* When you see the beginning of the sequence `Connecting ....___....`, press and hold the button labeled `GPIO0 (SW2)` on the PCB, then briefly press the button labeled `RESET (SW1)`, then release the button labeled `GPIO0 (SW2)`;
* You should see an indication that the firmware is being written to the device.


## Supported devices
This example was tested on:
- [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware)

## Features
List of features ready and TODOs for future development (other than the [features of the generic Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#features)). 

Ready:
* Automatically identifies the appropriate configurations for all versions: DSMR2-DSMR5.
* Retrieves information from all current smart meter variations: DSMR2-DSMR5.
* Utilizes timestamps provided by the P1 port whenever available for measurement timestamps.
* Discards inaccurate measurements (e.g., sporadic errors exhibited by Sagemcom XS210, meter type E0047).

To-do:
* Support Wi-Fi provisioning reset by holding down the K button (labeled as GPIO12 (SW3) on the PCB) for more than 10 seconds.
* Provide visual feedback on status and errors through LEDs.
* Support running on an [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) in combination with a (future) P1-BASE extension.


## Status
Project is: _in progress_

## License
This software is available under the [Apache 2.0 license](./LICENSE), Copyright 2023 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

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

Product owner:
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

We use and gratefully acknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache License 2.0](https://github.com/espressif/esp-idf/blob/9d34a1cd42f6f63b3c699c3fe8ec7216dd56f36a/LICENSE)
* [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware), by [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie), licensed under [Apache License 2.0](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/LICENSE.md)
