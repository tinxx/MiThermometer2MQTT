# MiThermometer2MQTT

Picks up the sensor data of Mi Thermometers with [custom firmware](https://github.com/atc1441/ATC_MiThermometer) and sends it to a MQTT broker.

## Description

This is a Bluetooth Low Energy (BLE) scanner for Mi Thermometer with custom firmware programmed with Arduino for ESP32.

The device scans the ether for sensor data advertised by Mi Thermometer with [custom firmware](https://github.com/atc1441/ATC_MiThermometer).  
The sensor data is written to the serial console in JSON format and will also be sent via MQTT to a predefined topic.
This makes it (in)directly available to your home automation central, e.g., Home Assistant, OpenHAB or Node-RED.

The Mi Thermometer sensor data will be identified via sender MAC address and BLE Service Data UID.
The MAC address starts with `a4:c1:38:`.
The Service Data UID should match `0000181a-0000-1000-8000-00805f9b34fb`.
All other Bluetooth LE advertisements will be ignored.

## Configuration

### Builtin LED

I found the LED to be connected to a variety of different pins on different ESP32 boards I own.
To make the LED work on your specific board set `#define LED_BUILTIN` to the correct value.
It might be one of the following:

- ESP-WROOM-32 development board: `2`
- WEMOS LoLin32 development board: `5`
- ESP32 NodeMCU development board: `22`

### Wifi

To configure Wifi SSID/Password and MQTT server/login/topic just edit `secrets.h` before compilation.
(The file is marked so that it will not show up as changed in git so that you don't accidentally check in your credentials.)

### MQTT

The configured `MQTT_TOPIC` will be used as prefix and will be prefixed to the device id (MAC address) that is the first part of the [custom advertising format](https://github.com/atc1441/ATC_MiThermometer#advertising-format-of-the-custom-firmware).  
Say the configured `MQTT_TOPIC` is `bluetooth/sensors` and your device id is `00:11:22:33:44:55` then the resulting topic for this device will be `bluetooth/sensors/001122334455`.

The payload will look something like this:

```
{
  "id": "001122334455",
  "temperature": 20.3,
  "humidity": 55,
  "battery": 92,
  "voltage": 3030,
  "rssi": -76,
  "linkquality": 44,
  "name": "ATC_334455"
}
```

Please note that the device *name* seems to be only randomly picked up and will be missing from the JSON object most of the time.

The link quality is roughly calculated from the Bluetooth RSSI.
RSSI values between `0` and `-20` will be considered as `100` link quality.
RSSI values between `-20` and `-120` will be inversely proportionally mapped onto link quality values between `100` and `0`.

### Internal Sensor BME280

So if we are already putting a device somewhere in our apartment to pick up temperature measurements from dispersed Mi Thermometers, why not make it also measure the temperature, too, right?!  
To allow this we can add an inexpensive *BME280* module to the mix.

The following steps are necessary to activate the BME280 sensor:

- Connect the sensor to your ESP32:
  - VIN -> 3.3V / 3V3 
  - GND -> GND
  - SDA -> SDA (Serial Data)
  - SCK / SCL -> SCL (Serial Clock)
- Uncomment `#include "BME280_Module.h"` to compile the necessary code for BME280 sensor.

If sensor code is activated but no BME280 sensor is found, a warning will be printed to serial terminal.

## Dependencies

The code can be compiled with [Arduino for ESP32](https://github.com/espressif/arduino-esp32). Tested with version 1.0.4.

Furthermore, you will need the following libraries installed in your Arduino IDE:

- [Wifi](https://github.com/arduino-libraries/WiFi) (tested with version 1.2.7)
- [ArduinoMqttClient](https://github.com/arduino-libraries/ArduinoMqttClient) (tested with version 0.1.5 BETA)
- *(optional)* [BME280](https://github.com/finitespace/BME280) (tested with version 2.3.0)

BLE is supposed to be integrated into Arduino directly now, however I could not swiftly find any documentation or source code.
However, you can refer to the [old snapshot](https://github.com/nkolban/ESP32_BLE_Arduino/tree/master/src) for reference.

## Caveats

I implemented the project in roughly two days so there is likely much potential for optimizations and improvements.

**Please note** that you will probably have change the partition scheme of your ESP32 to have enough program space.

To get this app compiled for the ESP32, in your Arduino IDE go to the *Tools* menu and select a Partition Scheme with at least ca. 1.5 MB for APP, e.g.:

- "No OTA (2MB APP/2MB SPIFFS)"
- "No OTA (2MB APP/2MB FATFS)"
- "HUGE APP (3MB No OTA/1MB SPIFFS)"
- "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS"

## Props

It was only possible to get the app up and running within under two day because there were excellent examples.  
Kudos to everyone who was involved in getting this out to us!

- The Bluetooth part is based on the example `BLE_scan` from [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32), ported to Arduino ESP32 by Evandro Copercini.  
It is in turn based on Neil Kolban [example](https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp) for IDF. (I found the link to be dead.)
- The MQTT part is based on the example `WiFiSimpleSender` from Arduino library [ArduinoMqttClient](https://github.com/arduino-libraries/ArduinoMqttClient) (ver 0.1.5 Beta).
