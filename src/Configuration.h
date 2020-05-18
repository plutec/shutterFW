
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
  char hostname[32] = "ESPITO";
  char alexa_name[32];
  uint32_t openTime = 10;
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
        
        // loop
        void loop();
};

#endif