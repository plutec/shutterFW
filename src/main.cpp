#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include <Espalexa.h>

#include "config.h"  // Sustituir con datos de vuestra red
#include "WiFi_Utils.hpp"
//#include <WiFiUdp.h> //TODO Borrar, ya ha hecho su trabajo
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>

#define PAYLOAD_STOP "BLINDSTOP"
#define PAYLOAD_OPEN "BLINDOPEN"
#define PAYLOAD_CLOSE "BLINDCLOSE"
#define HOMEASSISTANT_SUPPORT 1
//MQTT INFO
/*
availability_topic: "tele/persiana_despacho/LWT" (Online/Offline)
position_topic: stat/persiana_despacho/SHUTTER1
   stat/luz_cocina/RESULT {"POWER":"ON"}
   stat/luz_cocina/POWER ON
   ---
   stat/luz_cocina/RESULT {"POWER":"OFF"}
   stat/luz_cocina/POWER OFF

set_position_topic: "cmnd/persiana_despacho/shutterposition" -> Cuando manda HA, en este caso en lugar de ON OFF será un numero
   cmnd/luz_cocina/POWER ON   
   cmnd/luz_cocina/POWER OFF

command_topic: "cmnd/persiana_despacho/backlog"
payload_open: "SHUTTEROPEN"
payload_close: "SHUTTERCLOSE"
payload_stop: "SHUTTERSTOP"
position_open: 100 -> Clear
position_closed: 0 -> Clear
*/


#define DEBUG //Comment to remove debug via MQTT
#define DEBUG_TOPIC "persiana/debug"

// void debug_udp(const char *msg); //OLD CODE
void mqtt_callback(char* topic, byte* payload, unsigned int length);

/*
- Los botones activan la MCU y mandan el activo/inactivo a los relés
- El ESP debe mandar info a la MCU para activar y desactivar botones y relés
*/

//TODO
/*
- Cuando la MCU dice: AT+UPDATE="switch":"on","setclose":51   Hay que coger el valor y si ha cambiado, actualizarlo en HA mandado al topic.

*/
WiFiUDP Udp;
WiFiClient espClient;
PubSubClient mqtt(espClient);
ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;
Espalexa espalexa;

EspalexaDevice* device;

//ALEXA CALLBACKS
//new callback type, contains device pointer
//void alphaChanged(EspalexaDevice* dev);
//void betaChanged(EspalexaDevice* dev);
void percentBlind(uint8_t value);
//you can now use one callback for multiple devices
//void deltaOrEpsilonChanged(EspalexaDevice* dev);
char subscribe_topic[64];

/*
PASOS
- MQTT para logging -> OK
- MQTT Para control -> Base hecha
- Quitar pulsacion inicial con un pause -> TODO
- OTA -> OK (puerto 8080)
- ACTIVAR y desactivar relés mandando comandos por MQTT
  - Leer comandos AT desde la MCU (puerto Serial) (Necesario para mandar estado a HA)
  - Mandar comandos AT a la MCU (puerto Serial) (Hecho, Serial.write(...))
- Coger pulsaciones de la MCU (Para ver el estado en el que esta)
- Pines Relés 
    - Open
    - Close

- Alexa para control -> Falta enlazar las callback
*/

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(hostname)) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Conectado de nuevo");
      #endif
      mqtt.publish("tele/persiana_despacho/LWT", "Online", true); // Retained
      delay(200);
      mqtt.publish("homeassistant/cover/persiana_despacho/config", json_homeassistant, true);  // Retained
  
      //subscribe to:
      // cmnd/persiana_despacho/shutterposition
      mqtt.subscribe(subscribe_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup()
{

  Serial.begin(19200);

  ConnectWiFi_STA(!use_dhcp);
  
  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqtt_callback);
  
  if (!mqtt.connected()) {
    reconnect_mqtt();
  }

  //OTA Server
  httpUpdater.setup(&httpServer);
  httpServer.begin();

  device = new EspalexaDevice(ALEXA_NAME, percentBlind); //you can also create the Device objects yourself like here
  espalexa.addDevice(device); //and then add them
  //device->setValue(100); //this allows you to e.g. update their state value at any time!

  espalexa.begin();

  sprintf(subscribe_topic, "cmnd/%s/shutterposition", hostname);

}

void loop() 
{

  httpServer.handleClient();
  if (!mqtt.connected()) {
    reconnect_mqtt();
  }
  mqtt.loop();
  
  if (Serial.available()>0) {
    String st = Serial.readStringUntil('\n');
    st[st.length()-1] = '\0';

    #ifdef DEBUG
    mqtt.publish(DEBUG_TOPIC, st.c_str());
    #endif
  } 
  espalexa.loop();
}

void moveToPosition(uint8_t percent) {
  char str[80];
  sprintf(str, "AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":%d\x1b", percent); //TODO Poner ese valor aleatorio, si fuera necesario...
  Serial.print(str);
  Serial.flush();
  #ifdef DEBUG
  sprintf(str, "Valor cambiado a porcentaje %u", percent);
  mqtt.publish(DEBUG_TOPIC, str);
  #endif
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  
  String messageTemp;
  for (unsigned int i=0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  messageTemp += '\0';

  #ifdef DEBUG
  mqtt.publish(DEBUG_TOPIC, "Callback msg:");
  mqtt.publish(DEBUG_TOPIC, messageTemp.c_str());
  #endif
  int percent = atoi(messageTemp.c_str());
  moveToPosition(percent);
  //Serial.print("AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":0");
  //Serial.flush();
  //Topic: persiana/action
  /*if (messageTemp.equals("UP")) {
    //Serial.write("AT+UPDATE=\"switch\":\"on\",\"setclose\":0\x1b");
    Serial.print("AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":0");
    Serial.flush();
    mqtt.publish(DEBUG_TOPIC, "Manda subir");
  } else if (messageTemp.equals("DW")) {
    //Serial.write("AT+UPDATE=\"switch\":\"off\",\"setclose\":100\x1b");
    //Serial.print("AT+UPDATE=\"sequence\":\"%d%03d\",\"setclose\":%d", (157253, millis()%1000, 100));
    Serial.print("AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":100");
    Serial.flush();
    mqtt.publish(DEBUG_TOPIC, "Manda bajar");
  }*/
}



void percentBlind(uint8_t value) {
  uint percent = 100-(value*100)/255; //100=open; 0=close
  //char cadenica[512];
  moveToPosition(percent);
}

/***** HOMEASSISTANT *****/
// https://www.home-assistant.io/docs/mqtt/discovery/

/*
Configuration topic: homeassistant/cover/despacho_giu/config
State topic: homeassistant/cover/despacho_giu/state
Command topic: homeassistant/cover/despacho_giu/set
*/

/**
 * Cuando abre: state_topic -> state_opening
 * Cuando termina de abrir: state_topic -> state_open
 * Cuando cierra: state_topic -> state_closing
 * Cuando termina de cerrar: state_topic -> state_close
 * Porcentajes: (position_topic)
 * 0 -> Closed
 * 100 -> Open
 * 
 * 
 * 
 * */

void create_json_ha() {
  StaticJsonDocument<200> doc;

  // StaticJsonObject allocates memory on the stack, it can be
  // replaced by DynamicJsonDocument which allocates in the heap.
  //
  // DynamicJsonDocument  doc(200);

  // Add values in the document
  //
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;
}
