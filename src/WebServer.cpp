/*
  WebServer.cpp - Webserver to control ShutterFW
  Antonio Sánchez <asanchez@plutec.net>
  https://plutec.net
  https://github.com/plutec
*/

#include "WebServer.h"

ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

Configuration *configuration;
int8_t *virtual_button_web;
EspalexaDevice *device_alexa;

WebServer::WebServer(Configuration *config, int8_t *virtual_button, EspalexaDevice *alexa) {
  configuration = config;
  //Serial.print("Dentro");
  //Serial.println(*virtual_button);
  virtual_button_web = virtual_button;
  device_alexa = alexa;
  //Pruebas webserver
  httpServer.on("/", handleRoot);

  httpServer.on("/restart", handleRestart);

  httpServer.on("/up", handleVirtualUp);

  httpServer.on("/down", handleVirtualDown);

  httpServer.on("/stop", handleVirtualStop);

  httpServer.on("/stat", handleStatus);

  httpServer.on("/postplain", handlePlain);

  httpServer.on("/p", handleForm);

  httpServer.on("/haconfig",handleHAConfig);

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
    <a href=\"/haconfig\">See HomeAssistant configuration</a><br>\
    <h1>Control</h1><br>\
    Current position: <div id=\"cp\">"+String(configuration->getCurrentPosition())+"</div>\
    <button id=\"u\">Up</button><br>\
    <button id=\"s\">Stop</button><br>\
    <button id=\"d\">Down</button><br>\
    <h1>Node information</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/p\">\
      Hostname: <input type=\"text\" name=\"hostname\" value=\""+String(configuration->getHostname())+"\"><br>\
      Alexa name: <input type=\"text\" name=\"alexa_name\" value=\""+String(configuration->getAlexaName())+"\"><br>\
      <input type=\"hidden\" name=\"ha_enable\" value=\"off\">\
      Enable HomeAssistant integration: <input type=\"checkbox\" name=\"ha_enable\" "+String(configuration->homeAssistantEnabledChecked())+"><br>\
      Topic for HomeAssistant: <input type=\"text\" name=\"mqtt_topic\" value=\""+String(configuration->getMqttTopic())+"\"><br>";
      #ifdef OTHER_BOARD
      postForms += "Open/close time: <input type=\"text\" name=\"open_time\" value=\""+String(configuration->getOpenTime())+"\"> seconds<br>";
      #else
      postForms += "Current position: "+String(configuration->getCurrentPositionKA())+"<br>";
      #endif
      postForms += "<input type=\"submit\" value=\"Change general information\">\
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
    postForms += "<h1>Calibration</h1><br/>\
    <form method=\"post\" action=\"/p\">\
    <input type=\"text\" name=\"c_0\" size=\"2\" value=\""+String(configuration->getCalibration(0))+"\">\
    <input type=\"text\" name=\"c_1\" size=\"2\" value=\""+String(configuration->getCalibration(1))+"\">\
    <input type=\"text\" name=\"c_2\" size=\"2\" value=\""+String(configuration->getCalibration(2))+"\">\
    <input type=\"text\" name=\"c_3\" size=\"2\" value=\""+String(configuration->getCalibration(3))+"\"><br/>\
    30%&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;50%&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;70%&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;90%<br/>\
    <input type=\"hidden\" name=\"calibration_enable\" value=\"off\">\
    Enable calibration: <input type=\"checkbox\" name=\"calibration_enable\" "+String(configuration->calibrationEnabledChecked())+"><br>\
    <input type=\"submit\" value=\"Save calibration\">\
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
    postForms += "Version: "+String(VERSION_FW)+"\
  </body>\
  <script type=\"text/javascript\">\
  document.getElementById('u').onclick=function(){\
    var request = new XMLHttpRequest();\
    request.open('GET', '/up', true);\
    request.send();\
    myVar = setInterval(stat, 500);\
  };\
  document.getElementById('s').onclick=function(){\
    var request = new XMLHttpRequest();\
    request.open('GET', '/stop', true);\
    request.send();\
    clearInterval(myVar);\
  };\
  document.getElementById('d').onclick=function(){\
    var request = new XMLHttpRequest();\
    request.open('GET', '/down', true);\
    request.send();\
    myVar = setInterval(stat, 500);\
  };\
  function stat(){\
    console.log(\"Stat\");\
    var request = new XMLHttpRequest();\
    request.onreadystatechange=function(){if(request.readyState == 4 && request.status == 200){document.getElementById(\"cp\").innerHTML = request.responseText;}};\
    request.open('GET', '/stat', true);\
    request.send();\
  };\
  </script>\
</html>";

  httpServer.send(200, "text/html", postForms);
}

