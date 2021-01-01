/*
   MiThermometer2MQTT
   ==================
   
   Picks up the sensor data of Mi Thermometers with custom firmware and sends it to a MQTT broker.

   See README.md for more information.
*/

// Define your configurations and credentials in the following files. Changes are ignored by git.
#include "configs.h"
#include "secrets.h"

#include "MyUtils.h"

#if !defined(WIFI_SSID) || !defined(WIFI_PSK) || !defined(MQTT_USER) || !defined(MQTT_PASS)
#error "Don't forget to fill your credentials in 'secrets.h'!"
#endif // WIFI_SSID

#ifdef USE_BME280_SENSOR
#include "Module_BME280.h"
#endif // USE_BME280_SENSOR

#ifdef USE_HOME_ASSISTANT
#include "Module_HomeAssistant.h"
#endif // USE_HOME_ASSISTANT


// Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan* pBLEScan;

// Wifi
#include <WiFi.h>

// JSON
#include <Arduino_JSON.h>

// MQTT
#include <ArduinoMqttClient.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char mqtt_broker[] = MQTT_SERVER;
int        mqtt_port     = MQTT_PORT;
const char mqtt_topic[]  = MQTT_TOPIC;


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
bool online_state = true; // Consciously initialized `true`

unsigned long previousMillis = 0;
int count = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();

  String mac = WiFi.macAddress();
  BOARD_UID = stripColonsFromMac(mac.c_str());

  // BME280 setup
#ifdef MODULE_BME280_SENSOR
  Wire.begin();
  digitalWrite(LED_BUILTIN, 0);
  setup_bcm280_module();
  digitalWrite(LED_BUILTIN, 1);
#endif // MODULE_BME280_SENSOR

  WiFi.mode(WIFI_STA);

  // Bluetooth setup
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  Serial.println("Entering main loop...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (online_state) {
      Serial.print("No Wifi connection, (re)connecting");
      WiFi.begin(WIFI_SSID, WIFI_PSK);
      online_state = false;
    }
    digitalWrite(LED_BUILTIN, 0);
    delay(500);
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    Serial.print(".");
    return;
  }
  if (!online_state) {
    online_state = true;
    Serial.printf("\nConnected to Wifi network %s (%s).\n", WIFI_SSID, WiFi.localIP().toString().c_str());
  }

  if (!mqttClient.connected()) {
    Serial.printf("MQTT client not connected, (re)connecting to broker at %s:%i.\n", mqtt_broker, mqtt_port);
    // Create new MQTT client
    // FIXME: I was not able to reconnect existing client (ArduinoMqttClient version 0.1.5 BETA).
    mqttClient = MqttClient(wifiClient);
    mqttClient.setId(MQTT_CLIENTID);
    mqttClient.setUsernamePassword(MQTT_USER, MQTT_PASS);

    if (!mqttClient.connect(mqtt_broker, mqtt_port)) {
      Serial.printf("MQTT connection failed! Error code = %i\n", mqttClient.connectError());
      // Delay, so we won't bombard the MQTT server with connection attempts.
      delay(30000);
      return;
    }
    Serial.println("Connected to the MQTT broker.");

    // Online state will be deposited with MQTT broker
    Serial.println("Publishing retained online state and will...");
    publishState();
    publishWill();

      // Publish Home Assistant configurations via MQTT
#ifdef MODULE_HOME_ASSISTANT
    Serial.println("Publishing Home Assistant configs for configured sensors...");
    publishHomeAssistantConfigs(mqttClient, BOARD_UID.c_str());
#endif // MODULE_HOME_ASSISTANT
  }

  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SCAN_PAUSE) {
    previousMillis = currentMillis;

#ifdef MODULE_BME280_SENSOR
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
#endif // MODULE_BME280_SENSOR

    // Bluetooth scan
    BLEScanResults foundDevices = pBLEScan->start(SCAN_INTERVAL, false);
    pBLEScan->clearResults();   // delete results from BLEScan buffer to release memory
  }
}
