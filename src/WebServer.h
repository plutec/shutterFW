#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include "Configuration.h"

void handleRoot();
void handlePlain();
void handleFormMQTT();
void handleNotFound();

class WebServer {
    private:
        //Configuration *config;
    public:
        WebServer() {};
        WebServer(Configuration *config);
        void loop();
};
