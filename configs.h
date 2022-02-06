#ifndef MI_THERMOMETER_2_MQTT_CONFIG
#define MI_THERMOMETER_2_MQTT_CONFIG

const char APP_VERSION[] = "MiThermometer2MQTT 0.4";
#define WIFI_HOSTNAME "MiThermometer2MQTT"


/************************ HARDWARE ***********************/

// Builtin LED pin (if not defined by your selected board config)
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
// Duration in seconds taken for delay between scan sessions
#define SCAN_SESSION_INTERVAL 30


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


/******************** SSD1306 DISPLAY ********************/

// Uncomment to enable SSD1306 display
// #define USE_SSD1306_DISPLAY

// Display --> ESP-WROOM-32     --> Purpose
// -------     ------------         -------
//     GND --> GND              --> Ground
//     VCC --> 3V3              --> VIN (3.3V)
//     D0  --> GPIO 18          --> SCK (SPI Clock)
//     D1  --> GPIO 23          --> SPI MOSI (Master Out Slave In [data output from master])
//     RES --> GPIO 19 (random) --> Reset (pick any free digital pin)
//     DC  --> GPIO  4 (random) --> Data/Command (pick any free digital pin)
//     CS  --> GPIO  5          --> SPI SS (Slave Select) [CS=Chip Select?]

// Declaration for SSD1306 display
#define SSD1306_OLED_DC     4
#define SSD1306_OLED_CS     5
#define SSD1306_OLED_RESET 19

// OLED display width in pixels
#define SSD1306_SCREEN_WIDTH 128
// OLED display height in pixels
#define SSD1306_SCREEN_HEIGHT 64

// NTP time server to use
#define NTP_SERVER "pool.ntp.org"
// Offsets from UTC  are in seconds (3600 seconds = 1 hour)
// Offset between Universal Time and your time zone
#define UTC_OFFSET 3600
// Offset bewteen normal time and summer time
#define DAYLIGHT_SAVING_OFFSET 3600

// Minimal refresh time of the display.
// Note that things that are being calculated in between, e.g. bluetooth scan, might delay this. 
#define DISPLAY_UPDATE_INTERVAL 1000


/*************** HOME ASSISTANT INTEGRATION **************/

// Uncomment to enable Home Assistant integration
// #define USE_HOME_ASSISTANT
// Uncomment to print Home Assistant configs to serial terminal
// #define PRINT_HA_CONFIGS


/********************* CUSTOM NAMES **********************/

// Uncomment to map names to the devices.
// #define USE_MAC_NAME_MAPPINGS

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

// Uncomment to forward only known devices defined above.
// This is e.g. helpful if you use multiple bridges with overlapping ranges.
// #define ONLY_FORWARD_KNOWN_DEVICES

#endif // MI_THERMOMETER_2_MQTT_CONFIG
