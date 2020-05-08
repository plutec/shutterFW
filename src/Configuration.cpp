
#include "Configuration.h"

Configuration::Configuration() {
  EEPROM.begin(sizeof(storage_struct)); //bytes
  EEPROM.get(0, storage);
}

// eeprom management
void Configuration::save_eeprom_data() {
  if (storage.new_values == true) {
    storage.new_values = false;
    EEPROM.put(0, storage);
    EEPROM.commit();
  }
}

void Configuration::loop() {
    save_eeprom_data();
}
// MQTT
void Configuration::setMQTTData(const char *server, int port, const char *user, const char *password) {
    strncpy(storage.mqttServer, server, sizeof(storage.mqttServer));
    storage.mqttPort = port;
    strncpy(storage.mqttUser, user, sizeof(storage.mqttUser));
    strncpy(storage.mqttPassword, password, sizeof(storage.mqttPassword));
    storage.new_values = true;
}

void Configuration::setMQTTServer(const char *server) {
    strncpy(storage.mqttServer, server, sizeof(storage.mqttServer));
    storage.new_values = true;
}

void Configuration::setMQTTPort(int port) {
    storage.mqttPort = port;
    storage.new_values = true;
}

void Configuration::setMQTTUser(const char *user) {
    strncpy(storage.mqttUser, user, sizeof(storage.mqttUser));
    storage.new_values = true;
}

void Configuration::setMQTTPassword(const char *password) {
    strncpy(storage.mqttPassword, password, sizeof(storage.mqttPassword));
    storage.new_values = true;
}

char* Configuration::getMQTTServer() {
    return storage.mqttServer;
}

int Configuration::getMQTTPort() {
    return storage.mqttPort;
}

char* Configuration::getMQTTUser() {
    if (storage.mqttUser[0] == '\0') {
        return NULL;
    }
    return storage.mqttUser;
}

char* Configuration::getMQTTPassword() {
    if (storage.mqttPassword[0] == '\0') {
        return NULL;
    }
    return storage.mqttPassword;
}

// Position
void Configuration::setCurrentPosition(int current_position) {
    storage.current_position = current_position;
    storage.new_values = true;
}

int Configuration::getCurrentPosition() {
    return storage.current_position;
}

// WiFi
void Configuration::setWifiSsid(const char *ssid) {
    strncpy(storage.wifiSsid, ssid, sizeof(storage.wifiSsid));
    storage.new_values = true;
}

void Configuration::setWifiPass(const char *pass) {
    strncpy(storage.wifiPass, pass, sizeof(storage.wifiPass));
    storage.new_values = true;
}

char* Configuration::getWifiSsid() {
    if (storage.wifiSsid[0] == '\0') {
        return NULL;
    }
    return storage.wifiSsid;
}

char* Configuration::getWifiPass() {
    if (storage.wifiPass[0] == '\0') {
        return NULL;
    }
    return storage.wifiPass;
}