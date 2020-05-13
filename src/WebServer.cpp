
#include "WebServer.h"

ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

Configuration *configuration;

WebServer::WebServer(Configuration *config) {
  configuration = config;

  //Pruebas webserver
  httpServer.on("/", handleRoot);

  httpServer.on("/restart", handleRestart);

  httpServer.on("/postplain", handlePlain);

  httpServer.on("/p", handleFormMQTT);

  httpServer.onNotFound(handleNotFound);

  httpServer.begin();
  //Fin pruebas webserver

  // OTA Server
  httpUpdater.setup(&httpServer);
  //httpServer.begin();
}
void WebServer::loop() {
    httpServer.handleClient();
}

void handleRoot() {
  String postForms = "<html>\
  <head>\
    <title>ShutterFW</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <a href=\"/update\">Go to update page</a><br>\
    <a href=\"/restart\">Restart</a><br>\
    <h1>Node information</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      Hostname: <input type=\"text\" name=\"hostname\" value=\""+String(configuration->getHostname())+"\"><br>\
      Alexa name: <input type=\"text\" name=\"alexa_name\" value=\""+String(configuration->getAlexaName())+"\"><br>\
      Open/close time: <input type=\"text\" name=\"open_time\" value=\""+String(configuration->getOpenTime())+"\"> seconds <br>\
      <input type=\"submit\" value=\"Change general information\">\
    </form>\
    <h1>MQTT information</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      Server: <input type=\"text\" name=\"server\" value=\""+String(configuration->getMQTTServer())+"\"><br>\
      Port: <input type=\"text\" name=\"port\" value=\""+String(configuration->getMQTTPort())+"\"><br>\
      User: <input type=\"text\" name=\"user\" value=\""+String(configuration->getMQTTUser())+"\"><br>\
      Pass: <input type=\"text\" name=\"pass\" value=\""+String(configuration->getMQTTPassword())+"\"><br>\
      <input type=\"submit\" value=\"Change MQTT Information\">\
    </form>\
    <h1>WiFi information</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      SSID: <input type=\"text\" name=\"wifi_ssid\" value=\""+String(configuration->getWifiSsid())+"\"><br>\
      WiFi Password: <input type=\"password\" name=\"wifi_pass\" value=\"\"><br>\
      <input type=\"submit\" value=\"Change Wifi information\">\
    </form>\
  </body>\
</html>";

  httpServer.send(200, "text/html", postForms);
}

void handleRestart() {
  httpServer.send(200, "text/plain", "<META http-equiv=\"refresh\" content=\"15;URL=/\">Restarting...\n" + httpServer.arg("plain"));
  ESP.restart();
}

void handlePlain() {
  if (httpServer.method() != HTTP_POST) {
    //digitalWrite(led, 1);
    httpServer.send(405, "text/plain", "Method Not Allowed");
    //digitalWrite(led, 0);
  } else {
    //digitalWrite(led, 1);
    httpServer.send(200, "text/plain", "POST body was:\n" + httpServer.arg("plain"));
    //digitalWrite(led, 0);
  }
}

void handleFormMQTT() {
  if (httpServer.method() != HTTP_POST) {
    httpServer.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < httpServer.args(); i++) {
      message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
      if (httpServer.argName(i) == "server") {
        configuration->setMQTTServer(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "user") {
        configuration->setMQTTUser(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "password") {
        configuration->setMQTTPassword(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "port") {
        configuration->setMQTTPort(httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "wifi_ssid") {
        configuration->setWifiSsid(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "wifi_pass") {
        configuration->setWifiPass(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "hostname") {
        configuration->setHostname(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "alexa_name") {
        configuration->setAlexaName(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "open_time") {
        configuration->setOpenTime(httpServer.arg(i).toInt());
      }
    }
    httpServer.send(200, "text/plain", message);
  }
}

void handleNotFound() {
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
  //digitalWrite(led, 0);
}