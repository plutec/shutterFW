/*
  HomeAssistant.cpp - A very imple controller for HomeAssistant. Made for ShutterFW and based on Tasmota reversing.
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/


#include "HomeAssistant.h"
#include <ArduinoJson.h>

HomeAssistant::HomeAssistant() {
    this->mqtt = NULL;
    this->config = NULL;
}

HomeAssistant::HomeAssistant(PubSubClient* mqtt, Configuration* config) {
    this->mqtt = mqtt;
    this->config = config;
};

void HomeAssistant::SendState() {
    //El STATE se manda cada 5 minutos aproximadamente, junto con el SENSOR, así que no se manda en el SendDiscovery
    //tele/persiana_inventada1/STATE {"Time":"2020-05-18T23:34:00","Uptime":"0T00:38:03","UptimeSec":2283,"Heap":26,"SleepMode":"Dynamic","Sleep":50,"LoadAvg":19,"MqttCount":1,"POWER1":"ON","POWER2":"OFF","Wifi":{"AP":1,"SSId":"plutec_IoT","BSSId":"2A:E8:29:FE:75:F3","Channel":6,"RSSI":76,"Signal":-62,"LinkCount":1,"Downtime":"0T00:00:04"}}
    //tele/persiana_inventada1/SENSOR {"Time":"2020-05-18T23:36:07","Shutter1":{"Position":16,"Direction":0,"Target":16}}

}

void HomeAssistant::SendDiscovery() {
    //this->_mqtt->publish("esp/persiana", "Test desde homeassistant");

    //mqtt.publish("homeassistant/cover/persiana_despacho/config", json_homeassistant, true);  // Retained
    /*
    homeassistant/switch/E03583_RL_1/config 
    {
	"name": "Rele1 Rele2",
	"stat_t": "tele/persiana_inventada1/STATE",
	"avty_t": "tele/persiana_inventada1/LWT",
	"pl_avail": "Online",
	"pl_not_avail": "Offline",
	"cmd_t": "cmnd/persiana_inventada1/POWER1",
	"val_tpl": "{{value_json.POWER1}}",
	"pl_off": "OFF",
	"pl_on": "ON",
	"uniq_id": "E03583_RL_1",
	"dev": {
		"ids": ["E03583"]
	}
}*/
    /*
    homeassistant/switch/E03583_RL_2/config
    {
	"name": "Rele1 Tasmota2",
	"stat_t": "tele/persiana_inventada1/STATE",
	"avty_t": "tele/persiana_inventada1/LWT",
	"pl_avail": "Online",
	"pl_not_avail": "Offline",
	"cmd_t": "cmnd/persiana_inventada1/POWER2",
	"val_tpl": "{{value_json.POWER2}}",
	"pl_off": "OFF",
	"pl_on": "ON",
	"uniq_id": "E03583_RL_2",
	"dev": {
		"ids": ["E03583"]
	}
}
    */

    /*
    homeassistant/sensor/E03583_status/config 
    {
	"name": "Rele1 status",
	"stat_t": "tele/persiana_inventada1/HASS_STATE",
	"avty_t": "tele/persiana_inventada1/LWT",
	"pl_avail": "Online",
	"pl_not_avail": "Offline",
	"json_attr_t": "tele/persiana_inventada1/HASS_STATE",
	"unit_of_meas": "%",
	"val_tpl": "{{value_json['RSSI']}}",
	"ic": "mdi:information-outline",
	"uniq_id": "E03583_status",
	"dev": {
		"ids": ["E03583"],
		"name": "Rele1",
		"mdl": "Sonoff Dual R2",
		"sw": "8.3.1(tasmota)",
		"mf": "Tasmota"
	}
}
    */
    /* SWITCH1 */
    /*String jsonstr;
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(11);
    DynamicJsonDocument doc(capacity);

    doc["name"] = "Persiana Despacho Giu";
    doc["state_topic"] = "tele/persiana_despacho_giu/STATE";
    doc["availability_topic"] = "tele/persiana_despacho_giu/LWT";
    doc["payload_available"] = "Online";
    doc["payload_not_available"] = "Offline";
    doc["command_topic"] = "cmnd/persiana_despacho_giu/POWER1";
    doc["value_template"] = "{{value_json.POWER1}}";
    doc["payload_off"] = "OFF";
    doc["payload_on"] = "ON";
    doc["unique_id"] = ESP.getChipId();

    JsonObject device = doc.createNestedObject("device");
    JsonArray device_identifiers = device.createNestedArray("identifiers");
    device_identifiers.add(ESP.getChipId());

    //serializeJson(doc, Serial);
    serializeJson(doc, jsonstr);*/
    char str[512];
    char str_topic[128];
    snprintf(str, 512, "{\"name\":\"%s Relay1\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER1\",\"val_tpl\":\"{{value_json.POWER1}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_1\",\"dev\":{\"ids\":[\"%06X\"]}",
        config->getHostname(), config->getHostname(), config->getHostname(), config->getHostname(), (uint32_t)ESP.getChipId(), (uint32_t)ESP.getChipId());
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_1/config", (uint32_t)ESP.getChipId());
    this->mqtt->publish(str_topic, str);

    /* SWITCH2 */
    snprintf(str, 512, "{\"name\":\"%s Relay2\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER2\",\"val_tpl\":\"{{value_json.POWER2}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_2\",\"dev\":{\"ids\":[\"%06X\"]}",
        config->getHostname(), config->getHostname(), config->getHostname(), config->getHostname(), (uint32_t)ESP.getChipId(), (uint32_t)ESP.getChipId());
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_1/config", (uint32_t)ESP.getChipId());
    this->mqtt->publish(str_topic, str);
    
    /* SENSOR */
    //doc.clear();
    //doc = DynamicJsonDocument(capacity);
    /*
    doc["name"] = "Persiana Despacho status";
    doc["stat_t"] = "tele/persiana_despacho_giu/HASS_STATE";
    doc["avty_t"] = "tele/persiana_despacho_giu/LWT";
    doc["pl_avail"] = "Online";
    doc["pl_not_avail"] = "Offline";
    doc["json_attr_t"] = "tele/persiana_despacho_giu/HASS_STATE";
    doc["unit_of_meas"] = "%";
    doc["val_tpl"] = "{{value_json['RSSI']}}";
    doc["ic"] = "mdi:information-outline";
    doc["uniq_id"] = String(ESP.getChipId())+"_status";

    JsonObject dev = doc.createNestedObject("dev");
    JsonArray dev_ids = dev.createNestedArray("ids");
    dev_ids.add(ESP.getChipId());
    dev["name"] = "Persiana Despacho Giu";
    //dev["mdl"] = HARDWARE_DEVICE_NAME;
    //dev["sw"] = VERSION_FW;
    //dev["mf"] = NAME_FW;

    serializeJson(doc, jsonstr);
    this->_mqtt->publish("homeassistant/cover/97669F_status/config", jsonstr.c_str());
    */
    
    snprintf(str, 512, "{\"name\":\"%s status\",\"stat_t\":\"tele/%s/HASS_STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"json_attr_t\":\"tele/%s/HASS_STATE\",\"unit_of_meas\":\"%\",\"val_tpl\":\"{{value_json['RSSI']}}\",\"ic\":\"mdi:information-outline\",\"uniq_id\":\"%06X_status\",\"dev\":{\"ids\":[\"%06X\"],\"name\":\"%s\",\"mdl\":\"%s\",\"sw\":\"%s\",\"mf\":\"ShutterFW\"}}",
        config->getHostname(), config->getHostname(), config->getHostname(), config->getHostname(), (uint32_t)ESP.getChipId(), (uint32_t)ESP.getChipId(), config->getHostname(), "wachufrito TODO", VERSION_FW);
    snprintf(str_topic, 128, "homeassistant/sensor/%06X_status/config", (uint32_t)ESP.getChipId());
    this->mqtt->publish(str_topic, str);
    
   //ESP.getChipId()
}


