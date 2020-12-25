#ifndef SECRETS
#define SECRETS

#define WIFI_SSID "myWifiSsid"
#define WIFI_PSK "myWifiPassword"

#define MQTT_SERVER "192.168.1.10"
#define MQTT_PORT 1883
#define MQTT_TOPIC "bluetooth/sensors"
#define MQTT_TOPIC_STATUS "bluetooth/sensors/state"
#define MQTT_CLIENTID "bluetooth_bridge"
#define MQTT_USER "myMqttUser"
#define MQTT_PASS "myMqttPassword"

// Uncomment the whole block to map names to the devices.
/*
#define USE_MAC_NAME_MAPPINGS
// Uncomment to forward only known devices. This is helpful if you use multiple bridges. 
// #define ONLY_FORWARD_KNOWN_DEVICES
// Key is the device MAC address, value is a friendly name (24 characters max).
#include <map>
const std::map<std::string, std::string> MAC_NAME_MAPPING = {
    { "a4:c1:38:00:00:00", "Thermometer Livingroom" },
    { "a4:c1:38:11:11:11", "Thermometer Bedroom" },
    { "a4:c1:38:22:22:22", "Thermometer Attic" }
};
*/

#endif // SECRETS
