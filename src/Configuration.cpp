/*
  Configuration.cpp - Configuration class for ShutterFW
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/

#include "Configuration.h"

Configuration::Configuration() {
    if (storage.hostname[0]=='\0') {
        snprintf(storage.hostname, 32, "SHUTTERFW-%06X", (uint32_t)ESP.getChipId()); 
    }
}

// eeprom management
void Configuration::save_eeprom_data() {
  if (storage.new_values == true) {      
    storage.new_values = false;
    File fd = SPIFFS.open("/conf.txt", "w");
    if (fd) {
        fd.write((uint8_t*)&storage, sizeof(struct storage_struct));
        fd.close();
    }
  }
}

void Configuration::begin() {
    SPIFFS.begin();
    File fd = SPIFFS.open("/conf.txt", "r");
    if (!fd) {
        SPIFFS.format();
    }
    if (fd) {
        fd.read((uint8_t*)&storage, sizeof(struct storage_struct));
        fd.close();
    }
    storage.new_values = false;
}

void Configuration::memory_info() {
    SPIFFS.begin();
    FSInfo fs_info;
    SPIFFS.info(fs_info);

    float fileTotalKB = (float)fs_info.totalBytes / 1024.0;
    float fileUsedKB = (float)fs_info.usedBytes / 1024.0;

    float flashChipSize = (float)ESP.getFlashChipSize() / 1024.0 / 1024.0;
    float realFlashChipSize = (float)ESP.getFlashChipRealSize() / 1024.0 / 1024.0;
    float flashFreq = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
    FlashMode_t ideMode = ESP.getFlashChipMode();

    Serial.println("==========================================================");
    Serial.println("Firmware: ");
    Serial.printf(" Chip Id: %08X\n", ESP.getChipId());
    Serial.print(" Core version: "); Serial.println(ESP.getCoreVersion());
    Serial.print(" SDK version: "); Serial.println(ESP.getSdkVersion());
    Serial.print(" Boot version: "); Serial.println(ESP.getBootVersion());
    Serial.print(" Boot mode: "); Serial.println(ESP.getBootMode());
    Serial.printf("__________________________\n\n");

    Serial.println("Flash chip information: ");
    Serial.printf(" Flash chip Id: %08X (for example: Id=001640E0 Manuf=E0, Device=4016 (swap bytes))\n", ESP.getFlashChipId());
    Serial.printf(" Sketch thinks Flash RAM is size: "); Serial.print(flashChipSize); Serial.println(" MB");
    Serial.print(" Actual size based on chip Id: "); Serial.print(realFlashChipSize); Serial.println(" MB ... given by (2^( \"Device\" - 1) / 8 / 1024");
    Serial.print(" Flash frequency: "); Serial.print(flashFreq); Serial.println(" MHz");
    Serial.printf(" Flash write mode: %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    Serial.printf(" CPU frequency: %u MHz\n\n", ESP.getCpuFreqMHz());
    Serial.printf("__________________________\n\n");

    Serial.println("File system (SPIFFS): ");
    Serial.print(" Total KB: "); Serial.print(fileTotalKB); Serial.println(" KB");
    Serial.print(" Used KB: "); Serial.print(fileUsedKB); Serial.println(" KB");
    Serial.printf(" Block size: %lu\n", fs_info.blockSize);
    Serial.printf(" Page size: %lu\n", fs_info.pageSize);
    Serial.printf(" Maximum open files: %lu\n", fs_info.maxOpenFiles);
    Serial.printf(" Maximum path length: %lu\n\n", fs_info.maxPathLength);
    Serial.printf("__________________________\n\n");
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
    //storage.mqttEnabled = true;
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
    for (int i=0;i<strlen(storage.mqttServer);++i) {
        if (!isprint(storage.mqttServer[i])) {
            storage.mqttServer[0] = '\0';
            storage.new_values = true;
        }
    }
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
    for (int i=0;i<strlen(storage.wifiSsid);++i) {
        if (!isprint(storage.wifiSsid[i])) {
            storage.wifiSsid[0] = '\0';
            storage.new_values = true;
        }
    }
    return storage.wifiSsid;
}

char* Configuration::getWifiPass() {
    for (size_t i=0;i<strlen(storage.wifiPass);++i) {
        if (!isprint(storage.wifiPass[i])) {
            storage.wifiPass[0] = '\0';
            storage.new_values = true;
        }
    }
    return storage.wifiPass;
}

void Configuration::setHostname(const char *hostname) {    
    strncpy(storage.hostname, hostname, sizeof(storage.hostname));
    storage.new_values = true;
}

char* Configuration::getHostname() {
    for (size_t i=0;i<strlen(storage.hostname);++i) {
        if (!isprint(storage.hostname[i])) {
            storage.hostname[0] = '\0';
            storage.new_values = true;
        }
    }
    return storage.hostname;
}

void Configuration::setAlexaName(const char *alexa_name) {
    strncpy(storage.alexa_name, alexa_name, sizeof(storage.alexa_name));
    storage.new_values = true;
}

char* Configuration::getAlexaName() {
    if (storage.alexa_name[0] == '\0') {
        return "Alexa device";
    }
    return storage.alexa_name;
}

// Shutter management
void Configuration::setOpenTime(int time_sec) {
    storage.openTime = time_sec;
    storage.new_values = true;
}

uint32_t Configuration::getOpenTime() {
    return storage.openTime;
}

void Configuration::setCurrentPosition(uint8_t current_position) {
    storage.current_position = current_position;
    storage.new_values = true;
}

uint8_t Configuration::getCurrentPosition() {
    if (storage.current_position > 100) {
        storage.current_position = 100;
    }
    return storage.current_position;
}

uint8_t Configuration::getCurrentPositionKA() {
    uint8_t to_ret;
    if (storage.current_position > 100) {
        storage.current_position = 100;
    }
    to_ret = 100-storage.current_position;
    return to_ret;
}

bool Configuration::isSetPinouts() {
    bool to_ret = true;
    if (storage.gpio_relay_up == -1) {
        to_ret = false;
    }
    if (storage.gpio_relay_down == -1) {
        to_ret = false;
    }
    if (storage.gpio_button_up == -1) {
        to_ret = false;
    }
    if (storage.gpio_button_down == -1) {
        to_ret = false;
    }
    return to_ret;
}

void Configuration::setPinRelayUp(int8_t pin) {
    storage.gpio_relay_up = pin;
    storage.new_values = true;
}

void Configuration::setPinRelayDown(int8_t pin) {
    storage.gpio_relay_down = pin;
    storage.new_values = true;
}

void Configuration::setPinButtonUp(int8_t pin) {
    storage.gpio_button_up = pin;
    storage.new_values = true;
}

void Configuration::setPinButtonDown(int8_t pin) {
    storage.gpio_button_down = pin;
    storage.new_values = true;
}

// HomeAssistant
void Configuration::setMqttTopic(const char *topic) {
    strncpy(storage.mqttTopic, topic, sizeof(storage.mqttTopic));
    storage.new_values = true;
}

char* Configuration::getMqttTopic() {
    if (storage.mqttTopic[0] == '\0') {
        sprintf(storage.mqttTopic, "shutterfw_%06X", (uint32_t)ESP.getChipId());
    }
    return storage.mqttTopic;
}

int8_t Configuration::getCalibratedPosition(int8_t pos) {
    //Ask for the offset position, and the shutter needs the real. If user wants the shutter at 50%, we need to supply 65, for instance.
    int8_t to_ret;

    if (storage.calibrated_positions) {
        #ifdef DEBUG
            Serial.print("Calculate the calibrated value of ");
            Serial.println(pos);
        #endif
        if (pos < 30) {
           to_ret = pos*storage.calibration[0]/30;
        } else if (pos==30) {
            to_ret = storage.calibration[0];
        } else if (pos<50) { // (30-50)
            to_ret = ((float)(storage.calibration[1]-storage.calibration[0])/20)*(pos-30)+storage.calibration[0];
        } else if (pos==50) {
            to_ret = storage.calibration[1];
        } else if (pos<70) { // (50-70)
            to_ret = ((float)(storage.calibration[2]-storage.calibration[1])/20)*(pos-50)+storage.calibration[1];
        } else if (pos==70) {
            to_ret = storage.calibration[2];
        } else if (pos<90) { // (70-90)
            to_ret = ((float)(storage.calibration[3]-storage.calibration[2])/20)*(pos-70)+storage.calibration[2];
        } else if (pos==90) {
            to_ret = storage.calibration[3];
        } else { // (90 - 100]
            to_ret = ((float)(100-storage.calibration[3])/10)*(pos-90)+storage.calibration[3];
        }
        #ifdef DEBUG
            Serial.print("Calibrated value is: ");
            Serial.println(to_ret);
        #endif
        return to_ret;
    }
    return pos;
}
