#include "Module_HomeAssistant.h"

#ifndef USE_MAC_NAME_MAPPINGS
#error "To use Home Assistant integration, you need to configure MAC name mappings.\nOnly known devices will be auto configured with Home Assistant!"
#endif // USE_MAC_NAME_MAPPINGS

JSONVar formatConfigData(const char *uid,
                         const char *name,
                         const char *device_class,
                         const char *unit,
                         const char *manufacturer,
                         const char *model) {
  JSONVar availabilityObject;
  #define JSON_TEMPLATE_HA "{\"value_template\":\"{{ value_json.%s }}\"}"
  availabilityObject["topic"] = MQTT_TOPIC_STATE;
  JSONVar availabilityList;
  availabilityList[0] = availabilityObject;

  char state_topic[strlen(MQTT_TOPIC) + 1 + strlen(uid) + 1];
  sprintf(state_topic, "%s/%s", MQTT_TOPIC, uid);

  char name_with_class[strlen(name) + 1 + strlen(device_class) + 1];
  sprintf(name_with_class, "%s %s", name, device_class);

  JSONVar deviceObject;
  JSONVar deviceIdentifiersList;
  char device_idetifier[strlen(IDENTIFIER_PREFIX_HA) + strlen(uid) + 1];
  sprintf(device_idetifier, "%s%.12s", IDENTIFIER_PREFIX_HA, uid);
  deviceIdentifiersList[0] = device_idetifier;
  deviceObject["identifiers"]  = deviceIdentifiersList;
  deviceObject["manufacturer"] = manufacturer;
  deviceObject["model"]        = model;
  deviceObject["name"]         = name;
  deviceObject["sw_version"]   = APP_VERSION;

  char unique_id[strlen(IDENTIFIER_PREFIX_HA) + strlen(uid) + 1 + strlen(device_class) + 1];
  sprintf(unique_id, "%s%.12s_%.15s", IDENTIFIER_PREFIX_HA, uid, device_class);

  char value_template[VALUE_TEMPLATE_TEMPLATE_HA_CONFIG_SIZE];
  sprintf(value_template, VALUE_TEMPLATE_TEMPLATE_HA_CONFIG, device_class);

  JSONVar configData;
  configData["availability"]          = availabilityList;
  configData["state_topic"]           = state_topic;
  configData["json_attributes_topic"] = state_topic;
  configData["name"]                  = name_with_class;
  configData["device"]                = deviceObject;
  configData["device_class"]          = device_class;
  configData["unit_of_measurement"]   = unit;
  configData["unique_id"]             = unique_id;
  configData["value_template"]        = value_template;

  return configData;
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

    JSONVar configData = formatConfigData(board_uid,
                                          BME_SENSOR_NAME,
                                          classAndUnit_it->first.c_str(),
                                          classAndUnit_it->second.c_str(),
                                          BME_SENSOR_MANUFACTURER,
                                          BME_SENSOR_MODEL);
    String configDataString = JSON.stringify(configData);
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

      JSONVar configData = formatConfigData(uid.c_str(),
                                            name.c_str(),
                                            classAndUnit_it->first.c_str(),
                                            classAndUnit_it->second.c_str(),
                                            MI_THERMOMETER_MANUFACTURER,
                                            MI_THERMOMETER_MODEL);
      String configDataString = JSON.stringify(configData);
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
