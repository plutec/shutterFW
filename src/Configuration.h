/*
  Configuration.h - Configuration class for ShutterFW
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/

#include <FS.h>
#include <ctype.h>

#ifndef Configuration_h
#define Configuration_h

#define VERSION_FW "0.6(shutterFW)"

struct storage_struct{ 
  bool new_values = false;
  uint8_t current_position = 100;
  // WiFi
  char wifiSsid[32];
  char wifiPass[64];
  // MQTT
  bool mqttEnabled = true;
  //bool mqttEnabled2 = true;
  //bool mqttEnabled3 = true;
  //bool mqttEnabled4 = true;
  char mqttServer[32];// = "192.168.0.50";
  uint32_t mqttPort = 1883;  // Default 1883
  char mqttUser[32];// = NULL;  // In case no user use NULL
  char mqttPassword[32];// = NULL; // In case no password use NULL
  // Device information
  char hostname[32] = "";
  char alexa_name[32];
  uint32_t openTime = 10;
  // Relays and buttons
  int8_t gpio_relay_up = -1;
  int8_t gpio_relay_down = -1;
  int8_t gpio_button_up = -1;
  int8_t gpio_button_down = -1;
  // HomeAssistant
  bool ha_enabled = false;
  char mqttTopic[32] = "";
  // Real positions
  bool calibrated_positions = false;
  uint8_t calibration[4]; //30%, 50%, 70%, 90%
};


class Configuration {
    private:
        
        void save_eeprom_data();
        storage_struct storage;
    public:
        Configuration();
        void begin();
        void memory_info();
        //bool news() { return storage.new_values; };
        //MQTT
        void setMQTTData(const char *server, int port, const char *user, const char *password);
        char* getMQTTServer();
        int getMQTTPort();
        char* getMQTTUser();
        char* getMQTTPassword();
        void setMQTTServer(const char *server);
        void setMQTTPort(int port);
        void setMQTTUser(const char *user);
        void setMQTTPassword(const char *password);
        // WiFi
        void setWifiSsid(const char *ssid);
        void setWifiPass(const char *pass);
        char* getWifiSsid();
        char* getWifiPass();
        // Device information
        void setHostname(const char *hostname);
        char* getHostname();
        void setAlexaName(const char *alexaname);
        char* getAlexaName();

        // Shutter management
        void setOpenTime(int time_sec);
        uint32_t getOpenTime();
        void setCurrentPosition(uint8_t current_position);
        uint8_t getCurrentPosition();
        uint8_t getCurrentPositionKA();
        
        // Relays and button
        bool isSetPinouts();
        void setPinRelayUp(int8_t pin);
        int8_t getPinRelayUp() { return storage.gpio_relay_up; }
        void setPinRelayDown(int8_t pin);
        int8_t getPinRelayDown() { return storage.gpio_relay_down; }
        void setPinButtonUp(int8_t pin);
        int8_t getPinButtonUp() { return storage.gpio_button_up; }
        void setPinButtonDown(int8_t pin);
        int8_t getPinButtonDown() { return storage.gpio_button_down; }

        // HomeAssistant
        bool homeAssistantEnabled() { return storage.ha_enabled; }
        char* homeAssistantEnabledChecked() { if (storage.ha_enabled) { return "checked";} return ""; } // I know this is not correct, but it's a microcontroller and the memory is very limited :(
        void setHomeAssistantEnabled(bool stat) { storage.ha_enabled = stat; storage.new_values = true;}
        void setMqttTopic(const char *topic);
        char* getMqttTopic();
        
        // Calibration
        void setCalibrationEnabled(bool stat) { storage.calibrated_positions = stat; storage.new_values = true;}
        void setCalibration(uint8_t index, uint8_t value) { storage.calibration[index] = value; storage.new_values = true; }
        char* calibrationEnabledChecked() { if (storage.calibrated_positions) { return "checked";} return ""; } // I know this is not correct, but it's a microcontroller and the memory is very limited :(
        uint8_t getCalibration(uint8_t index) { return storage.calibration[index]; }
        int8_t getCalibratedPosition(int8_t pos);

        // loop
        void loop();
};

#endif