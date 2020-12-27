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
       Serial.print("Found BME280 with humidity sensor.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.print("Found BMP280 sensor without humidity.");
       break;
     default:
       Serial.println("Error! Found UNKNOWN sensor model!");
  }
}

JSONVar formatBmeSensorData(const char *uid, int rssi) {
  float temp(NAN), humi(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, humi, tempUnit, presUnit);

  // Clip some decimals
  double temperature = 0.01  * (int)(temp *  100);
  double humidity    = 0.01  * (int)(humi *  100);
  double pressure    = 0.001 * (int)(pres * 1000);

  JSONVar output;
  output["id"]              = uid;
  output["name"]            = BME_SENSOR_NAME;
  output["temperature"]     = temperature;
  output["humidity"]        = humidity;
  output["pressure"]        = pressure;
  output["signal_strength"] = rssi;

  return output;
}
