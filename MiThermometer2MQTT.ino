/*
   MiThermometer2MQTT
   ==================
   
   Picks up the sensor data of Mi Thermometers with custom firmware and sends it to a MQTT broker.

   See README.md for more information.
*/

// Builtin LED pin
// #define LED_BUILTIN 2 // ESP-WROOM-32 DevBoard
#define LED_BUILTIN 5 // WEMOS LoLin32 DevBoard
// #define LED_BUILTIN 22 // ESP32 NodeMCU DevBoard

// Baud rate for serial terminal
#define SERIAL_BAUD 115200

// Flip endianness of values when interpreting data received from sensor (true for ESP32 <-> Mi Thermometer)
#define FLIP_ENDIAN true
// MAC address prefix of sensors
#define VENDOR_PREFIX "a4:c1:38:"
// Service data UID of the sensor data sent by Mi Thermometer (custom firmware)
#define SERVICE_DATA_UID "0000181a-0000-1000-8000-00805f9b34fb"
// Duration in seconds used for scanning sessions
#define SCAN_INTERVAL 5
// Duration in miliseconds taken for delay between scan sessions
#define SCAN_PAUSE 30000

// Wifi password and MQTT credentials are defined there. Changes are ignored by git.
#include "secrets.h"

#include "MyUtils.h"

// Home Assistant integration
// #include "Module_HomeAssistant.h"

// BME280 sensor
// #include "Module_BME280.h" // Uncomment to enable sensor
#ifdef USE_BME280_SENSOR
#include <Wire.h>
#endif // USE_BME280_SENSOR

// Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Wifi
#include <WiFi.h>

// MQTT
#include <ArduinoMqttClient.h>

// JSON
#include <Arduino_JSON.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char mqtt_broker[] = MQTT_SERVER;
int        mqtt_port     = MQTT_PORT;
const char mqtt_topic[]  = MQTT_TOPIC;

unsigned long previousMillis = 0;
int count = 0;


BLEScan* pBLEScan;


JSONVar formatSensorData(BLEAdvertisedDevice advertisedDevice, std::string mac_addr) {
  std::string sensorData = advertisedDevice.getServiceData();
  const char *sensorData_ptr = sensorData.c_str();
  std::string id = hexStr((unsigned char*)(sensorData_ptr), 6);
  int16_t temp = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 6)) : *(int16_t *)(sensorData_ptr + 6);
  double temperature = 1.0 * temp / 10;
  byte *humidity = (byte *) (sensorData_ptr + 8);
  byte *battery_level = (byte *) (sensorData_ptr + 9);
  int16_t voltage = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 10)) : *(int16_t *)(sensorData_ptr + 10);

  JSONVar output;
  output["id"] = id.c_str();

#ifdef USE_MAC_NAME_MAPPINGS
  std::map<std::string, std::string>::const_iterator name_it = MAC_NAME_MAPPING.find(mac_addr);
  if (name_it != MAC_NAME_MAPPING.end()) {
    output["name"] = name_it->second.c_str();
  }
#endif // USE_MAC_NAME_MAPPINGS
  if (!output.hasOwnProperty("name") && advertisedDevice.haveName()) {
    std::string name = advertisedDevice.getName();
    output["name"] = name.c_str();
  }

  output["temperature"] = temperature;
  output["humidity"] = *humidity;
  output["battery"] = *battery_level;
  output["voltage"] = voltage;
  if (advertisedDevice.haveRSSI()) {
    // RSSI (Received Signal Strength Indicator) is a negative dBm value.
    // Lower values mean lesser signal strengths (e.g. -20 is good, -120 is bad). 
    output["signal_strength"] = advertisedDevice.getRSSI();
  }
  output["humidity"] = *humidity;

  return output;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    std::string mac = advertisedDevice.getAddress().toString();
    if (mac.rfind(VENDOR_PREFIX, 0) != 0) {
      return;
    }

#ifdef ONLY_FORWARD_KNOWN_DEVICES
    if (MAC_NAME_MAPPING.find(mac.c_str()) == MAC_NAME_MAPPING.end()) {
      return;
    }
