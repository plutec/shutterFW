//#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include "Espalexa.h"

#include "config.h"  // Sustituir con datos de vuestra red
#include "WiFi_Utils.hpp"
#include <ESP8266HTTPUpdateServer.h>
//#include <ESP8266WebServer.h>
#include "HomeAssistant.h"
#include "WebServer.h"
#include "Configuration.h"


void mqtt_callback(char* topic, byte* payload, unsigned int length);

//ALEXA CALLBACK
void percentBlind(uint8_t value);

void moveToPosition(uint8_t percent, uint8_t alexa_value);

#if defined(OTHER_BOARD)
unsigned long milliseconds_per_percent;
bool using_buttons = false;
#endif
bool already_moving_up = false;
bool already_moving_down = false;
int8_t virtual_button = 0;
bool netConnection = false;


Configuration config;
WebServer webserver;
WiFiClient espClient;
PubSubClient mqtt(espClient);
Espalexa espalexa;
HomeAssistant ha;


EspalexaDevice* device;

char subscribe_topic[2][64];
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
      mqtt.subscribe(subscribe_topic[0]);
      mqtt.subscribe(subscribe_topic[1]);
    } else {
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

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
  Serial.println("AT+UPDATE=\"sequence\":\"1572542635565\",\"switch\":\"pause\"\x1b");
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
  sprintf(subscribe_topic[0], "cmnd/%s/Backlog", config.getHostname());
  sprintf(subscribe_topic[1], "cmnd/%s/ShutterPosition1", config.getHostname());
}

#if defined(OTHER_BOARD)
/**
 * moveOrStop: true (move), false (stop)
 * relay: Pin to put HIGH (move true), or LOW (move false)
 * Note: In kingart it is managed with MCU, ESP is not in charge of this.
 */
