#ifndef MI_THERMOMETER_2_MQTT_CONFIG
#define MI_THERMOMETER_2_MQTT_CONFIG

const char APP_VERSION[] = "MiThermometer2MQTT 0.2";


/************************ HARDWARE ***********************/

// Builtin LED pin
#ifndef LED_BUILTIN
// #define LED_BUILTIN 2 // ESP-WROOM-32 DevBoard
#define LED_BUILTIN 5 // WEMOS LoLin32 DevBoard
// #define LED_BUILTIN 22 // ESP32 NodeMCU DevBoard
#endif // LED_BUILTIN

// Baud rate for serial terminal
#define SERIAL_BAUD 115200

// Flip endianness of values when interpreting data received from sensor (true for ESP32 <-> Mi Thermometer)
#define FLIP_ENDIAN true


/********************* MI THERMOMETER ********************/

// MAC address prefix of sensors
#define VENDOR_PREFIX "a4:c1:38:"
// Service data UID of the sensor data sent by Mi Thermometer (custom firmware)
#define SERVICE_DATA_UID "0000181a-0000-1000-8000-00805f9b34fb"


/************************ BLUETOOTH **********************/

// Duration in seconds used for scanning sessions
#define SCAN_INTERVAL 5
// Duration in miliseconds taken for delay between scan sessions
#define SCAN_PAUSE 30000


/************************** MQTT *************************/

#define MQTT_SERVER "192.168.1.10"
#define MQTT_PORT 1883
// Don't forget to fill in your user name and password in `secrets.h`.

#define MQTT_CLIENTID "bluetooth_bridge"
#define MQTT_TOPIC "bluetooth/sensors"
#define MQTT_TOPIC_STATE "bluetooth/bridge/state"


/********************* BME280 SENSOR *********************/

// Uncomment to enable BME280 sensor
// #define USE_BME280_SENSOR

// Defined name is used to identify the sensor in the JSON payload sent via MQTT.
#define BME_SENSOR_NAME "Thermometer Bridge"


/*************** HOME ASSISTANT INTEGRATION **************/

// Uncomment to enable Home Assistant integration
// #define USE_HOME_ASSISTANT
// Uncomment to print Home Assistant configs to serial terminal
// #define PRINT_HA_CONFIGS


/********************* CUSTOM NAMES **********************/

// Uncomment to map names to the devices. Specify mapping in `secrets.h`.
// #define USE_MAC_NAME_MAPPINGS

// Uncomment to forward only known devices. (Only works together with name mappings.)
// This is e.g. helpful if you use multiple bridges with overlapping ranges.
// #define ONLY_FORWARD_KNOWN_DEVICES

// If you enable `USE_MAC_NAME_MAPPINGS` in `configs.h`,
// Uncomment the following block and fill in your mappings.
/*
#include <map>
// Key is the device MAC address (lower key), value is a friendly name.
const std::map<std::string, std::string> MAC_NAME_MAPPING = {
    { "a4:c1:38:00:00:00", "Thermometer Livingroom" },
    { "a4:c1:38:11:11:11", "Thermometer Bedroom" },
    { "a4:c1:38:22:22:22", "Thermometer Attic" }
};
*/

#endif // MI_THERMOMETER_2_MQTT_CONFIG
