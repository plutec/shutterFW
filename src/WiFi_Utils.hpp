#include "config.h"

bool wifiAP = false;

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
    wifiAP = false;
    return true;
  }
  return false;
}

void ConnectWiFi_AP(bool useStaticIP = false)
{ 
  if (wifiAP == false) {
    Serial.println("Go to AP mode");
    WiFi.mode(WIFI_AP);
    char ap_name[17];
    snprintf(ap_name, 17, "SHUTTERFW-%06X",(uint32_t)ESP.getChipId());
    while(!WiFi.softAP(ap_name, "fibonacci"))
    {
      Serial.println(".");
      delay(100);
    }
    //if(useStaticIP) WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), subnet);
    wifiAP = true;
  }
}