#endif // ONLY_FORWARD_KNOWN_DEVICES

    BLEUUID uid = advertisedDevice.getServiceDataUUID();
    if (!advertisedDevice.haveServiceData() || uid.toString().rfind(SERVICE_DATA_UID, 0) != 0) {
      return;
    }

    JSONVar sensorData = formatSensorData(advertisedDevice, mac);
    String sensorDataString = JSON.stringify(sensorData);
    const char *payload = sensorDataString.c_str();
    String id = JSON.stringify(sensorData["id"]);
    char topic[strlen(mqtt_topic) + 1 + id.length() + 1];
    sprintf(topic, "%s/%.12s", mqtt_topic, id.c_str() + 1);
    mqttClient.beginMessage(topic,
                            sensorDataString.length(), // size
                            false,                     // retain
                            0,                         // qos
                            false);                    // dup
    mqttClient.print(payload);
    mqttClient.endMessage();
    Serial.println(payload);
  }
};


void publishState(bool online = true) {
  char topic[] = MQTT_TOPIC_STATE;
  mqttClient.beginMessage(topic, 
                          true, // retain
                          1);   // qos
  mqttClient.print(online ? "online" : "offline");
  mqttClient.endMessage();
}

void publishWill() {
  char topic[] = MQTT_TOPIC_STATE;
  mqttClient.beginWill(topic, 
                       true, // retain
                       1);   // qos
  mqttClient.print("offline");
  mqttClient.endWill();
}


std::string BOARD_UID;

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();

  String mac = WiFi.macAddress();
  BOARD_UID = stripColonsFromMac(mac.c_str());

  // BME280 setup
#ifdef USE_BME280_SENSOR
  Wire.begin();
  digitalWrite(LED_BUILTIN, 0);
  setup_bcm280_module();
  digitalWrite(LED_BUILTIN, 1);
#endif // USE_BME280_SENSOR

  // Wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, 0);
    delay(500);
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("Connected to Wifi network %s (%s).\n", WIFI_SSID, WiFi.localIP().toString().c_str());

  // MQTT setup
  mqttClient.setId(MQTT_CLIENTID);
  mqttClient.setUsernamePassword(MQTT_USER, MQTT_PASS);

  Serial.printf("Attempting to connect to the MQTT broker at %s:%i.\n", mqtt_broker, mqtt_port);
  if (!mqttClient.connect(mqtt_broker, mqtt_port)) {
    Serial.printf("MQTT connection failed! Error code = %i\n", mqttClient.connectError());
    while (1);
  }
  Serial.println("Connected to the MQTT broker.");

  // Bluetooth setup
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // Online state will be deposited with MQTT broker
  Serial.println("Publishing retained online state and will...");
  publishState();
  publishWill();

  // Publish Home Assistant configurations via MQTT
#ifdef USE_HOME_ASSISTANT
  Serial.println("Publishing Home Assistant configs for configured sensors...");
  publishHomeAssistantConfigs(mqttClient, BOARD_UID.c_str());
#endif // USE_HOME_ASSISTANT

  Serial.println("Entering main loop...\n");
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SCAN_PAUSE) {
    previousMillis = currentMillis;

#ifdef USE_BME280_SENSOR
  // Process attached BME280 sensor data
  JSONVar sensorData = formatBmeSensorData(BOARD_UID.c_str(), WiFi.RSSI());
  String sensorDataString = JSON.stringify(sensorData);
  const char *payload = sensorDataString.c_str();
  char topic[strlen(mqtt_topic) + 1 + strlen(BOARD_UID.c_str()) + 1];
  sprintf(topic, "%s/%.12s", mqtt_topic, BOARD_UID.c_str());
  mqttClient.beginMessage(topic,
                          sensorDataString.length(), // size
                          false,                     // retain
                          0,                         // qos
                          false);                    // dup
  mqttClient.print(payload);
  mqttClient.endMessage();
  Serial.println(payload);
#endif // USE_BME280_SENSOR

    // Bluetooth scan
    BLEScanResults foundDevices = pBLEScan->start(SCAN_INTERVAL, false);
    pBLEScan->clearResults();   // delete results from BLEScan buffer to release memory
  }
}
