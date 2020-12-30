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

The configuration is done in `configs.h`.  
Also, don't forget to set your Wifi and MQTT credentials in `secrets.h`.

*Both files are marked so that they will not show up as changed in git so that you don't accidentally check in your credentials.  
To check in your changes, anyway, use `git update-index --no-assume-unchanged <file>`, stage your changes and before committing, use `git update-index --assume-unchanged <file>` again.*

### MQTT

The configured `MQTT_TOPIC` will be used as prefix and will be prefixed to the device id (MAC address) that is the first part of the [custom advertising format](https://github.com/atc1441/ATC_MiThermometer#advertising-format-of-the-custom-firmware).  
Say the configured `MQTT_TOPIC` is `bluetooth/sensors` and your device id is `00:11:22:33:44:55` then the resulting topic for this device will be `bluetooth/sensors/001122334455`.

The payload will look something like this:

```
{
  "id": "001122334455",
  "name": "ATC_334455",
  "temperature": 20.3,
  "humidity": 55,
  "battery": 92,
  "voltage": 3030,
  "signal_strength": -76
}
```

Please note that the device *name* seems to be only randomly picked up and will be missing from the JSON object most of the time.
However, you can manually define a name (see [Friendly Device Names](#friendly-device-names)).

The `signal_strength` is the Bluetooth RSSI.

The online state will be deposited with the MQTT broker with topic `MQTT_TOPIC_STATE`.
This is done by storing a *retained* message `online` that each client will receive on subscribe.
The message `offline` is stored as a *will* to be sent after the device disconnected from MQTT broker.

### Internal Sensor BME280

So if we are already putting a device somewhere in our apartment to pick up temperature measurements from dispersed Mi Thermometers, why not make it also measure the temperature, too, right?!  
To allow this we can add an inexpensive *BME280* module to the mix.

The following steps are necessary to activate the BME280 sensor:

- Connect the sensor to your ESP32:
  - VIN -> 3.3V / 3V3 
  - GND -> GND
  - SDA -> SDA / I2C_SDA (Serial Data)
  - SCK / SCL -> SCL / I2C_SCL (Serial Clock)
- Uncomment `#define USE_BME280_SENSOR"` in `configs.h` to compile the necessary code for BME280 sensor.

The payload will look something like this:

```
{
  "id": "001122334455",
  "name": "MiThermomether2MQTT",
  "temperature": 23.67,
  "humidity": 40.02,
  "pressure": 979.435,
  "signal_strength": -60
}
```

ID (MAC address) and signal strength (RSSI) are taken from the builtin ESP32 Wifi connection.  
The name is configured by `BME_SENSOR_NAME` in `configs.h`.

If sensor code is activated but no BME280 sensor is found, a warning will be printed to serial terminal.

Please note that placing both ESP32 and BME280 in the same enclosure can significantly increase the measured temperature.
At a room temperature of 20°C you can get values above 21 or 22°C even though the sensor is fixed against a hole in the casing.

### Filter Devices (Allow List)

It is possible to define a list of known devices so that unknown devices are ignored (*allow list*).

There are several reasons you might want this, e.g.: 

- There might be multiple devices with this firmware on your network because...
  - one will not pick up data of all your dispersed thermometers.
  - you want some kind of logical segmentation (i.e. different Wifi networks/MQTT servers/topics/...). 
- You might want to avoid picking up your neighbors thermometer data, too.

If that is the case, uncomment `#define ONLY_FORWARD_KNOWN_DEVICES` in `configs.h` and read the next section about *friendly names*.

### Friendly Device Names

You can define a list of known devices in `configs.h` along with custom names.

To do this, uncomment `#define USE_MAC_NAME_MAPPINGS`.
Then uncomment the block defining `MAC_NAME_MAPPING` by removing `/*` and `*/` and edit it to your liking.

### Home Assistant Integration

You can enable *Home Assistant* integration by uncommenting `#define USE_HOME_ASSISTANT` in `configs.h`.
This will automatically provide Home Assistant with configuration data for supported sensors via MQTT. 

Please note that this only works for configured devices from *friendly device names* mapping (see [above](#friendly-device-names)).  

### Builtin LED

I found the LED to be connected to a variety of different pins on different ESP32 boards I own.
To make the LED work on your specific board set `#define LED_BUILTIN` to the correct value.
It might be one of the following:

- ESP-WROOM-32 development board: `2`
- WEMOS LoLin32 development board: `5`
- ESP32 NodeMCU development board: `22`

## Dependencies

The code can be compiled with [Arduino for ESP32](https://github.com/espressif/arduino-esp32). Tested with version 1.0.4.

Furthermore, you will need the following libraries installed in your Arduino IDE:

- [ArduinoMqttClient](https://github.com/arduino-libraries/ArduinoMqttClient) (tested with version 0.1.5 BETA)
- [JSON Library for Arduino](https://github.com/arduino-libraries/Arduino_JSON) (tested with version 0.1.0 BETA)
- *(optional)* [BME280](https://github.com/finitespace/BME280) (tested with version 2.3.0)

BLE is supposed to be integrated into Arduino directly now, however I could not swiftly find any documentation or source code.
However, you can refer to the [old snapshot](https://github.com/nkolban/ESP32_BLE_Arduino/tree/master/src) for reference.

## Caveats

I implemented the initial release version in roughly two days but I kept on optimizing and improving.
Still, there is always potential for more improvements and additional features.

**Please note** that you will probably have change the partition scheme of your ESP32 to have enough program space.

To get this app compiled for the ESP32, in your Arduino IDE go to the *Tools* menu and select a Partition Scheme with at least ca. 1.5 MB for APP, e.g.:

- "No OTA (2MB APP/2MB SPIFFS)"
- "No OTA (2MB APP/2MB FATFS)"
- "HUGE APP (3MB No OTA/1MB SPIFFS)"
- "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS"

## Props

It was only possible to get the initial release up and running within under two day because there were excellent examples and the libraries in the first place.  
Kudos to everyone who was involved in getting this out to us!

- The Bluetooth part is based on the example `BLE_scan` from [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32), ported to Arduino ESP32 by Evandro Copercini.  
It is in turn based on Neil Kolban [example](https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp) for IDF. (I found the link to be dead.)
- The MQTT part is based on the example `WiFiSimpleSender` from Arduino library [ArduinoMqttClient](https://github.com/arduino-libraries/ArduinoMqttClient) (ver 0.1.5 Beta).
