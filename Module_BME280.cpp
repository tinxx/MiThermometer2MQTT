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

std::string formatBmeSensorData(char *jsonBuffer) {
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  sprintf(jsonBuffer,
        JSON_TEMPLATE_BME,
        BME_MQTT_UID,
        BME_MQTT_NAME,
        temp,
        hum,
        pres);

  return std::string(BME_MQTT_UID);
}
