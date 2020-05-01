#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include "Espalexa.h"

#include "config.h"  // Sustituir con datos de vuestra red
#include "WiFi_Utils.hpp"
//#include <WiFiUdp.h> //TODO Borrar, ya ha hecho su trabajo
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
//#include "HomeAssistant.h"

#define PAYLOAD_STOP "BLINDSTOP"
#define PAYLOAD_OPEN "BLINDOPEN"
#define PAYLOAD_CLOSE "BLINDCLOSE"
#define HOMEASSISTANT_SUPPORT


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

void mqtt_callback(char* topic, byte* payload, unsigned int length);

#ifdef BW_SS4
uint8 current_position = 100; //TODO. At startup it takes 100 position (totally open). It can update later from retained message in MQTT or from Flash memory
unsigned long milliseconds_per_percent = TIMEOPEN*10; // Time take to open or close 1 percent (in milliseconds).
#endif

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
//HomeAssistant ha;

EspalexaDevice* device;

//ALEXA CALLBACK
void percentBlind(uint8_t value);

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
  while (!mqtt.connected()) {
    if (mqtt.connect(hostname)) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Conectado de nuevo");
      mqtt.publish(DEBUG_TOPIC, hostname);
      #endif
      //mqtt.publish("tele/persiana_despacho/LWT", "Online", true); // Retained TODO mover a HA??
      //subscribe to:
      // cmnd/<device_name>/shutterposition
      mqtt.subscribe(subscribe_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup()
{
  #ifdef KINGART_Q4
  Serial.begin(19200);
  #endif
  Serial.begin(19200);

  #ifdef BW_SS4
   pinMode(RELAY1, OUTPUT);
   pinMode(RELAY2, OUTPUT);
   pinMode(LED, OUTPUT);
   digitalWrite(4, HIGH);
   //pinMode(BUTTON1, INPUT);
   //pinMode(BUTTON2, INPUT);
  #endif
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
  #ifdef BW_SS4
  device->setValue(current_position); //this allows you to e.g. update their state value at any time!
  #endif

  espalexa.begin();

  sprintf(subscribe_topic, "cmnd/%s/shutterposition", hostname);

  //Probamos el HomeAssistant class
  //ha = HomeAssistant(&mqtt);
  
}

void loop() 
{
  httpServer.handleClient();
  if (!mqtt.connected()) {
    reconnect_mqtt();
  }
  mqtt.loop();
  #ifdef KINGART_Q4
    if (Serial.available()>0) {
      String st = Serial.readStringUntil('\n');
      st[st.length()-1] = '\0';

      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, st.c_str());
      #endif
    }
  #endif
  espalexa.loop();
  /*ha.SendDiscovery();
  delay(500);*/
  /*digitalWrite(RELAY1, HIGH);
  delay(1000);
  digitalWrite(RELAY1, LOW);*/
}

void moveToPosition(uint8_t percent) {
  char str[80];
  #ifdef DEBUG
    sprintf(str, "Value going to change to percent %u", percent);
    mqtt.publish(DEBUG_TOPIC, str);
    delay(50);
  #endif
  #ifdef KINGART_Q4
    sprintf(str, "AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":%d\x1b", percent); //TODO Poner ese valor aleatorio, si fuera necesario...
    Serial.print(str);
    Serial.flush();
  #endif
  #ifdef BW_SS4
    //Estoy en la posicion X
    if (current_position == percent) {
      //Do nothing
    } else if (current_position > percent) { // We need to close
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Close...");
        delay(50);
      #endif
      //RELAY1 OFF, RELAY2 ON
      unsigned long diff = (current_position - percent)*milliseconds_per_percent;
      sprintf(str, "El tiempo es de... %lu", diff);
      mqtt.publish(DEBUG_TOPIC, str);
        delay(50);
      // First, assure RELAY1 is stopped
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, HIGH);
      delay(diff);
      digitalWrite(RELAY2, LOW);
      /*digitalWrite(LED, LOW);
      delayMicroseconds(diff);
      digitalWrite(LED, HIGH);*/
    } else if (current_position < percent) { // We need to open
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Open...");
        delay(50);
      #endif
      //RELAY1 ON, RELAY2 OFF
      unsigned long diff = (percent-current_position)*milliseconds_per_percent;
      sprintf(str, "El tiempo es de... %lu", diff);
      mqtt.publish(DEBUG_TOPIC, str);
      delay(50);
      // First, assure RELAY2 is stopped
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY1, HIGH);
      delay(diff);
      digitalWrite(RELAY1, LOW);
      /*digitalWrite(LED, LOW);
      delayMicroseconds(diff);
      digitalWrite(LED, HIGH);*/
    }
    current_position = percent;
  #endif

}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  mqtt.publish(DEBUG_TOPIC, "Callback msg...");
  delay(50);
  String messageTemp;
  for (unsigned int i=0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  messageTemp += '\0';

  #ifdef DEBUG
  mqtt.publish(DEBUG_TOPIC, "Callback msg:");
  mqtt.publish(DEBUG_TOPIC, messageTemp.c_str());
  delay(50);
  #endif
  int percent = atoi(messageTemp.c_str());
  moveToPosition(percent);
}



void percentBlind(uint8_t value) {
  #ifdef DEBUG
  mqtt.publish(DEBUG_TOPIC, "mueve el callback de alexa");
  #endif
  uint percent = 100-(value*100)/255; //100=open; 0=close
  //char cadenica[512];
  moveToPosition(percent);
}
