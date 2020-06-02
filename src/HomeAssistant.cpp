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
    char str[512];
    char str_topic[128];
    uint32_t chip_id = (uint32_t)ESP.getChipId();

    // tele/persiana_inventada1/LWT Online
    snprintf(str_topic, 128, "tele/%s/LWT", config->getHostname());
    this->mqtt->publish(str_topic, "Online");

    // cmnd/persiana_inventada1/POWER (null)
    snprintf(str_topic, 128, "cmnd/%s/POWER", config->getHostname());
    this->mqtt->publish(str_topic, NULL);

    // tele/persiana_inventada1/INFO1 {"Module":"Sonoff Dual R2","Version":"8.3.1(tasmota)","FallbackTopic":"cmnd/DVES_E03583_fb/","GroupTopic":"cmnd/tasmotas/"}
    snprintf(str, 512, "{\"Module\":\"Unknown Hardware\",\"Version\":\"%s\",\"FallbackTopic\":\"cmnd/DVES_%06X_fb/\",\"GroupTopic\":\"cmnd/shutters/\"}",
                        VERSION_FW, chip_id);
    snprintf(str_topic, 128, "tele/%s/INFO1", config->getHostname());
    this->mqtt->publish(str_topic, str);

    // tele/persiana_inventada1/INFO2 {"WebServerMode":"Admin","Hostname":"Persianainventada","IPAddress":"192.168.0.125"}
    snprintf(str, 512, "{\"WebServerMode\":\"Admin\",\"Hostname\":\"%s\",\"IPAddress\":\"%s\"}",
                        config->getHostname(), "192.168.0.32");
    snprintf(str_topic, 128, "tele/%s/INFO2", config->getHostname());
    this->mqtt->publish(str_topic, str);

    // tele/persiana_inventada1/INFO3 {"RestartReason":"Software/System restart"}
    // Not now
    uint8_t i;
    for(i=1;i<3;++i) {
        // stat/persiana_inventada1/RESULT {"POWER1":"OFF"}
        snprintf(str, 512, "{\"POWER%d\":\"OFF\"}", i);
        snprintf(str_topic, 128, "stat/%s/RESULT", config->getHostname());
        this->mqtt->publish(str_topic, str);

        // stat/persiana_inventada1/POWER1 OFF
        snprintf(str_topic, 128, "stat/%s/POWER%d", config->getHostname(), i);
        this->mqtt->publish(str_topic, "OFF");
    }

    // tele/persiana_inventada1/STATE {"Time":"2020-05-18T22:53:39","Uptime":"0T00:00:10","UptimeSec":10,"Heap":27,"SleepMode":"Dynamic","Sleep":50,"LoadAvg":19,"MqttCount":1,"POWER1":"OFF","POWER2":"OFF","Wifi":{"AP":1,"SSId":"plutec_IoT","BSSId":"2A:E8:29:FE:75:F3","Channel":6,"RSSI":82,"Signal":-59,"LinkCount":1,"Downtime":"0T00:00:04"}}
    // Not now

    // tele/persiana_inventada1/SENSOR {"Time":"2020-05-18T22:53:39","Shutter1":{"Position":0,"Direction":0,"Target":0}}
    snprintf(str, 512, "{\"Time\":\"2020-05-18T22:53:39\",\"Shutter1\":{\"Position\":0,\"Direction\":0,\"Target\":0}}");
    snprintf(str_topic, 128, "tele/%s/SENSOR", config->getHostname());
    this->mqtt->publish(str_topic, str);


    //El STATE se manda cada 5 minutos aproximadamente, junto con el SENSOR, así que no se manda en el SendDiscovery
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

    #define STRING_LENGTH 65
    char topics_to_clean_four[6][STRING_LENGTH] = {
        "homeassistant/device_automation/%06X_BTN_%d_SINGLE/config", 
        "homeassistant/device_automation/%06X_BTN_%d_DOUBLE/config", 
        "homeassistant/device_automation/%06X_BTN_%d_TRIPLE/config",
        "homeassistant/device_automation/%06X_BTN_%d_QUAD/config", 
        "homeassistant/device_automation/%06X_BTN_%d_PENTA/config", 
        "homeassistant/device_automation/%06X_BTN_%d_HOLD/config"}; // 1 - 4
    char topics_to_clean_eight[4][STRING_LENGTH] = {
        "homeassistant/device_automation/%06X_SW_%d_TOGGLE",
        "homeassistant/device_automation/%06X_SW_%d_HOLD",
        "homeassistant/binary_sensor/%06X_SW_%d/config",
        "homeassistant/light/%06X_LI_%d/config",
    };


    // homeassistant/device_automation/E03583_SW_%d_TOGGLE, homeassistant/device_automation/E03583_SW_%d_HOLD   // 1-8
    //homeassistant/binary_sensor/E03583_SW_%d/config  // 1-8
    //homeassistant/light/E03583_LI_%d/config // 1-8
    // homeassistant/switch/E03583_RL_%d/config // 3-8
    /*
homeassistant/device_automation/E03583_BTN_1_SINGLE/config (null)
homeassistant/device_automation/E03583_BTN_1_DOUBLE/config (null)
homeassistant/device_automation/E03583_BTN_1_TRIPLE/config (null)
homeassistant/device_automation/E03583_BTN_1_QUAD/config (null)
homeassistant/device_automation/E03583_BTN_1_PENTA/config (null)
homeassistant/device_automation/E03583_BTN_1_HOLD/config (null)
homeassistant/device_automation/E03583_BTN_2_SINGLE/config (null)
homeassistant/device_automation/E03583_BTN_2_DOUBLE/config (null)
homeassistant/device_automation/E03583_BTN_2_TRIPLE/config (null)
homeassistant/device_automation/E03583_BTN_2_QUAD/config (null)
homeassistant/device_automation/E03583_BTN_2_PENTA/config (null)
homeassistant/device_automation/E03583_BTN_2_HOLD/config (null)
homeassistant/device_automation/E03583_BTN_3_SINGLE/config (null)
homeassistant/device_automation/E03583_BTN_3_DOUBLE/config (null)
homeassistant/device_automation/E03583_BTN_3_TRIPLE/config (null)
homeassistant/device_automation/E03583_BTN_3_QUAD/config (null)
homeassistant/device_automation/E03583_BTN_3_PENTA/config (null)
homeassistant/device_automation/E03583_BTN_3_HOLD/config (null)
homeassistant/device_automation/E03583_BTN_4_SINGLE/config (null)
homeassistant/device_automation/E03583_BTN_4_DOUBLE/config (null)
homeassistant/device_automation/E03583_BTN_4_TRIPLE/config (null)
homeassistant/device_automation/E03583_BTN_4_QUAD/config (null)
homeassistant/device_automation/E03583_BTN_4_PENTA/config (null)
homeassistant/device_automation/E03583_BTN_4_HOLD/config (null)


homeassistant/device_automation/E03583_SW_1_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_1_HOLD/config (null)
homeassistant/device_automation/E03583_SW_2_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_2_HOLD/config (null)
homeassistant/device_automation/E03583_SW_3_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_3_HOLD/config (null)
homeassistant/device_automation/E03583_SW_4_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_4_HOLD/config (null)
homeassistant/device_automation/E03583_SW_5_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_5_HOLD/config (null)
homeassistant/device_automation/E03583_SW_6_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_6_HOLD/config (null)
homeassistant/device_automation/E03583_SW_7_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_7_HOLD/config (null)
homeassistant/device_automation/E03583_SW_8_TOGGLE/config (null)
homeassistant/device_automation/E03583_SW_8_HOLD/config (null)


homeassistant/binary_sensor/E03583_SW_1/config (null)
homeassistant/binary_sensor/E03583_SW_2/config (null)
homeassistant/binary_sensor/E03583_SW_3/config (null)
homeassistant/binary_sensor/E03583_SW_4/config (null)
homeassistant/binary_sensor/E03583_SW_5/config (null)
homeassistant/binary_sensor/E03583_SW_6/config (null)
homeassistant/binary_sensor/E03583_SW_7/config (null)
homeassistant/binary_sensor/E03583_SW_8/config (null)


homeassistant/light/E03583_LI_1/config (null)
homeassistant/light/E03583_LI_2/config (null)
homeassistant/light/E03583_LI_3/config (null)
homeassistant/light/E03583_LI_4/config (null)
homeassistant/light/E03583_LI_5/config (null)
homeassistant/light/E03583_LI_6/config (null)
homeassistant/light/E03583_LI_7/config (null)
homeassistant/light/E03583_LI_8/config (null)

homeassistant/switch/E03583_RL_3/config (null)
homeassistant/switch/E03583_RL_4/config (null)
homeassistant/switch/E03583_RL_5/config (null)
homeassistant/switch/E03583_RL_6/config (null)
homeassistant/switch/E03583_RL_7/config (null)
homeassistant/switch/E03583_RL_8/config (null)
*/ 

    uint8_t i, j;
    uint32_t chip_id = (uint32_t)ESP.getChipId();
    for(i=1;i<5;++i) { //Number of repeats
        for(j=0;j<6;++j) { //Number of strings
            snprintf(str_topic, 128, topics_to_clean_four[j], chip_id, i);
            this->mqtt->publish(str_topic, NULL);
        }
    }
    for(i=1;i<9;++i) { //Number of repeats
        for(j=0;j<4;++j) { //Number of strings
            snprintf(str_topic, 128, topics_to_clean_eight[j], chip_id, i);
            this->mqtt->publish(str_topic, NULL);
        }
    }
    for(i=3;i<9;++i) {
        snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_%d/config", chip_id, i);
        this->mqtt->publish(str_topic, NULL);
    }


    
    //this->mqtt->publish("persiana/debug", "por aqui vamos");
    //delay(500);
    /*
    for (uint8_t i=0;i<2;++i) {
        snprintf(str_topic, 128, "homeassistant/device_automation/%06X%s/config", (uint32_t)ESP.getChipId(), topics_to_clean[i]);
        this->mqtt->publish(str_topic, NULL);
    }*/

    snprintf(str, 512, "{\"name\":\"Relay1\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER1\",\"val_tpl\":\"{{value_json.POWER1}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_1\",\"dev\":{\"ids\":[\"%06X\"]}",
        config->getHostname(), config->getHostname(), config->getHostname(), chip_id, chip_id);
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_1/config", chip_id);
    this->mqtt->publish(str_topic, str, true); // Retained

    /* SWITCH2 */
    snprintf(str, 512, "{\"name\":\"Relay2\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER2\",\"val_tpl\":\"{{value_json.POWER2}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_2\",\"dev\":{\"ids\":[\"%06X\"]}",
        config->getHostname(), config->getHostname(), config->getHostname(), chip_id, chip_id);
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_2/config", chip_id);
    this->mqtt->publish(str_topic, str, true); // Retained
    
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
    
    snprintf(str, 512, "{\"name\":\"%s status\",\"stat_t\":\"tele/%s/HASS_STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"json_attr_t\":\"tele/%s/HASS_STATE\",\"unit_of_meas\":\"%%\",\"val_tpl\":\"{{value_json['RSSI']}}\",\"ic\":\"mdi:information-outline\",\"uniq_id\":\"%06X_status\",\"dev\":{\"ids\":[\"%06X\"],\"name\":\"%s\",\"mdl\":\"%s\",\"sw\":\"%s\",\"mf\":\"ShutterFW\"}}",
        config->getHostname(), config->getHostname(), config->getHostname(), config->getHostname(), (uint32_t)ESP.getChipId(), (uint32_t)ESP.getChipId(), config->getHostname(), "Unknown Hardware", VERSION_FW);
    snprintf(str_topic, 128, "homeassistant/sensor/%06X_status/config", (uint32_t)ESP.getChipId());
    this->mqtt->publish(str_topic, str, true); // Retained
    
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


