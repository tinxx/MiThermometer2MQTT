#ifndef USE_BME280_SENSOR
#define USE_BME280_SENSOR

#include <BME280I2C.h>
#include <Arduino_JSON.h>


// Defined name is used to identify the sensor in the JSON payload sent via MQTT.
#define BME_SENSOR_NAME "MiThermomether2MQTT"

void setup_bcm280_module();
JSONVar formatBmeSensorData(const char *uid, int rssi);

#endif // USE_BME280_SENSOR
