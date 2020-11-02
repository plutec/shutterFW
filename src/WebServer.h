/*
  WebServer.h - Webserver to control ShutterFW
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include "Configuration.h"
#include "Espalexa.h"
#include "config.h"

void handleRoot();
void handleRestart();
void handlePlain();
void handleForm();
void handleVirtualUp();
void handleVirtualDown();
void handleVirtualStop();
void handleStatus();
void handleHAConfig();
void handleNotFound();

class WebServer {
    private:
        //Configuration *config;
        int8_t *virtual_button;
    public:
        WebServer() {};
        WebServer(Configuration *config, int8_t *virtual_button, EspalexaDevice *alexa);
        void loop();
};
