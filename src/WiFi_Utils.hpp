#include <ESP8266WiFi.h>

bool wifiAP = false;

bool ConnectWiFi_STA(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //if(useStaticIP) WiFi.config(ip, gateway, subnet);
  uint8 count = 100;
  while ((WiFi.status() != WL_CONNECTED) && count>0) 
  { 
    delay(100);  
    Serial.print('*');
    count--;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiAP = false;
    return true;
  }
  return false;
}

void ConnectWiFi_AP()
{ 
  char APName[17];
  if (wifiAP == false) {
    WiFi.mode(WIFI_AP);
    snprintf(APName, 17, "SHUTTERFW-%06X", (uint32_t)ESP.getChipId());
    Serial.print("Go to AP mode: ");
    Serial.println(APName);
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255, 255, 255, 0));
    while(!WiFi.softAP(APName, "fibonacci", 5, 0, 4))
    {
      Serial.println("-");
      delay(100);
    }
    wifiAP = true;
  }
}
