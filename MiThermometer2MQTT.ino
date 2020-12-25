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

// BME280 sensor
// #include "BME280_Module.h" // Uncomment to enable sensor
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

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char mqtt_broker[] = MQTT_SERVER;
int        mqtt_port     = MQTT_PORT;
const char mqtt_topic[]  = MQTT_TOPIC;

unsigned long previousMillis = 0;
int count = 0;


// Thanks to Mat at StackExchange for hex string conversion.
// https://codereview.stackexchange.com/questions/78535/converting-array-of-bytes-to-the-hex-string-representation
constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(unsigned char *data, int len)
{
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

BLEScan* pBLEScan;

// Bluetooth device name can be between 0 and 248 octets but we will only pass the first 24 characters.
#define JSON_EXAMPLE "{\"id\": \"001122334455\", \"temperature\": 00.00, \"humidity\": 00, \"battery\": 00, \"voltage\": -32768, \"rssi\": -32768, \"linkquality\": 100, \"name\": \"_SHORTENED_DEVICE_NAME__\"}"
#define JSON_TEMPLATE "{\"id\": \"%.12s\", \"temperature\": %.1f, \"humidity\": %i, \"battery\": %i, \"voltage\": %i, \"rssi\": %i, \"linkquality\": %i%s}"
const size_t JSON_BUFFER_SIZE = sizeof(JSON_EXAMPLE);

std::string formatSensorData(char * jsonBuffer, BLEAdvertisedDevice advertisedDevice, std::string mac_addr) {
  std::string sensorData = advertisedDevice.getServiceData();
  const char *sensorData_ptr = sensorData.c_str();
  std::string id = hexStr((unsigned char*)(sensorData_ptr), 6);
  int16_t temp = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 6)) : *(int16_t *)(sensorData_ptr + 6);
  double temperature = 1.0 * temp / 10;
  byte *humidity = (byte *) (sensorData_ptr + 8);
  byte *battery_level = (byte *) (sensorData_ptr + 9);
  int16_t voltage = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 10)) : *(int16_t *)(sensorData_ptr + 10);

  // RSSI (Received Signal Strength Indicator) is a negative dBm value.
  // Lower values mean lesser signal strengths (e.g. -20 is good, -120 is bad). 
  short rssi = advertisedDevice.haveRSSI() && advertisedDevice.getRSSI() >= -32768
                ? advertisedDevice.getRSSI()
                : 404;
  size_t linkquality = 0;
  if (rssi < 0 && rssi > -120) linkquality = rssi >= -20 ? 100 : 100 + std::round(rssi + 20);

  char nameJson[35] = "";
#ifdef USE_MAC_NAME_MAPPINGS
  std::map<std::string, std::string>::const_iterator name_it = MAC_NAME_MAPPING.find(mac_addr);
  if (name_it != MAC_NAME_MAPPING.end()) {
    sprintf(nameJson, ", \"name\": \"%.24s\"", name_it->second.c_str());
  }
#endif // USE_MAC_NAME_MAPPINGS
  if (advertisedDevice.haveName() && strlen(nameJson) == 0) {
    std::string name = advertisedDevice.getName();
    sprintf(nameJson, ", \"name\": \"%.24s\"", name.c_str());
  }

  sprintf(jsonBuffer,
          JSON_TEMPLATE,
          id.c_str(),
          temperature,
          *humidity,
          *battery_level,
          voltage,
          rssi,
          linkquality,
          nameJson);

  return id;
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

    char jsonBuffer[JSON_BUFFER_SIZE];
    std::string id = formatSensorData(jsonBuffer, advertisedDevice, mac);
    char topic[sizeof(mqtt_topic) + 13];
    sprintf(topic, "%s/%s", mqtt_topic, id.c_str());
    mqttClient.beginMessage(topic);
    mqttClient.print(jsonBuffer);
    mqttClient.endMessage();
    Serial.println(jsonBuffer);
  }
};

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();

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
  char jsonBuffer[JSON_BUFFER_SIZE_BME];
  std::string id = formatBmeSensorData(jsonBuffer);
  char topic[sizeof(mqtt_topic) + sizeof(BME_MQTT_UID) + 1];
  sprintf(topic, "%s/%s", mqtt_topic, id.c_str());
  mqttClient.beginMessage(topic);
  mqttClient.print(jsonBuffer);
  mqttClient.endMessage();
  Serial.println(jsonBuffer);
#endif // USE_BME280_SENSOR

    // Bluetooth scan
    BLEScanResults foundDevices = pBLEScan->start(SCAN_INTERVAL, false);
    pBLEScan->clearResults();   // delete results from BLEScan buffer to release memory
  }
}
