#ifndef USE_BME280_SENSOR
#define USE_BME280_SENSOR

#include <BME280I2C.h>

// Defined UID and name are used to identify the sensor in the JSON payload sent via MQTT.
// UID is also used as the MQTT topic suffix (e.g. "bluetooth/sensors/bme280").
#define BME_MQTT_UID "bme280"
#define BME_MQTT_NAME "BME280 Sensor"

#define JSON_EXAMPLE_BME "{\"id\": \"BME280\", \"name\": \"_SHORTENED_DEVICE_NAME__\", \"temperature\": 00.00, \"humidity\": 00.00, \"pressure\": 0000.00}"
#define JSON_TEMPLATE_BME "{\"id\": \"%.12s\", \"name\": \"%.24s\", \"temperature\": %.2f, \"humidity\": %.2f, \"pressure\": %.2f}"
const short JSON_BUFFER_SIZE_BME = sizeof(JSON_EXAMPLE_BME);

void setup_bcm280_module();
std::string formatBmeSensorData(char *jsonBuffer);

#endif // USE_BME280_SENSOR
