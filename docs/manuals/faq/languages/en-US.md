# Smart meter module - FAQ

Frequently asked information about the smart meter module.

# 1. What data does this device collect?

Every 10 minutes, the smart meter module registers via the P1 port of your smart electricity meter:

- meter readings of your gas meter;
- meter readings of your electricity meter;
- DSMR version and type of your electricity meter.

## 2. How can I install the device?

For installation instructions, please refer to the [installation manual](../../../installation/).

## 3. How can I tell if the device is working properly?

Whenever the smart meter is successfully read, you'll see a light flashing two quick greens. If the reading fails, the light will flash two quick reds. The smart meter is read at fixed intervals: every full hour and 10, 20, 30, 40, and 50 minutes past the hour. You can wait for it if you want to be sure. The 'Last seen at' time under 'Smart meter module' in the 'Sources' screen of the NeedForHeat app is updated shortly afterward (use pull-down to refresh).

## 4. How can I re-connect the device to Wi-Fi?

A Wi-Fi reset of the device may be necessary if you change your home Wi-Fi network, if you want to connect the device to a different home Wi-Fi network, or for other reasons.

![device](../assets/p1-gateway-wi-fi-reset.jpg)

The Wi-Fi reset works as follows:

1. Press and hold the K-button on the smart meter module (you will feel a distinct click).
2. After holding it down for about 10 seconds, after you see a green light blinking 5 times, release the button (if the surroundings are bright, you may not see it).
3. In the NeedForhat app, scan the QR code of your smart meter module again and follow the instructions.

## 5. Where can I find more technical information about this device?
The source code of this measurement device is publicly available on GitHub under [twomes-p1-reader-firmware](https://github.com/energietransitie/twomes-p1-reader-firmware).

## What if I have other quesions or remarks?
Please send an email to the NeedForHeat research helpdesk via [needforheatonderzoek@windesheim.nl](needforheatonderzoek@windesheim.nl).