/*
void create_json_ha() {
  StaticJsonDocument<200> doc;

  // StaticJsonObject allocates memory on the stack, it can be
  // replaced by DynamicJsonDocument which allocates in the heap.
  //
  // DynamicJsonDocument  doc(200);

  // Add values in the document
  //
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;
}
*/

/***** HOMEASSISTANT *****/
// https://www.home-assistant.io/docs/mqtt/discovery/

/*
Configuration topic: homeassistant/cover/despacho_giu/config
State topic: homeassistant/cover/despacho_giu/state
Command topic: homeassistant/cover/despacho_giu/set
*/

/**
 * Cuando abre: state_topic -> state_opening
 * Cuando termina de abrir: state_topic -> state_open
 * Cuando cierra: state_topic -> state_closing
 * Cuando termina de cerrar: state_topic -> state_close
 * Porcentajes: (position_topic)
 * 0 -> Closed
 * 100 -> Open
 * 
 * 
 * 
 * */
void HomeAssistant::SendStatus(uint8_t percent, bool relay1, bool relay2){ 
    // El status se manda cuando hay movimiento.
    /*
    stat/persiana_inventada1/RESULT {"POWER1":"ON"}
stat/persiana_inventada1/POWER1 ON
stat/persiana_inventada1/RESULT {"ShutterPosition1":100}
stat/persiana_inventada1/RESULT {"Shutter1":{"Position":0,"Direction":1,"Target":100}}
stat/persiana_inventada1/RESULT {"Shutter1":{"Position":5,"Direction":1,"Target":100}}
stat/persiana_inventada1/RESULT {"ShutterPosition1":16}
stat/persiana_inventada1/RESULT {"Shutter1":{"Position":15,"Direction":1,"Target":16}}
tele/persiana_inventada1/STATE {"Time":"2020-05-18T23:34:02","Uptime":"0T00:38:05","UptimeSec":2285,"Heap":26,"SleepMode":"Dynamic","Sleep":50,"LoadAvg":19,"MqttCount":1,"POWER1":"OFF","POWER2":"OFF","Wifi":{"AP":1,"SSId":"plutec_IoT","BSSId":"2A:E8:29:FE:75:F3","Channel":6,"RSSI":76,"Signal":-62,"LinkCount":1,"Downtime":"0T00:00:04"}}
stat/persiana_inventada1/RESULT {"POWER1":"OFF"}
stat/persiana_inventada1/POWER1 OFF
stat/persiana_inventada1/SHUTTER1 16
stat/persiana_inventada1/RESULT {"Shutter1":{"Position":16,"Direction":0,"Target":16}}
*/

}


