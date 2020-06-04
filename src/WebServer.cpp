
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

String generate_options(uint8_t selected) {
  String to_ret;
  for (uint8_t i=0;i<17;++i) {
    if (i<6 || (i>8 && i!=11)) { // Invalids: 6, 7, 8, 11
      
        to_ret += String("<option value=\"");
        to_ret += i;
        if (i==selected){
          to_ret += String("\" selected>GPIO");
        } else {
          to_ret += String("\">GPIO");
        }
        to_ret += i;
        to_ret += String("</option>");
    }
  }
  return to_ret;
}

void handleRoot() {
  String postForms = "<html>\
  <head>\
    <title>ShutterFW</title>\
    <style>\
      body { font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Helvetica, Arial, sans-serif; line-height: 1.6; color: #222; max-width: 40rem; padding: 2rem; margin: auto; background: #fafafa; } img { max-width: 100%; } a { color: #2ECC40; } h1, h2, strong { color: #111; }\
    </style>\
  </head>\
  <body>\
    <h1>"+String(configuration->getHostname())+"</h1>\
    <a href=\"/update\">Go to update page</a><br>\
    <a href=\"/restart\">Restart</a><br>\
    <h1>Node information</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      Hostname: <input type=\"text\" name=\"hostname\" value=\""+String(configuration->getHostname())+"\"><br>\
      Alexa name: <input type=\"text\" name=\"alexa_name\" value=\""+String(configuration->getAlexaName())+"\"><br>\
      Enable HomeAssistant integration: <input type=\"checkbox\" name=\"ha_enable\"><br>";      
      #ifdef OTHER_BOARD
      postForms += "Open/close time: <input type=\"text\" name=\"open_time\" value=\""+String(configuration->getOpenTime())+"\"> seconds<br>";
      #endif
      postForms += "Current position: "+String(configuration->getCurrentPosition())+"<br>\
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
    </form>";
    #ifdef OTHER_BOARD
    postForms += "<h1>Pinout (be careful!)</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      Relay UP: <select name=\"relay_up\">"+generate_options(configuration->getPinRelayUp())+"</select><br>\
      Relay DOWN: <select name=\"relay_down\">"+generate_options(configuration->getPinRelayDown())+"</select><br>\
      Button UP: <select name=\"button_up\">"+generate_options(configuration->getPinButtonUp())+"</select><br>\
      Button DOWN: <select name=\"button_down\">"+generate_options(configuration->getPinButtonDown())+"</select><br>\
      <input type=\"submit\" value=\"Change Pinout\">\
    </form><br>";
    #else
    postForms += "<br>Compiled for KingArt Curtain Q4<br>";
    #endif
    postForms += "Version:\
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
      if (httpServer.argName(i) == "relay_up") {
        configuration->setPinRelayUp(httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "relay_down") {
        configuration->setPinRelayDown(httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "button_up") {
        configuration->setPinButtonUp(httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "button_down") {
        configuration->setPinButtonDown(httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "ha_enable") {
        if (httpServer.arg(i) == "on") {
          configuration->setHomeAssistantEnabled(true);
        } else {
          configuration->setHomeAssistantEnabled(false);
        }
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