void handleRestart() {
  httpServer.send(200, "text/html", "<META http-equiv=\"refresh\" content=\"15;URL=/\">Restarting...\n");
  ESP.restart();
}

void handleHAConfig() {
      String topic = String(configuration->getMqttTopic());

    String config="\
cover:\n\
  - platform: mqtt\n\
    name: \"Persiana Invitados\"\n\
    availability_topic: \"tele/"+topic+"/LWT\"\n\
    payload_available: \"Online\"\n\
    payload_not_available: \"Offline\"\n\
    position_topic: \"stat/"+topic+"/RESULT\"\n\
    value_template: >\n\
      {% if ('Shutter1' in value_json) and ('Position' in value_json.Shutter1) %}\n\
        {{ value_json.Shutter1.Position }}\n\
      {% else %}\n\
        {% if is_state('cover."+topic+"', 'unknown') %}\n\
          50\n\
        {% else %}\n\
          {{ state_attr('cover."+topic+"','current_position') }}\n\
        {% endif %}\n\
      {% endif %}\n\
    position_open: 100\n\
    position_closed: 0\n\
    set_position_topic: \"cmnd/"+topic+"/ShutterPosition1\"\n\
    command_topic: \"cmnd/"+topic+"/Backlog\"\n\
    payload_open: \"ShutterOpen1\"\n\
    payload_close: \"ShutterClose1\"\n\
    payload_stop: \"ShutterStop1\"\n\
    retain: false\n\
    optimistic: false\n\
    qos: 1";
httpServer.send(200, "text/plain", config);
}

void handleVirtualUp() {
  *virtual_button_web = 1;
  httpServer.send(200, "text/plain", "UP");
}

void handleVirtualDown() {
  *virtual_button_web = -1;
  httpServer.send(200, "text/plain", "DOWN");
}
void handleVirtualStop() {
  *virtual_button_web = 2;
  httpServer.send(200, "text/plain", "STOP");
}

void handleStatus() {
  char buffer [4];
  //Serial.println(device_alexa->getPercent());
  itoa(device_alexa->getPercent(), buffer, 10);
  //String toRet = "a " + 
  httpServer.send(200, "text/plain", buffer);
}

void handlePlain() {
  if (httpServer.method() != HTTP_POST) {
    httpServer.send(405, "text/plain", "Method Not Allowed");
  } else {
    httpServer.send(200, "text/plain", "POST body was:\n" + httpServer.arg("plain"));
  }
}

void handleForm() {
  if (httpServer.method() != HTTP_POST) {
    httpServer.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "";
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
      if (httpServer.argName(i) == "mqtt_topic") {
        configuration->setMqttTopic(httpServer.arg(i).c_str());
      }
      if (httpServer.argName(i) == "c_0") {
        configuration->setCalibration(0, httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "c_1") {
        configuration->setCalibration(1, httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "c_2") {
        configuration->setCalibration(2, httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "c_3") {
        configuration->setCalibration(3, httpServer.arg(i).toInt());
      }
      if (httpServer.argName(i) == "calibration_enable") {
        if (httpServer.arg(i) == "on") {
          configuration->setCalibrationEnabled(true);
        } else {
          configuration->setCalibrationEnabled(false);
        }
      }
    }
    #ifdef DEBUG
    httpServer.send(200, "text/plain", message);
    #else
    httpServer.send(200, "text/html", "<META http-equiv=\"refresh\" content=\"0;URL=/\">");
    #endif
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