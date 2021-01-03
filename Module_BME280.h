#ifndef MODULE_BME280_SENSOR
#define MODULE_BME280_SENSOR

#include <Arduino_JSON.h>
#include <BME280I2C.h>
#include <Wire.h>
#include "configs.h"

void setup_bcm280_module();
JSONVar formatBmeSensorData(const char *uid, int rssi);
void printBME280Data(void (*callb) (float, float, float));

#endif // MODULE_BME280_SENSOR
