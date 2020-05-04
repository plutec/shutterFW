#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
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


#define DEBUG //Comment to remove debug via MQTT
#define DEBUG_TOPIC "persiana/debug"



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

struct storage{ 
    uint percent = 0;
    //char str[20] = "";
} storage_struct;

void save_eeprom_data() {
  uint addr = 0;
  EEPROM.begin(sizeof(struct storage)); //bytes
  EEPROM.put(addr,storage_struct);
}

uint8_t load_eeprom_data() {
  //Return only the percent of the shutter
  uint addr = 0;
  EEPROM.begin(128); //bytes
  EEPROM.get(addr, storage_struct);
}



void mqtt_callback(char* topic, byte* payload, unsigned int length);

#if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
uint8 current_position = 80; //TODO. At startup it takes 100 position (totally open). It can update later from retained message in MQTT or from Flash memory
unsigned long milliseconds_per_percent = TIMEOPEN*10; // Time take to open or close 1 percent (in milliseconds).
bool state_btn1 = false;
bool state_btn2 = false;
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
- Quitar pulsacion inicial con un pause (KingArt) -> TODO
- OTA -> OK (puerto 8080/update)
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
/*
void button_interrupt() {
  #ifdef DEBUG
  mqtt.publish(DEBUG_TOPIC, "Recibo interrupcion btn1");
  delay(100);
  #endif
  //debounce delay
  int Milliseconds = millis()+50;
  while (Milliseconds > millis()) ;

}
*/

void setup()
{
  #ifdef KINGART_Q4
  Serial.begin(19200);
  #endif
  Serial.begin(19200);

  #if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
   pinMode(RELAY1, OUTPUT);
   pinMode(RELAY2, OUTPUT);
   //pinMode(LED, OUTPUT);
   //digitalWrite(LED, LOW);
   pinMode(BUTTON1, INPUT);
   pinMode(BUTTON2, INPUT);
   //attachInterrupt(digitalPinToInterrupt(BUTTON1), button_interrupt, RISING);
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
  #if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
  device->setValue(current_position); //this allows you to e.g. update their state value at any time!
  #endif

  espalexa.begin();

  sprintf(subscribe_topic, "cmnd/%s/shutterposition", hostname);

/*  storage_struct.percent = 85;
  save_eeprom_data();
  storage_struct.percent = 1;
*/
  //Probamos el HomeAssistant class
  //ha = HomeAssistant(&mqtt);
  
}

#if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
/**
 * moveOrStop: true (move), false (stop)
 * relay: Pin to put HIGH (move true), or LOW (move false)
 */
//In kingart it is managed with MCU, ESP is out of this.
void move(bool moveOrStop, uint8_t relay) {
  if (moveOrStop) {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
}
#endif
long timming;
unsigned long last_timming, first_timming;


void loop() 
{
  httpServer.handleClient();
  if (!mqtt.connected()) {
    reconnect_mqtt();
  }
  mqtt.loop();

  /*
  load_eeprom_data();
  char pa[12];
  sprintf(pa, "percent: %d",storage_struct.percent);
  mqtt.publish(DEBUG_TOPIC, pa);
  delay(100);
  */
  #ifdef KINGART_Q4
    if (Serial.available()>0) {
      String st = Serial.readStringUntil('\n');
      st[st.length()-1] = '\0';

      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, st.c_str());
      #endif
    }
  #endif
  //TODO Falta contar el tiempo que está activado cada relé, para saber en qué posición se encuentra la persiana
  #if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
  // Button 1
  //bool pinValue = digitalRead(BUTTON1);

  if (digitalRead(BUTTON1) == LOW) { // Remind this is active at LOW
    //Esto es para subir, tenemos la posición actual en "current_position". Sabemos que hasta el 100% le queda 100-current_position (pongamos 20%).
    //En subir ese 20% tarda 20*milliseconds_per_percent. Pongo un tope de estos millis y retrocemos hasta que llegue a 0.
    if (state_btn1 == false && current_position < 100) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Activa relé 1");
      delay(100);
      #endif
      move(true, RELAY1);
      state_btn1 = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = (100-current_position)*milliseconds_per_percent; //Time to reach 100%
      last_timming = millis(); 
    } else if (current_position == 100){ //Está ya activo
      //Do nothing
    } else { 
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ELSE");
        //delay(100);
      #endif
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      #ifdef DEBUG
      char str_debug[256];
      sprintf(str_debug, "timming %lu last_timming %lu \n", timming, last_timming );
      mqtt.publish(DEBUG_TOPIC, str_debug);
      delay(100);
      #endif
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, RELAY1);
        state_btn1 = false;
        current_position = 100;
        #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ha llegado al final, tenemos que parar!");
        //delay(100);
        #endif
      }
    }
  } else {
    if (state_btn1 == true) { //El usuario ha desactivado el relé
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 1");
      delay(100);
      #endif
      move(false, RELAY1);
      state_btn1 = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      current_position = current_position+(int)percent_calcula;
      if (100<current_position) {
        current_position = 100;
      }
    }
  }
  // Button 2
  if (digitalRead(BUTTON2) == LOW) { // Remind this is active at LOW
    if (state_btn2 == false && current_position > 0) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Activa relé 2");
      delay(100);
      #endif
      move(true, RELAY2);
      state_btn2 = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = current_position*milliseconds_per_percent; //Time to reach 0%
      last_timming = millis(); 
    } else if (current_position == 0){ 
      //Do nothing
    } else { //Está ya activo
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, RELAY2);
        state_btn2 = false;
        current_position = 0;
      }
    }
  } else {
    if (state_btn2 == true) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 2");
      delay(100);
      #endif
      move(false, RELAY2);
      state_btn2 = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      current_position = current_position-(int)percent_calcula;
      if (current_position<0) {
        current_position = 0;
      }
    }
  }
  //debounce delay
  int Milliseconds = millis()+50;
  while (Milliseconds > millis()) ;
  #endif

  // Alexa integration
  espalexa.loop();


  //Homeassistant integration (TODO)
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
    sprintf(str, "AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":%d\x1b", percent); //TODO Poner valor aleatorio, si fuera necesario...
    Serial.print(str);
    Serial.flush();
  #endif
  #if defined(BW_SS4) || defined(SONOFF_DUAL_R2)
    if (current_position == percent) { //In the requested position
      //Do nothing
    } else if (current_position > percent) { // We need to close
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Close...");
        delay(50);
      #endif
      //RELAY1 OFF, RELAY2 ON
      unsigned long diff = (current_position - percent)*milliseconds_per_percent;
      //sprintf(str, "El tiempo es de... %lu", diff);
      //mqtt.publish(DEBUG_TOPIC, str);
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
      #ifdef DEBUG
      sprintf(str, "El tiempo es de... %lu", diff);
      mqtt.publish(DEBUG_TOPIC, str);
      delay(50);
      #endif
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
