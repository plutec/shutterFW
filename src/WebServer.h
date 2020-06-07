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
#include "config.h"

void handleRoot();
void handleRestart();
void handlePlain();
void handleForm();
void handleHAConfig();
void handleNotFound();

class WebServer {
    private:
        //Configuration *config;
    public:
        WebServer() {};
        WebServer(Configuration *config);
        void loop();
};
