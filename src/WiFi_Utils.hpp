#include "config.h"

bool ConnectWiFi_STA(const char *ssid, const char *password, bool useStaticIP = false)
{
   //Serial.println("");
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   if(useStaticIP) WiFi.config(ip, gateway, subnet);
   uint8 count = 100;
   while ((WiFi.status() != WL_CONNECTED) && count>0) 
   { 
     delay(100);  
     Serial.print('.'); 
     count--;
   }
   if (WiFi.status() == WL_CONNECTED) {
     return true;
   }
   return false;
}

void ConnectWiFi_AP(bool useStaticIP = false)
{ 
   Serial.println("Go to AP mode");
   WiFi.mode(WIFI_AP);
   while(!WiFi.softAP("MIESPITO", "fibonacci"))
   {
     Serial.println(".");
     delay(100);
   }
   //if(useStaticIP) WiFi.softAPConfig(ip, gateway, subnet);
   WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), subnet);
}