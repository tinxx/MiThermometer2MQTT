/*
   Bluetooth Low Energy Scanner for Mi Thermometer with custom firmware for ESP32.

   Scans the ether for sensor data advertised by Mi Thermometer with custom firmware (see link below).
   The sensor data is written to the serial console in JSON format and should eventually be sent
   advertised via MQTT, too, to make it directly available to, e.g.,  Home Assistant.

   Custom firmware is found at: https://github.com/atc1441/ATC_MiThermometer

   BLE is supposed to be integrated into Arduino directly now, however I could not find any docuetation or source code.
   You can refer to the old snapshot for reference: https://github.com/nkolban/ESP32_BLE_Arduino/tree/master/src

   Based on the example "BLE_scan" from Arduino core for the ESP32: https://github.com/espressif/arduino-esp32

   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Flip endianness of values when interpreting data received from sensor (true for ESP32 <-> Mi Thermometer)
#define FLIP_ENDIAN true
// MAC address prefix of sensors
#define VENDOR_PREFIX "a4:c1:38:"
// Service data UID of the sensor data sent by Mi Thermometer (custom firmware)
#define SERVICE_DATA_UID "0000181a-0000-1000-8000-00805f9b34fb"
// Duration in seconds used for scanning sessions
#define SCAN_INTERVAL 5
// Duration in miliseconds taken for delay between scan sessions
#define SCAN_PAUSE 20000


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
#define JSON_EXAMPLE "{id: \"001122334455\", temperature: 00.00, humidity: 00, battery_lvl: 00, battery_v: 0.000, rssi: -32768, name: \"_SHORTENED_DEVICE_NAME__\"}"
#define JSON_TEMPLATE "{id: \"%.12s\", temperature: %.1f, humidity: %i, battery_lvl: %i, battery_v: %.3f, rssi: %i%s}"
const size_t JSON_BUFFER_SIZE = sizeof(JSON_EXAMPLE);

const char* formatSensorData(char * jsonBuffer, BLEAdvertisedDevice advertisedDevice) {
  std::string sensorData = advertisedDevice.getServiceData();
  const char *sensorData_ptr = sensorData.c_str();
  std::string id = hexStr((unsigned char*)(sensorData_ptr), 6);
  int16_t temp = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 6)) : *(int16_t *)(sensorData_ptr + 6);
  double temperature = 1.0 * temp / 10;
  byte *humidity = (byte *) (sensorData_ptr + 8);
  byte *battery_level = (byte *) (sensorData_ptr + 9);
  int16_t volt = FLIP_ENDIAN ? __builtin_bswap16(*(int16_t *)(sensorData_ptr + 10)) : *(int16_t *)(sensorData_ptr + 10);
  double voltage = 1.0 * volt / 1000;

  // RSSI (Received Signal Strength Indicator) is a negative dBm value.
  // Lower values mean lesser signal strengths (e.g. -20 is good, -120 is bad). 
  short rssi = advertisedDevice.haveRSSI() && advertisedDevice.getRSSI() >= -32768
                ? advertisedDevice.getRSSI()
                : 404;

  char nameJson[35];
  if (advertisedDevice.haveName()) {
    std::string name = advertisedDevice.getName();
    sprintf(nameJson, ", name: \"%.24s\"", name.c_str());
  } else {
    nameJson[0] = '\0';
  }
  
  sprintf(jsonBuffer,
          JSON_TEMPLATE,
          id.c_str(),
          temperature,
          *humidity,
          *battery_level,
          voltage,
          rssi,
          nameJson);
  return jsonBuffer;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      std::string mac = advertisedDevice.getAddress().toString();
      if (mac.rfind(VENDOR_PREFIX, 0) != 0) {
        return;
      }

      BLEUUID uid = advertisedDevice.getServiceDataUUID();
      if (!advertisedDevice.haveServiceData() || uid.toString().rfind(SERVICE_DATA_UID, 0) != 0) {
        return;
      }

      char jsonBuffer[JSON_BUFFER_SIZE];
      formatSensorData(jsonBuffer, advertisedDevice);
      Serial.println(jsonBuffer);

      // TODO: Send data via MQTT
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_INTERVAL, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(SCAN_PAUSE);
}
