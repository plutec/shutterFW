#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include "Espalexa.h"

#include "config.h"  // Sustituir con datos de vuestra red
#include "WiFi_Utils.hpp"
//#include <WiFiUdp.h> //TODO Borrar, ya ha hecho su trabajo
#include <ESP8266HTTPUpdateServer.h>
//#include <ESP8266WebServer.h>
#include "HomeAssistant.h"
#include "WebServer.h"
#include "Configuration.h"

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

void mqtt_callback(char* topic, byte* payload, unsigned int length);

//ALEXA CALLBACK
void percentBlind(uint8_t value);

#if defined(OTHER_BOARD)
//uint8 current_position = 80; //TODO. At startup it takes 100 position (totally open). It can update later from retained message in MQTT or from Flash memory
unsigned long milliseconds_per_percent;
bool state_btn1 = false;
bool state_btn2 = false;
#endif
bool netConnection = false;


/*
- Los botones activan la MCU y mandan el activo/inactivo a los relés
- El ESP debe mandar info a la MCU para activar y desactivar botones y relés
*/

//TODO
/*
- Cuando la MCU dice: AT+UPDATE="switch":"on","setclose":51   Hay que coger el valor y si ha cambiado, actualizarlo en HA mandado al topic.

*/
WiFiUDP Udp;
Configuration config;
WebServer webserver;
WiFiClient espClient;
PubSubClient mqtt(espClient);
//ESP8266WebServer httpServer(8080);
//ESP8266HTTPUpdateServer httpUpdater;
Espalexa espalexa;
#ifdef HOMEASSISTANT_SUPPORT
HomeAssistant ha;
#endif

EspalexaDevice* device;

char subscribe_topic[64];
long timming;
unsigned long last_timming, first_timming;


