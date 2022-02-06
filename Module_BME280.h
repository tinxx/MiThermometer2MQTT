#ifndef MODULE_BME280_SENSOR
#define MODULE_BME280_SENSOR

#include <ArduinoJson.h>
#include <BME280I2C.h>
#include <Wire.h>
#include "configs.h"

void setup_bcm280_module();
void formatBmeSensorData(JsonDocument& target, const char *uid, int rssi);
void printBME280Data(void (*callb) (float, float, float));

#endif // MODULE_BME280_SENSOR
