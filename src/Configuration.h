
#include <EEPROM.h>

#ifndef Configuration_h
#define Configuration_h

struct storage_struct{ 
  bool new_values = false;
  int current_position = 100;
  // WiFi
  char wifiSsid[32];
  char wifiPass[64];
  // MQTT
  bool mqttEnabled = false;
  char mqttServer[64];// = "192.168.0.50";
  int mqttPort = 1883;  // Default 1883
  char mqttUser[64];// = NULL;  // In case no user use NULL
  char mqttPassword[64];// = NULL; // In case no password use NULL
};


class Configuration {
    private:
        void save_eeprom_data();
        storage_struct storage;
    public:
        Configuration();
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
        // Position
        void setCurrentPosition(int current_position);
        int getCurrentPosition();
        // WiFi
        void setWifiSsid(const char *ssid);
        void setWifiPass(const char *pass);
        char* getWifiSsid();
        char* getWifiPass();

        // loop
        void loop();
};

#endif