void reconnect_mqtt() {
  //uint8_t count = 5;
  if (!mqtt.connected()) {
    if (mqtt.connect(config.getHostname())) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Conectado de nuevo");
      mqtt.publish(DEBUG_TOPIC, config.getHostname());
      #endif
      //subscribe to:
      // cmnd/<device_name>/shutterposition
      mqtt.subscribe(subscribe_topic);
    } else {
      // Wait 5 seconds before retrying
      delay(1000);
    }
    //count--;
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

void networkManagement() {
  if (!ConnectWiFi_STA(config.getWifiSsid(), config.getWifiPass())) {
    ConnectWiFi_AP();
    netConnection = false;
  } else {
    netConnection = true;
  }
}

void pinConfiguration() {
  #if defined(OTHER_BOARD)
  pinMode(config.getPinRelayUp(), OUTPUT);
  pinMode(config.getPinRelayDown(), OUTPUT);
  //pinMode(LED, OUTPUT);
  //digitalWrite(LED, LOW);
  pinMode(config.getPinButtonUp(), INPUT);
  pinMode(config.getPinButtonDown(), INPUT);
  //attachInterrupt(digitalPinToInterrupt(BUTTON1), button_interrupt, RISING);
  #endif
}

void alexaConfiguration() {
  device = new EspalexaDevice(config.getAlexaName(), percentBlind);
  espalexa.addDevice(device); //and then add them
  #if defined(OTHER_BOARD)
  device->setPercent(config.getCurrentPosition()); //this allows you to e.g. update their state value at any time!
  #endif
}
void setup()
{
  
  #ifdef KINGART_Q4
  Serial.begin(19200);
  delay(1000);
  Serial.println("AT+UPDATE=\"sequence\":\"1572542635565\",\"switch\":\"pause\"\x1b"); // \x1b
  Serial.flush();
  #else
  Serial.begin(115200);
  #endif
 
  config.begin();
  #if defined(OTHER_BOARD)
    milliseconds_per_percent = config.getOpenTime()*10; // Time take to open or close 1 percent (in milliseconds).
  #endif
 
  pinConfiguration();
  /*
  if (!ConnectWiFi_STA(config.getWifiSsid(), config.getWifiPass(), !use_dhcp)) {
    ConnectWiFi_AP();
    netConnection = false;
  } else {SONOFF_DUAL_R2
    netConnection = true;
  }*/
  networkManagement();
  
  // MQTT
  if (netConnection) {
    mqtt.setServer(config.getMQTTServer(), config.getMQTTPort());
    mqtt.setCallback(mqtt_callback);
  
    if (!mqtt.connected()) {
      reconnect_mqtt();
    }
  }

  // Webserver stuff
  webserver = WebServer(&config);

  // Alexa stuff
  if (netConnection) {
    alexaConfiguration();
    espalexa.begin();
  }

  // Other stuff
  sprintf(subscribe_topic, "cmnd/%s/shutterposition", config.getHostname());
  Serial.println("Pasa subscribe topic");
  Serial.println(subscribe_topic);
 
}

#if defined(OTHER_BOARD)
/**
 * moveOrStop: true (move), false (stop)
 * relay: Pin to put HIGH (move true), or LOW (move false)
 */
// In kingart it is managed with MCU, ESP is not in charge of this.
void move(bool moveOrStop, uint8_t relay) {
  if (moveOrStop) {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
}

#endif

void clickManagement() {
  #ifdef KINGART_Q4
    if (Serial.available()>0) {
      String st = Serial.readStringUntil('\n');
      st[st.length()-1] = '\0';

      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, st.c_str());
      #endif
      int8_t pos = st.lastIndexOf("\"setclose\":");
      if (pos != -1) { 
        pos += 11; // len of "setclose":

        String num;// = "";
        while(isdigit(st[pos])) {
          num += st[pos];
          ++pos;
        }
        if (config.getCurrentPosition() != num.toInt()) {
          config.setCurrentPosition(num.toInt());
          device->setPercent(100-num.toInt());
        }
      }
      
    }
  #endif
  #if defined(OTHER_BOARD)
  // Button 1
  //bool pinValue = digitalRead(BUTTON1);

  if (digitalRead(config.getPinButtonUp()) == LOW) { // Remind this is active at LOW
    //Esto es para subir, tenemos la posición actual en "current_position". Sabemos que hasta el 100% le queda 100-current_position (pongamos 20%).
    //En subir ese 20% tarda 20*milliseconds_per_percent. Pongo un tope de estos millis y retrocemos hasta que llegue a 0.
    if (state_btn1 == false && config.getCurrentPosition() < 100) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Activa relé 1");
      Serial.println("Activa rele1");
      delay(100);
      #endif
      move(true, config.getPinRelayUp());
      state_btn1 = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = (100-config.getCurrentPosition())*milliseconds_per_percent; //Time to reach 100%
      last_timming = millis(); 
    } else if (config.getCurrentPosition() == 100){ //Está ya activo y ha llegado al final
      //Do nothing
    } else { 
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ELSE");
        Serial.println("Rele1 else");
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
        move(false, config.getPinRelayUp());
        state_btn1 = false;
        config.setCurrentPosition(100);
        device->setPercent(100);
        #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ha llegado al final, tenemos que parar!");
        Serial.println("Rele1 ha llegado al final, paramos");
        //delay(100);
        #endif
      }
    }
  } else {
    if (state_btn1 == true) { //El usuario ha desactivado el relé
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 1");
      Serial.println("Desactiva relé 1");
      delay(100);
      #endif
      move(false, config.getPinRelayUp());
      state_btn1 = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      uint8_t new_position = config.getCurrentPosition()+(int)percent_calcula;
      if (new_position > 100) {
        new_position = 100;
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);
    }
  }
  // Button 2
  if (digitalRead(config.getPinButtonDown()) == LOW) { // Remind this is active at LOW
    if (state_btn2 == false && config.getCurrentPosition() > 0) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Activa relé 2");
      Serial.println("Activa rele2");
      delay(100);
      #endif
      move(true, config.getPinRelayDown());
      state_btn2 = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = config.getCurrentPosition()*milliseconds_per_percent; //Time to reach 0%
      last_timming = millis(); 
    } else if (config.getCurrentPosition() == 0){ 
      //Do nothing
    } else { //Está ya activo
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, config.getPinRelayDown());
        state_btn2 = false;
        config.setCurrentPosition(0);
        device->setPercent(0);
      }
    }
  } else {
    if (state_btn2 == true) { //El usuario ha desactivado el relé
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 2");
      Serial.println("Desactiva rele2");
      delay(100);
      #endif
      move(false, config.getPinRelayDown());
      state_btn2 = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      int8_t new_position = config.getCurrentPosition()-(int)percent_calcula;
      if (new_position < 0) {
        new_position = 0;
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);
    }
  }
  //debounce delay
  unsigned long Milliseconds = millis()+50;
  while (Milliseconds > millis()) ;
  #endif
}

int firsttime = 1;

