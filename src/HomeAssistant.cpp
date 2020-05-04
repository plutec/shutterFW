/*
  HomeAssistant.cpp - A simple controller for HomeAssistant.
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/


#include "HomeAssistant.h"
#include <ArduinoJson.h>
//#include "config.h"
HomeAssistant::HomeAssistant() {
    this->_mqtt = NULL;
}

HomeAssistant::HomeAssistant(PubSubClient* mqtt) {
    this->_mqtt = mqtt;
};

void HomeAssistant::SendDiscovery() {
    //this->_mqtt->publish("esp/persiana", "Test desde homeassistant");

    //mqtt.publish("homeassistant/cover/persiana_despacho/config", json_homeassistant, true);  // Retained
    /*
    homeassistant/switch/97669F_RL_1/config 
        {
        "name": "Persiana patio",
        "state_topic": "tele/persiana_patio/STATE",
        "availability_topic": "tele/persiana_patio/LWT",
        "payload_available": "Online",
        "payload_not_available": "Offline",
        "command_topic": "cmnd/persiana_patio/POWER1",
        "value_template": "{{value_json.POWER1}}",
        "payload_off": "OFF",
        "payload_on": "ON",
        "unique_id": "97669F_RL_1",
        "device": {
            "identifiers": ["97669F"]
        }
    }*/

    /*
    homeassistant/sensor/97669F_status/config 
    {
	"name": "Persiana patio status",
	"stat_t": "tele/persiana_patio/HASS_STATE",
	"avty_t": "tele/persiana_patio/LWT",
	"pl_avail": "Online",
	"pl_not_avail": "Offline",
	"json_attr_t": "tele/persiana_patio/HASS_STATE",
	"unit_of_meas": "%",
	"val_tpl": "{{value_json['RSSI']}}",
	"ic": "mdi:information-outline",
	"uniq_id": "97669F_status",
	"dev": {
		"ids": ["97669F"],
		"name": "Persiana patio",
		"mdl": "Sonoff Dual R2",
		"sw": "8.2.0(tasmota)",
		"mf": "Tasmota"
	}
}
    */
    /* SWITCH */
    String jsonstr;
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
    serializeJson(doc, jsonstr);
    
    this->_mqtt->publish("homeassistant/switch/persiana_despacho/config", jsonstr.c_str());

    /* SENSOR */
    doc.clear();
    //doc = DynamicJsonDocument(capacity);

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


    
   //ESP.getChipId()
};


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


/**
 * Mandar status (cada 5 minutos)
 * tele/persiana_patio/HASS_STATE {"Version":"8.2.0(tasmota)","BuildDateTime":"2020-03-20T14:45:23","Core":"STAGE","SDK":"2.2.2-dev(38a443e)","Module":"Sonoff Dual R2","RestartReason":"Software/System restart","Uptime":"3T06:00:14","WiFi LinkCount":2,"WiFi Downtime":"0T00:01:50","MqttCount":2,"BootCount":51,"SaveCount":886,"IPAddress":"192.168.0.114","RSSI":"80","LoadAvg":19}
tele/persiana_patio/STATE {"Time":"2020-05-04T17:05:17","Uptime":"3T06:00:25","UptimeSec":280825,"Heap":26,"SleepMode":"Dynamic","Sleep":50,"LoadAvg":19,"MqttCount":2,"POWER1":"OFF","POWER2":"OFF","Wifi":{"AP":1,"SSId":"plutec_IoT","BSSId":"2A:E8:29:FE:75:F3","Channel":6,"RSSI":76,"Signal":-62,"LinkCount":2,"Downtime":"0T00:01:50"}}
tele/persiana_patio/SENSOR {"Time":"2020-05-04T17:05:17","Switch1":"OFF","Switch2":"OFF","Shutter1":{"Position":98,"Direction":0,"Target":97}}

*/
