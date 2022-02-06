#include "Module_BME280.h"

BME280I2C bme;

void setup_bcm280_module() {
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  switch(bme.chipModel())
  {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 with humidity sensor.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor without humidity.");
      break;
    default:
      Serial.println("Error! Found UNKNOWN sensor model!");
  }
}

void formatBmeSensorData(JsonDocument& target, const char *uid, int rssi) {
  float temp(NAN), humi(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, humi, tempUnit, presUnit);

  // Clip some decimals
  double temperature = 0.01  * (int)(temp *  100);
  double humidity    = 0.01  * (int)(humi *  100);
  double pressure    = 0.001 * (int)(pres * 1000);

  target["id"]              = uid;
  target["name"]            = BME_SENSOR_NAME;
  target["temperature"]     = temperature;
  target["humidity"]        = humidity;
  target["pressure"]        = pressure;
  target["signal_strength"] = rssi;
}

void printBME280Data(void (*callb) (float, float, float)) {
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  callb(temp, hum, pres);
}
