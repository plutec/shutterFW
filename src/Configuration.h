
#include <FS.h>
//#include <LittleFS.h>
#include <ctype.h>

#ifndef Configuration_h
#define Configuration_h

#define VERSION_FW "0.0.1(shutterFW)"

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
  char hostname[32] = "ShutterFW";
  char alexa_name[32];
  uint32_t openTime = 10;
  // Relays and buttons
  int8_t gpio_relay_up = -1;
  int8_t gpio_relay_down = -1;
  int8_t gpio_button_up = -1;
  int8_t gpio_button_down = -1;
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

        // loop
        void loop();
};

#endif