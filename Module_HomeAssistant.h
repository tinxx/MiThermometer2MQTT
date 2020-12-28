#ifndef MODULE_HOME_ASSISTANT
#define MODULE_HOME_ASSISTANT

#include "Arduino.h"
#include <ArduinoMqttClient.h>
#include <Arduino_JSON.h>
#include "configs.h"
#include "MyUtils.h"

#ifdef USE_BME280_SENSOR
#include "Module_BME280.h"
#endif // USE_BME280_SENSOR

#define IDENTIFIER_PREFIX_HA "bluetooth_"

#define TOPIC_EXAMPLE_HA_CONFIG "homeassistant/sensor/001122334455/signal_strength/config"
#define TOPIC_TEMPLATE_HA_CONFIG "homeassistant/sensor/%.12s/%.15s/config"
const short TOPIC_TEMPLATE_HA_CONFIG_SIZE = strlen(TOPIC_EXAMPLE_HA_CONFIG) + 1;

#define VALUE_TEMPLATE_EXAMPLE_HA_CONFIG "{{ value_json.signal_strength }}"
#define VALUE_TEMPLATE_TEMPLATE_HA_CONFIG "{{ value_json.%.15s }}"
const short VALUE_TEMPLATE_TEMPLATE_HA_CONFIG_SIZE = strlen(VALUE_TEMPLATE_EXAMPLE_HA_CONFIG) + 1;

const char MI_THERMOMETER_MANUFACTURER[] = "Xiaomi";
const char MI_THERMOMETER_MODEL[] = "Mi Thermometer (LYWSD03MMC)";
const std::map<std::string, std::string> MI_THERMOMETER_CLASS_UNIT_MAPPING = {
    { "temperature",     "°C"  },
    { "humidity",        "%"   },
    { "battery",         "%"   },
    { "signal_strength", "dBm" }
};

#ifdef MODULE_BME280_SENSOR
#define BME_SENSOR_MODEL "Humidity Sensor BME280"
#define BME_SENSOR_MANUFACTURER "Bosch Sensortec GmbH"
const std::map<std::string, std::string> BME_CLASS_UNIT_MAPPING = {
    { "temperature",     "°C"  },
    { "humidity",        "%"   },
    { "pressure",        "hPa" },
    { "signal_strength", "dBm" },
};
#endif // MODULE_BME280_SENSOR

void publishHomeAssistantConfigs(MqttClient mqttClient, const char *board_uid);

#endif // MODULE_HOME_ASSISTANT