void loop() 
{
  webserver.loop();

  // MQTT
  if (netConnection) {
    if (!mqtt.connected()) {
      reconnect_mqtt();
    }
    mqtt.loop();
  }

  // Click management. Also for KingArt Serial communication (feedback from physical buttons)
  if (config.isSetPinouts()) {
    clickManagement();
  }

  // Alexa
  if (netConnection) {
    espalexa.loop();
  }

  // Save configuration data if necessary
  config.loop();

  // HomeAssistant stuff
  #if defined(HOMEASSISTANT_SUPPORT)
  if (netConnection && firsttime==1) {
    delay(1000);
    mqtt.publish("persiana/debug", "hay conexine");
    ha = HomeAssistant(&mqtt, &config);
    ha.SendDiscovery();
    firsttime=0;
  }
  #endif
  delay(50);
}

void moveToPosition(uint8_t percent, uint8_t alexa_value) {
  #if defined(DEBUG) || defined(KINGART_Q4)
  char str[90];
  #endif
  //uint8_t percent_alexa = 100-percent;
  //device->setPercent(percent_alexa);
  device->setValue(alexa_value);
  espalexa.loop();
  #ifdef DEBUG
    sprintf(str, "Value going to change to percent %u; alexa_value = %u", percent, alexa_value);
    mqtt.publish(DEBUG_TOPIC, str);
    delay(50);
  #endif
  #ifdef KINGART_Q4
    percent = 100-percent;
    sprintf(str, "AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":%d\x1b", percent); //TODO Poner valor de secuencia aleatorio, si fuera necesario...
    Serial.print(str);
    Serial.flush();
  #endif
  #if defined(OTHER_BOARD)
    //percent = 100-percent;
    if (config.getCurrentPosition() == percent) { //In the requested position
      //Do nothing
    } else if (config.getCurrentPosition() > percent) { // We need to close
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Close...");
        delay(50);
      #endif
      //RELAY1 OFF, RELAY2 ON
      unsigned long diff = (config.getCurrentPosition() - percent)*milliseconds_per_percent;
      //sprintf(str, "El tiempo es de... %lu", diff);
      //mqtt.publish(DEBUG_TOPIC, str);
        delay(50);
      // First, assure RELAY1 is stopped
      digitalWrite(config.getPinRelayUp(), LOW);
      digitalWrite(config.getPinRelayDown(), HIGH);
      // This block of code is to prevent the error in alexa like: "Device does not response"
      uint8_t ent = diff/500;
      uint8_t dec = diff%500;
      for (uint8_t i=0;i<ent;++i) {
        delay(500);
        espalexa.loop();
      }
      delay(dec);
      espalexa.loop();
      //delay(diff);
      // End of the block of alexa problem
      digitalWrite(config.getPinRelayDown(), LOW);
      /*digitalWrite(LED, LOW);
      delayMicroseconds(diff);
      digitalWrite(LED, HIGH);*/
    } else if (config.getCurrentPosition() < percent) { // We need to open
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Open...");
        delay(50);
      #endif
      //RELAY1 ON, RELAY2 OFF
      unsigned long diff = (percent-config.getCurrentPosition())*milliseconds_per_percent;
      #ifdef DEBUG
      sprintf(str, "El tiempo es de... %lu", diff);
      mqtt.publish(DEBUG_TOPIC, str);
      delay(50);
      #endif
      // First, assure RELAY2 is stopped
      digitalWrite(config.getPinRelayDown(), LOW);
      digitalWrite(config.getPinRelayUp(), HIGH);
      // This block of code is to prevent the error in alexa like: "Device does not response"
      uint8_t ent = diff/1000;
      uint8_t dec = diff%1000;
      for (uint8_t i=0;i<ent;++i) {
        delay(1000);
        espalexa.loop();
      }
      delay(dec);
      espalexa.loop();
      //delay(diff);
      // End of the block of alexa problem
      digitalWrite(config.getPinRelayUp(), LOW);
    }
    config.setCurrentPosition(percent);
    //device->setPercent(percent);
  #endif

}

// MQTT callback (when receive a msg in the subscribed topic)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
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
  uint8_t percent = atoi(messageTemp.c_str());
  uint8_t alexa_value = (percent * 255)/100;
  moveToPosition(percent, alexa_value);
}

// Alexa callback
void percentBlind(uint8_t value) {
  #ifdef DEBUG
  char str[80];
  sprintf(str, "Mueve el callback de alexa valor %u", value);
  mqtt.publish(DEBUG_TOPIC, str);
  #endif
  //0 para alexa es 0 para mi, 255 para alexa es 100 para mi
  uint8_t percent = (value*100)/255; //100=open; 0=close

  moveToPosition(percent, value);
}
