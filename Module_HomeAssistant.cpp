#include "Module_HomeAssistant.h"

#ifdef USE_HOME_ASSISTANT

#ifndef USE_MAC_NAME_MAPPINGS
#error "To use Home Assistant integration, you need to configure MAC name mappings. Only known devices will be auto configured with Home Assistant!"
#endif // USE_MAC_NAME_MAPPINGS

// https://arduinojson.org/v6/how-to/determine-the-capacity-of-the-jsondocument/
const int HA_JSON_CAPACITY = 768;

void formatConfigData(JsonDocument& target,
                      const char *uid,
                      const char *name,
                      const char *device_class,
                      const char *unit,
                      const char *manufacturer,
                      const char *model) {

  char state_topic[strlen(MQTT_TOPIC) + 1 + strlen(uid) + 1];
  sprintf(state_topic, "%s/%s", MQTT_TOPIC, uid);

  char name_with_class[strlen(name) + 1 + strlen(device_class) + 1];
  sprintf(name_with_class, "%s %s", name, device_class);

  char device_idetifier[strlen(IDENTIFIER_PREFIX_HA) + strlen(uid) + 1];
  sprintf(device_idetifier, "%s%.12s", IDENTIFIER_PREFIX_HA, uid);


  char unique_id[strlen(IDENTIFIER_PREFIX_HA) + strlen(uid) + 1 + strlen(device_class) + 1];
  sprintf(unique_id, "%s%.12s_%.15s", IDENTIFIER_PREFIX_HA, uid, device_class);

  char value_template[VALUE_TEMPLATE_TEMPLATE_HA_CONFIG_SIZE];
  sprintf(value_template, VALUE_TEMPLATE_TEMPLATE_HA_CONFIG, device_class);

  target["availability"][0]["topic"] = MQTT_TOPIC_STATE;
  target["state_topic"]              = state_topic;
  target["json_attributes_topic"]    = state_topic;
  target["name"]                     = name_with_class;
  target["device"]["identifiers"][0] = device_idetifier;
  target["device"]["manufacturer"]   = manufacturer;
  target["device"]["model"]          = model;
  target["device"]["name"]           = name;
  target["device"]["sw_version"]     = APP_VERSION;
  target["device_class"]             = device_class;
  target["unit_of_measurement"]      = unit;
  target["unique_id"]                = unique_id;
  target["value_template"]           = value_template;
}

void publishHomeAssistantConfigs(MqttClient mqttClient, const char *board_uid) {
  // Configure BME280
  #ifdef MODULE_BME280_SENSOR
  for (std::map<std::string, std::string>::const_iterator classAndUnit_it = BME_CLASS_UNIT_MAPPING.begin();
       classAndUnit_it != BME_CLASS_UNIT_MAPPING.end();
       classAndUnit_it++) {

    char topic[TOPIC_TEMPLATE_HA_CONFIG_SIZE];
    sprintf(topic,
            TOPIC_TEMPLATE_HA_CONFIG,
            board_uid,
            classAndUnit_it->first.c_str());

    StaticJsonDocument<HA_JSON_CAPACITY> configData;
    formatConfigData(configData,
                      board_uid,
                      BME_SENSOR_NAME,
                      classAndUnit_it->first.c_str(),
                      classAndUnit_it->second.c_str(),
                      BME_SENSOR_MANUFACTURER,
                      BME_SENSOR_MODEL);
    String configDataString;
    serializeJson(configData, configDataString);
    const char *payload =  configDataString.c_str();
    #ifdef PRINT_HA_CONFIGS
    Serial.println(topic);
    Serial.println(payload);
    #endif // PRINT_HA_CONFIGS

    mqttClient.beginMessage(topic,
                            configDataString.length(), // size
                            true,                      // retain
                            1,                         // qos
                            false);                    // dup
    mqttClient.print(payload);
    mqttClient.endMessage();
  }
  #endif // MODULE_BME280_SENSOR

  // Configure Mi Thermometers
  for(std::map<std::string, std::string>::const_iterator sensor_it = MAC_NAME_MAPPING.begin();
      sensor_it != MAC_NAME_MAPPING.end();
      sensor_it++) {
    std::string mac_addr = sensor_it->first;
    std::string uid = stripColonsFromMac(mac_addr.c_str());
    std::string name = sensor_it->second;

    for (std::map<std::string, std::string>::const_iterator classAndUnit_it = MI_THERMOMETER_CLASS_UNIT_MAPPING.begin();
         classAndUnit_it != MI_THERMOMETER_CLASS_UNIT_MAPPING.end();
         classAndUnit_it++) {

      char topic[TOPIC_TEMPLATE_HA_CONFIG_SIZE];
      sprintf(topic,
              TOPIC_TEMPLATE_HA_CONFIG,
              uid.c_str(),
              classAndUnit_it->first.c_str());

      StaticJsonDocument<HA_JSON_CAPACITY> configData;
      formatConfigData(configData,
                        uid.c_str(),
                        name.c_str(),
                        classAndUnit_it->first.c_str(),
                        classAndUnit_it->second.c_str(),
                        MI_THERMOMETER_MANUFACTURER,
                        MI_THERMOMETER_MODEL);
      String configDataString;
      serializeJson(configData, configDataString);
      const char *payload =  configDataString.c_str();

      #ifdef PRINT_HA_CONFIGS
      Serial.println(topic);
      Serial.println(payload);
      #endif // PRINT_HA_CONFIGS

      mqttClient.beginMessage(topic,
                              configDataString.length(), // size
                              true,                      // retain
                              1,                         // qos
                              false);                    // dup
      mqttClient.print(payload);
      mqttClient.endMessage();
    }
  }
}

#endif //USE_HOME_ASSISTANT