void move(bool moveOrStop, uint8_t relay) {
  if (moveOrStop) {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
}

#endif

/**
 * int8_t updownstop: up == 1, down == -1, stop == 0, continue_with_previous == 2
 */
void movementManager(int8_t updownstop) {
  #if defined(OTHER_BOARD)
  uint8_t current_percent;
  if (updownstop == 1) {//if (digitalRead(config.getPinButtonUp()) == LOW) { // Remind this is active at LOW
    //Esto es para subir, tenemos la posición actual en "current_position". Sabemos que hasta el 100% le queda 100-current_position (pongamos 20%).
    //En subir ese 20% tarda 20*milliseconds_per_percent. Pongo un tope de estos millis y retrocemos hasta que llegue a 0.
    if (already_moving_up == false && config.getCurrentPosition() < 100) { // It's the first loop after press buttonup
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Enable relayUp");
      Serial.println("Enable relayUp");
      delay(100);
      #endif
      move(true, config.getPinRelayUp());
      already_moving_up = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = (100-config.getCurrentPosition())*milliseconds_per_percent; //Time to reach 100%
      last_timming = millis();

    } else if (config.getCurrentPosition() == 100){ //Está ya activo y ha llegado al final
      //Do nothing, this conditional is required (or not...)
    } else { // Second and following loops after press buttonUP
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ELSE");
        Serial.println("Rele1 else");
        //delay(100);
      #endif
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      // HomeAssistant stuff
      if (config.homeAssistantEnabled()) {
        current_percent = config.getCurrentPosition()+(last_timming-first_timming)/milliseconds_per_percent;
        ha.SendUpdate(current_percent, 100, 1);
      }
    
      #ifdef DEBUG
      //char str_debug[256];
      //sprintf(str_debug, "timming %lu last_timming %lu \n", timming, last_timming );
      //mqtt.publish(DEBUG_TOPIC, str_debug);
      //delay(100);
      #endif
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, config.getPinRelayUp());
        already_moving_up = false;
        if (config.homeAssistantEnabled()) {
          ha.SendUpdate(100, 100, 0);
        }
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
    if (already_moving_up == true) { //El usuario ha desactivado el relé
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 1");
      Serial.println("Desactiva relé 1");
      delay(100);
      #endif
      move(false, config.getPinRelayUp());
      already_moving_up = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      uint8_t new_position = config.getCurrentPosition()+(int)percent_calcula;
      if (new_position > 100) {
        new_position = 100;
      }
      if (config.homeAssistantEnabled()) {
        ha.SendUpdate(new_position, new_position, 0);
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);
    }
  }
  // Button 2
  if (updownstop==-1) {//if (digitalRead(config.getPinButtonDown()) == LOW) { // Remind this is active at LOW
    if (already_moving_down == false && config.getCurrentPosition() > 0) {
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Activa relé 2");
      Serial.println("Activa rele2");
      delay(100);
      #endif
      move(true, config.getPinRelayDown());
      already_moving_down = true;
      //timming = millis(); //Enable timming
      first_timming = millis(); //Time of activation
      timming = config.getCurrentPosition()*milliseconds_per_percent; //Time to reach 0%
      last_timming = millis(); 
    } else if (config.getCurrentPosition() == 0){ 
      //Do nothing
    } else { //Está ya activo
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      // HomeAssistant stuff
      if (config.homeAssistantEnabled()) {
        current_percent = config.getCurrentPosition()-(last_timming-first_timming)/milliseconds_per_percent;
        ha.SendUpdate(current_percent, 0, -1);
      }
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, config.getPinRelayDown());
        already_moving_down = false;
        if (config.homeAssistantEnabled()) {
          ha.SendUpdate(0, 0, 0);
        }
        config.setCurrentPosition(0);
        device->setPercent(0);
      }
    }
  } else {
    if (already_moving_down == true) { //El usuario ha desactivado el relé
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "DESactiva relé 2");
      Serial.println("Desactiva rele2");
      delay(100);
      #endif
      move(false, config.getPinRelayDown());
      already_moving_down = false;
      timming = timming-(millis()-last_timming);
      unsigned long moving_time = millis() - first_timming;
      //Calculate percent
      unsigned long percent_calcula = moving_time/milliseconds_per_percent;
      int8_t new_position = config.getCurrentPosition()-(int)percent_calcula;
      if (new_position < 0) {
        new_position = 0;
      }
      if (config.homeAssistantEnabled()) {
        ha.SendUpdate(new_position, new_position, 0);
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);
    }
  }
  if (updownstop==0) {
    move(false, config.getPinRelayUp());
    move(false, config.getPinRelayDown());
    already_moving_down = false;
    already_moving_up = false;
  }
  #else // KingArt Q4
  if (updownstop == 1) {
    moveToPosition(100, 255);
  }
  if (updownstop == -1) {
    moveToPosition(0, 0);
  }
  if (updownstop == 0) {
    Serial.println("AT+UPDATE=\"sequence\":\"1572542635565\",\"switch\":\"pause\"\x1b");
    Serial.flush();
  }
  #endif
}

