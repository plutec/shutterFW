/*
  HomeAssistant.h - A very simple controller for HomeAssistant. Made for ShutterFW and based on Tasmota reversing.
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/


#ifndef HomeAssistant_h
#define HomeAssistant_h
#include "PubSubClient.h"
#include "Configuration.h"
#include <ESP8266WiFi.h>


class HomeAssistant {
    private:
        PubSubClient* mqtt;
        Configuration* config;
        uint32_t chip_id;
    public:
        HomeAssistant();
        HomeAssistant(PubSubClient* mqtt, Configuration* config);
        void SendDiscovery();
        void SendState();
        void ManageSubscription();
        void SendUpdate(uint8_t percent, uint8_t target, int8_t direction);
};

#endif // HomeAssistant_h
