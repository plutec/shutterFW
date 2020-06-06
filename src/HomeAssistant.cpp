/*
  HomeAssistant.cpp - A very simple controller for HomeAssistant. Made for ShutterFW and based on Tasmota reversing.
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
    this->chip_id = (uint32_t)ESP.getChipId();
};

void HomeAssistant::SendState() {
    char str[512];
    char str_topic[128];
    char topic[32];
    strncpy(topic, config->getMqttTopic(), sizeof(topic));

    // tele/persiana_inventada1/LWT Online
    snprintf(str_topic, 128, "tele/%s/LWT", topic);
    this->mqtt->publish(str_topic, "Online", true); //Retained

    // cmnd/persiana_inventada1/POWER (null)
    snprintf(str_topic, 128, "cmnd/%s/POWER", topic);
    this->mqtt->publish(str_topic, NULL);

    // tele/persiana_inventada1/INFO1 {"Module":"Sonoff Dual R2","Version":"8.3.1(tasmota)","FallbackTopic":"cmnd/DVES_E03583_fb/","GroupTopic":"cmnd/tasmotas/"}
    snprintf(str, 512, "{\"Module\":\"Unknown Hardware\",\"Version\":\"%s\",\"FallbackTopic\":\"cmnd/DVES_%06X_fb/\",\"GroupTopic\":\"cmnd/shutters/\"}",
                        VERSION_FW, chip_id);
    snprintf(str_topic, 128, "tele/%s/INFO1", topic);
    this->mqtt->publish(str_topic, str);

    // tele/persiana_inventada1/INFO2 {"WebServerMode":"Admin","Hostname":"Persianainventada","IPAddress":"192.168.0.125"}
    snprintf(str, 512, "{\"WebServerMode\":\"Admin\",\"Hostname\":\"%s\",\"IPAddress\":\"%s\"}",
                        topic, WiFi.localIP().toString().c_str());
    snprintf(str_topic, 128, "tele/%s/INFO2", topic);
    this->mqtt->publish(str_topic, str);

    // tele/persiana_inventada1/INFO3 {"RestartReason":"Software/System restart"}
    // Not now

    for(uint8_t i=1;i<3;++i) {
        // stat/persiana_inventada1/RESULT {"POWER1":"OFF"}
        snprintf(str, 512, "{\"POWER%d\":\"OFF\"}", i);
        snprintf(str_topic, 128, "stat/%s/RESULT", topic);
        this->mqtt->publish(str_topic, str);

        // stat/persiana_inventada1/POWER1 OFF
        snprintf(str_topic, 128, "stat/%s/POWER%d", topic, i);
        this->mqtt->publish(str_topic, "OFF", true); //Retained
    }

    // tele/persiana_inventada1/STATE {"Time":"2020-05-18T22:53:39....
    /*snprintf(str, 512, "{\"Time\":\"2020-05-18T22:53:39\",\"Uptime\":\"0T00:00:10\",\"UptimeSec\":10,\"Heap\":27,\"SleepMode\":\"Dynamic\",\"Sleep\":50,\"LoadAvg\":19,\"MqttCount\":1,\"POWER1\":\"OFF\",\"POWER2\":\"OFF\",\"Wifi\":{\"AP\":1,\"SSId\":\"Test_IoT\",\"BSSId\":\"2E:E8:30:F2:73:F7\",\"Channel\":6,\"RSSI\":82,\"Signal\":-59,\"LinkCount\":1,\"Downtime\":\"0T00:00:04\"}}");
    snprintf(str_topic, 128, "tele/%s/STATE", config->getHostname());
    this->mqtt->publish(str_topic, str);*/

    // tele/persiana_inventada1/SENSOR {"Time":"2020-05-18T22:53:39","Shutter1":{"Position":0,"Direction":0,"Target":0}}
    snprintf(str, 512, "{\"Shutter1\":{\"Position\":%d,\"Direction\":0,\"Target\":%d}}", 
                        config->getCurrentPosition(), config->getCurrentPosition());
    snprintf(str_topic, 128, "tele/%s/SENSOR", topic);
    this->mqtt->publish(str_topic, str);

}
/**
 * percent: current position
 * target: final objective position (possible 0 or 100)
 * direction: 1 UP, -1 DOWN, 0 STOP.
 */
void HomeAssistant::SendUpdate(uint8_t percent, uint8_t target, int8_t direction) {
    char str[512];
    char str_topic[128];

    // stat/persiana_inventada1/RESULT {"Shutter1":{"Position":54,"Direction":1,"Target":100}}
    snprintf(str_topic, 128, "stat/%s/RESULT", config->getMqttTopic());
    snprintf(str, 512, "{\"Shutter1\":{\"Position\":%d,\"Direction\":%d,\"Target\":%d}}", percent, direction, target);
    this->mqtt->publish(str_topic, str);
}

void HomeAssistant::SendDiscovery() {

    char str[512];
    char str_topic[128];
    char topic[32];
    strncpy(topic, config->getMqttTopic(), sizeof(topic));
        

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

    uint8_t i, j;
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

    snprintf(str, 512, "{\"name\":\"Relay1\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER1\",\"val_tpl\":\"{{value_json.POWER1}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_1\",\"dev\":{\"ids\":[\"%06X\"]}",
        topic, topic, topic, chip_id, chip_id);
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_1/config", chip_id);
    this->mqtt->publish(str_topic, str, true); // Retained

    /* SWITCH2 */
    snprintf(str, 512, "{\"name\":\"Relay2\",\"stat_t\":\"tele/%s/STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"cmnd/%s/POWER2\",\"val_tpl\":\"{{value_json.POWER2}}\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"uniq_id\":\"%06X_RL_2\",\"dev\":{\"ids\":[\"%06X\"]}",
        topic, topic, topic, chip_id, chip_id);
    snprintf(str_topic, 128, "homeassistant/switch/%06X_RL_2/config", chip_id);
    this->mqtt->publish(str_topic, str, true); // Retained
        
    snprintf(str, 512, "{\"name\":\"%s status\",\"stat_t\":\"tele/%s/HASS_STATE\",\"avty_t\":\"tele/%s/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"json_attr_t\":\"tele/%s/HASS_STATE\",\"unit_of_meas\":\"%%\",\"val_tpl\":\"{{value_json['RSSI']}}\",\"ic\":\"mdi:information-outline\",\"uniq_id\":\"%06X_status\",\"dev\":{\"ids\":[\"%06X\"],\"name\":\"%s\",\"mdl\":\"%s\",\"sw\":\"%s\",\"mf\":\"ShutterFW\"}}",
        topic, topic, topic, topic, chip_id, chip_id, topic, "Unknown Hardware", VERSION_FW);
    snprintf(str_topic, 128, "homeassistant/sensor/%06X_status/config", chip_id);
    this->mqtt->publish(str_topic, str, true); // Retained
}