void clickManagement() {
  //uint8_t current_percent;
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
          if (config.homeAssistantEnabled()) {
            int8_t direction;
            int8_t target;
            if (num.toInt()>config.getCurrentPosition()) {
              direction=-1;
              target=0;
            } else {
              direction=1;
              target=100;
            }
            ha.SendUpdate(100-num.toInt(), target, direction);
          }
        }
        else if (config.homeAssistantEnabled()) {
          pos = st.lastIndexOf("\"pause\"");
          if (pos != -1) {
            ha.SendUpdate(100-num.toInt(), 100-num.toInt(), 0);
          }
        }
      }
    }
  #endif
  #if defined(OTHER_BOARD)
  // Button 1
  //bool pinValue = digitalRead(BUTTON1);

  if (digitalRead(config.getPinButtonUp()) == LOW) { // Remind this is active at LOW
    movementManager(1);
    //mqtt.publish(DEBUG_TOPIC, "Activa PARRIBA");
    //Serial.println("Activa rele2");
    //delay(100);
    using_buttons = true;
    //Esto es para subir, tenemos la posición actual en "current_position". Sabemos que hasta el 100% le queda 100-current_position (pongamos 20%).
    //En subir ese 20% tarda 20*milliseconds_per_percent. Pongo un tope de estos millis y retrocemos hasta que llegue a 0.
    /*if (state_btn1 == false && config.getCurrentPosition() < 100) { // It's the first loop after press buttonup
      #ifdef DEBUG
      mqtt.publish(DEBUG_TOPIC, "Enable relayUp");
      Serial.println("Enable relayUp");
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
    } else { // Second and following loops after press buttonUP
      #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ELSE");
        Serial.println("Rele1 else");
        //delay(100);
      #endif
      timming = timming-(millis()-last_timming);
      last_timming = millis();
      // HomeAssistant stuff
      if (config.homeAssistantEnabled()) {
        current_percent = config.getCurrentPosition()+(last_timming-first_timming)/milliseconds_per_percent;
        ha.SendUpdate(current_percent, 100, 1);
      }
    
      #ifdef DEBUG
      //char str_debug[256];
      //sprintf(str_debug, "timming %lu last_timming %lu \n", timming, last_timming );
      //mqtt.publish(DEBUG_TOPIC, str_debug);
      //delay(100);
      #endif
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, config.getPinRelayUp());
        state_btn1 = false;
        if (config.homeAssistantEnabled()) {
          ha.SendUpdate(100, 100, 0);
        }
        config.setCurrentPosition(100);
        device->setPercent(100);
        #ifdef DEBUG
        mqtt.publish(DEBUG_TOPIC, "Relé1 ha llegado al final, tenemos que parar!");
        Serial.println("Rele1 ha llegado al final, paramos");
        //delay(100);
        #endif
      }
    }
  } 
   else {
    if (already_moving_up == true) { //El usuario ha desactivado el relé y se estaba moviendo
      movementManager(0);
    
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
      if (config.homeAssistantEnabled()) {
        ha.SendUpdate(new_position, new_position, 0);
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);*/
    //}
  }

  // Button 2
  else if (digitalRead(config.getPinButtonDown()) == LOW) { // Remind this is active at LOW
    movementManager(-1);
    //mqtt.publish(DEBUG_TOPIC, "Activa PABAJO");
    //Serial.println("Activa rele2");
    //delay(100);
    using_buttons = true;
    /*if (state_btn2 == false && config.getCurrentPosition() > 0) {
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
      // HomeAssistant stuff
      if (config.homeAssistantEnabled()) {
        current_percent = config.getCurrentPosition()-(last_timming-first_timming)/milliseconds_per_percent;
        ha.SendUpdate(current_percent, 0, -1);
      }
      if (timming <= 0) { //Ha llegado al final, tenemos que parar!
        move(false, config.getPinRelayDown());
        state_btn2 = false;
        if (config.homeAssistantEnabled()) {
          ha.SendUpdate(0, 0, 0);
        }
        config.setCurrentPosition(0);
        device->setPercent(0);
      }
    }
  } else {
    
    if (already_moving_down == true) { //El usuario ha desactivado el relé y se estaba moviendo
      movementManager(0);
    
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
      if (config.homeAssistantEnabled()) {
        ha.SendUpdate(new_position, new_position, 0);
      }
      config.setCurrentPosition(new_position);
      device->setPercent(new_position);*/
    //}
  } /*else if (using_buttons) {
    using_buttons = false;
    movementManager(0);
  }*/
  
  else if (virtual_button == 1) { // Enabled the virtual button up
    movementManager(1);
  } else if (virtual_button == -1) {
    movementManager(-1);
  } else if (already_moving_up || already_moving_down) {
    movementManager(0);
  }
  //debounce delay
  unsigned long Milliseconds = millis()+50;
  while (Milliseconds > millis()) ;
  #else
  else if (virtual_button == 1) { // Enabled the virtual button up
    movementManager(1);
  } else if (virtual_button == -1) {
    movementManager(-1);
  } else if (virtual_button == 2) { //Stop
    virtual_button = 0;
    movementManager(0);
    
  }
  #endif
  
}

uint16_t loop_cnt = 0;
bool first_loop = true;
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
  #ifdef OTHER_BOARD
  if (config.isSetPinouts()) {
    clickManagement();
  }
  #else
  clickManagement();
  #endif
  // Alexa
  if (netConnection) {
    espalexa.loop();
  }

  // Save configuration data if necessary
  config.loop();

  // HomeAssistant stuff
  
  if (config.homeAssistantEnabled() && netConnection && loop_cnt==30 && first_loop) {
    //delay(1000);
    //mqtt.publish("persiana/debug", "hay conexine");
    ha = HomeAssistant(&mqtt, &config);
    ha.SendDiscovery();
    ha.SendState();
    first_loop=false; 
    
  }
  if (config.homeAssistantEnabled() && netConnection && loop_cnt == 0 && !first_loop) { //TODO Make to repeat each 5 minutes
    ha.SendState();
    //ntpClient.update();
  }
  loop_cnt++;
  delay(50);
}

void moveToPosition(uint8_t percent, uint8_t alexa_value) {
  /**
   * percent: (0-100) where 0 is closed and 100 is totally open
   * alexa_value: (0-255) where 0 is closed and 255 is totally open
   */
  //uint8_t current_percent;
  #if defined(DEBUG) || defined(KINGART_Q4)
  char str[90];
  #endif
  device->setValue(alexa_value);
  espalexa.loop();
  #ifdef DEBUG
    sprintf(str, "Value going to change to percent %u; alexa_value = %u", percent, alexa_value);
    mqtt.publish(DEBUG_TOPIC, str);
    delay(50);
  #endif
  #ifdef KINGART_Q4
    uint8_t percent_ka = 100-percent;
    sprintf(str, "AT+UPDATE=\"sequence\":\"1572536577552\",\"setclose\":%d\x1b", percent_ka); //TODO Change to random values in sequence if needed... (aparently not)
    Serial.print(str);
    Serial.flush();
    if (config.homeAssistantEnabled()) {
      if (config.getCurrentPosition() > percent) {
        ha.SendUpdate(percent, 0, -1);
      } else {
        ha.SendUpdate(percent, 100, 1);
      }
      
    }
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
      // First, assure RELAY-UP is stopped
      digitalWrite(config.getPinRelayUp(), LOW);
      digitalWrite(config.getPinRelayDown(), HIGH);
      // This block of code is to prevent the error in alexa like: "Device does not response"
      uint8_t ent = diff/1000;
      uint8_t dec = diff%1000;
      for (uint8_t i=0;i<ent;++i) {
        delay(1000);
        espalexa.loop();
        if (config.homeAssistantEnabled()) {
          current_percent = config.getCurrentPosition()-1000*(i+1)/milliseconds_per_percent;
          ha.SendUpdate(current_percent, percent, -1);
        } 
      }
      delay(dec);
      espalexa.loop();

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
        if (config.homeAssistantEnabled()) {
          current_percent = config.getCurrentPosition()+1000*(i+1)/milliseconds_per_percent;
          ha.SendUpdate(current_percent, percent, 1);
        }
      }
      delay(dec);
      espalexa.loop();
       
      //delay(diff);
      // End of the block of alexa problem
      digitalWrite(config.getPinRelayUp(), LOW);
    }
    config.setCurrentPosition(percent);
    if (config.homeAssistantEnabled()) {
      ha.SendUpdate(percent, percent, 0);
    } 
    //device->setPercent(percent);
  #endif

}

// MQTT callback (when receive a msg in the subscribed topics)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  String topic_str = String(topic);
  for (unsigned int i=0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  messageTemp += '\0';
  
  #ifdef DEBUG
  mqtt.publish(DEBUG_TOPIC, "Callback msg: ");
  mqtt.publish(DEBUG_TOPIC, messageTemp.c_str());
  delay(50);
  #endif
  if (messageTemp == "ShutterOpen1") {
    virtual_button = 1;
  }
  if (messageTemp == "ShutterStop1") {
    virtual_button = 2;
  }
  if (messageTemp == "ShutterClose1") {
    virtual_button = -1;
  }
  if (topic_str.endsWith("ShutterPosition1")) {
    uint8_t percent = messageTemp.toInt();
    uint8_t alexa_value = (percent * 255)/100;
    moveToPosition(percent, alexa_value);
  }
  //uint8_t percent = atoi(messageTemp.c_str());
  //uint8_t alexa_value = (percent * 255)/100;
  //moveToPosition(percent, alexa_value);
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
